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

#ifndef LATENCYSTATS_H
#define LATENCYSTATS_H

#include <QString>
#include <QMutex>
#include <QMap>
#include <QVector>

/*!
 * \brief Latency measured on the connections to the servers during the session.
 * Nothing is sent for it: it only times what ngPost already does.
 *  - CONNECT: TCP connection (network)
 *  - SSL:     SSL handshake
 *  - LOGIN:   AUTHINFO user + pass (two round trips)
 *  - ROUND_TRIP: command POST until the answer 340 (one round trip, no data: the real latency)
 *  - ACCEPT:  article data sent until the answer 240 (depends on the size, the bandwidth and the server)
 * Thread safe: the connections live in several threads.
 */
class LatencyStats
{
public:
    enum Kind {CONNECT = 0, SSL, LOGIN, ROUND_TRIP, ACCEPT, NB_KINDS};

    struct Summary {
        int    count  = 0;
        double minMs  = 0;
        double medMs  = 0;
        double p95Ms  = 0;
        double maxMs  = 0;
    };

    static LatencyStats &instance();

    void add(const QString &server, Kind kind, qint64 elapsedNs);
    void clear();

    QStringList servers() const;
    Summary summary(const QString &server, Kind kind) const;
    //! summary of all the servers together
    Summary summary(Kind kind) const;

    //! text for the diagnostic report (empty lines if nothing was measured)
    QString report() const;
    //! one line for the log at the end of a post ("" if there is nothing)
    QString oneLine() const;

    static QString kindName(Kind kind);
    //! median of the round trip above which a warning is written
    static constexpr double sHighRoundTripMs = 500.;

    static Summary compute(QVector<double> values, int total);

private:
    static constexpr int sMaxSamples = 20000; //!< per server and kind (then random replacement)

    struct Samples {
        QVector<double> values; //!< in ms
        int             total = 0;
    };

    mutable QMutex                    _mutex;
    QMap<QString, Samples>            _data[NB_KINDS];
    unsigned                          _rnd = 12345;
};

#endif // LATENCYSTATS_H
