//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================

#include "LatencyStats.h"

#include <QMutexLocker>
#include <QStringList>
#include <algorithm>

LatencyStats &LatencyStats::instance()
{
    static LatencyStats stats;
    return stats;
}

void LatencyStats::add(const QString &server, Kind kind, qint64 elapsedNs)
{
    if (elapsedNs < 0 || kind < 0 || kind >= NB_KINDS)
        return;
    double ms = static_cast<double>(elapsedNs) / 1e6;
    QMutexLocker lock(&_mutex);
    Samples &s = _data[kind][server];
    ++s.total;
    if (s.values.size() < sMaxSamples)
        s.values.append(ms);
    else
    {
        // reservoir sampling: every measure keeps the same chance to be in the list
        _rnd = _rnd * 1103515245u + 12345u;
        int idx = static_cast<int>((_rnd >> 8) % static_cast<unsigned>(s.total));
        if (idx < sMaxSamples)
            s.values[idx] = ms;
    }
}

void LatencyStats::clear()
{
    QMutexLocker lock(&_mutex);
    for (int k = 0; k < NB_KINDS; ++k)
        _data[k].clear();
}

QStringList LatencyStats::servers() const
{
    QMutexLocker lock(&_mutex);
    QStringList list;
    for (int k = 0; k < NB_KINDS; ++k)
        for (auto it = _data[k].constBegin(); it != _data[k].constEnd(); ++it)
            if (!list.contains(it.key()))
                list << it.key();
    list.sort();
    return list;
}

LatencyStats::Summary LatencyStats::compute(QVector<double> values, int total)
{
    Summary r;
    r.count = total;
    if (values.isEmpty())
        return r;
    std::sort(values.begin(), values.end());
    int n = values.size();
    r.minMs = values.first();
    r.maxMs = values.last();
    r.medMs = n % 2 ? values[n / 2] : (values[n / 2 - 1] + values[n / 2]) / 2.;
    r.p95Ms = values[qMin(n - 1, static_cast<int>(0.95 * n))];
    return r;
}

LatencyStats::Summary LatencyStats::summary(const QString &server, Kind kind) const
{
    QMutexLocker lock(&_mutex);
    auto it = _data[kind].constFind(server);
    if (it == _data[kind].constEnd())
        return Summary();
    return compute(it->values, it->total);
}

LatencyStats::Summary LatencyStats::summary(Kind kind) const
{
    QMutexLocker lock(&_mutex);
    QVector<double> all;
    int total = 0;
    for (auto it = _data[kind].constBegin(); it != _data[kind].constEnd(); ++it)
    {
        all += it->values;
        total += it->total;
    }
    return compute(all, total);
}

QString LatencyStats::kindName(Kind kind)
{
    switch (kind)
    {
    case CONNECT:    return "connection (TCP)";
    case SSL:        return "SSL handshake";
    case LOGIN:      return "login";
    case ROUND_TRIP: return "round trip (POST -> 340)";
    case ACCEPT:     return "article sent -> 240";
    default:         return QString();
    }
}

static QString ms(double v)
{
    return QString::number(v, 'f', v < 10 ? 1 : 0);
}

QString LatencyStats::report() const
{
    QStringList srv = servers();
    if (srv.isEmpty())
        return "nothing measured (no connection to a server in this session)\n";

    QString out;
    out += "(times in ms; the measures are only taken from what ngPost does, nothing is sent for them)\n";
    for (const QString &s : srv)
    {
        out += s + "\n";
        for (int k = 0; k < NB_KINDS; ++k)
        {
            Summary r = summary(s, static_cast<Kind>(k));
            if (!r.count)
                continue;
            out += QString("    %1: n=%2, min %3, median %4, p95 %5, max %6\n").arg(
                        kindName(static_cast<Kind>(k)), -26).arg(r.count).arg(ms(r.minMs), ms(r.medMs), ms(r.p95Ms), ms(r.maxMs));
        }
        Summary rt = summary(s, ROUND_TRIP);
        if (rt.count >= 5 && rt.medMs > sHighRoundTripMs)
            out += QString("    WARNING: the round trip is high (median %1 ms): the delay comes from the network, the distance to the server"
                           " or the server itself (a VPN adds delay), not from the computer.\n").arg(ms(rt.medMs));
        else if (rt.count >= 5)
            out += "    round trip: no high latency.\n";
    }
    out += "note: with many connections on a slow computer, the measures can be a bit higher than the real network delay.\n";
    return out;
}

QString LatencyStats::oneLine() const
{
    Summary rt = summary(ROUND_TRIP);
    if (!rt.count)
        return QString();
    QStringList parts;
    Summary c = summary(CONNECT);
    if (c.count)
        parts << QString("connection %1 ms").arg(ms(c.medMs));
    parts << QString("round trip %1 ms (p95 %2 ms)").arg(ms(rt.medMs), ms(rt.p95Ms));
    return QString("Latency (median, since the start of ngPost): %1").arg(parts.join(", "));
}
