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

#include "Notifier.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QStringList>
#include <QUrl>

Notifier::Notifier(QObject *parent) :
    QObject(parent), _net(new QNetworkAccessManager(this))
{
}

bool Notifier::_wanted(State state) const
{
    return state == State::OK ? _settings.onSuccess : _settings.onErrors;
}

QJsonObject Notifier::discordBody(const QString &text)
{
    QJsonObject o;
    o["content"] = text;
    return o;
}

static QString stateText(Notifier::State s)
{
    switch (s)
    {
    case Notifier::State::OK:     return Notifier::tr("finished");
    case Notifier::State::ERRORS: return Notifier::tr("finished with errors");
    case Notifier::State::FAILED: return Notifier::tr("stopped or failed");
    }
    return QString();
}

QString Notifier::message(const QList<Result> &results, const QString &appName, QString *title)
{
    int nbOk = 0, nbErr = 0, nbFail = 0;
    for (const Result &r : results)
    {
        if (r.state == State::OK) ++nbOk;
        else if (r.state == State::ERRORS) ++nbErr;
        else ++nbFail;
    }
    auto describe = [](const Result &r){
        QString line = r.name;
        if (r.state == State::OK)
            line += QString(" - %1, %2, %3").arg(r.size, r.duration, r.speed);
        else if (r.state == State::ERRORS)
            line += " - " + tr("%1 of %2 articles failed").arg(r.nbFailed).arg(r.nbTotal);
        else
            line += " - " + stateText(r.state);
        return line;
    };

    if (results.size() == 1)
    {
        const Result &r = results.first();
        if (title)
            *title = QString("%1: %2").arg(appName, stateText(r.state));
        return describe(r);
    }

    if (title)
        *title = tr("%1: %2 post(s) done").arg(appName).arg(results.size());
    QStringList lines;
    lines << tr("%1 OK, %2 with errors, %3 stopped or failed").arg(nbOk).arg(nbErr).arg(nbFail);
    const int maxLines = 10;
    for (int i = 0; i < results.size() && i < maxLines; ++i)
        lines << "- " + describe(results.at(i));
    if (results.size() > maxLines)
        lines << tr("... and %1 more").arg(results.size() - maxLines);
    return lines.join("\n");
}

void Notifier::jobEnded(const Result &result, bool allDone)
{
    if (!_settings.isActive())
        return;
    if (_settings.allDone)
    {
        _waiting << result;
        if (!allDone)
            return;
        QList<Result> toSend;
        for (const Result &r : _waiting)
            if (_wanted(r.state))
                toSend << r;
        _waiting.clear();
        if (toSend.isEmpty())
            return;
        QString title;
        QString text = message(toSend, _appName, &title);
        _send(title, text);
    }
    else if (_wanted(result.state))
    {
        QString title;
        QString text = message({result}, _appName, &title);
        _send(title, text);
    }
}

QString Notifier::sendTest(const QString &appName)
{
    QStringList done;
    const QString title = tr("%1: test").arg(appName), text = tr("This is a test message.");
    if (_settings.tray)
    {
        emit trayMessage(title, text);
        done << tr("system notification");
    }
    if (_settings.hasRemote())
    {
        _send(title, text);
        if (!_settings.discord.isEmpty()) done << "Discord";
        if (!_settings.telegramToken.isEmpty() && !_settings.telegramChat.isEmpty()) done << "Telegram";
        if (!_settings.webhook.isEmpty()) done << "webhook";
    }
    return done.isEmpty() ? tr("No notification channel is set.") : tr("Test message sent: %1 (a failure is written in the log).").arg(done.join(", "));
}

void Notifier::_send(const QString &title, const QString &text)
{
    if (_settings.tray)
        emit trayMessage(title, text);

    const QString body = title + "\n" + text;
    if (!_settings.discord.isEmpty())
        _post("Discord", QUrl(_settings.discord), QJsonDocument(discordBody(body)).toJson(QJsonDocument::Compact));
    if (!_settings.webhook.isEmpty())
    {
        QJsonObject o;
        o["title"] = title;
        o["text"]  = text;
        _post("webhook", QUrl(_settings.webhook), QJsonDocument(o).toJson(QJsonDocument::Compact));
    }
    if (!_settings.telegramToken.isEmpty() && !_settings.telegramChat.isEmpty())
    {
        QString base = qEnvironmentVariable("NGPOST_TELEGRAM_API");
        if (base.isEmpty())
            base = "https://api.telegram.org";
        QJsonObject o;
        o["chat_id"] = _settings.telegramChat;
        o["text"]    = body;
        _post("Telegram", QUrl(QString("%1/bot%2/sendMessage").arg(base, _settings.telegramToken)),
              QJsonDocument(o).toJson(QJsonDocument::Compact));
    }
}

void Notifier::_post(const QString &channel, const QUrl &url, const QByteArray &json)
{
    if (!url.isValid() || (url.scheme() != "https" && url.scheme() != "http"))
    {
        emit log(tr("Notification (%1) not sent: the address is not valid").arg(channel));
        return;
    }
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply *reply = _net->post(req, json);
    reply->setProperty("channel", channel);
    ++_pending;
    connect(reply, &QNetworkReply::finished, this, &Notifier::_onReplyFinished);
    QTimer::singleShot(10000, reply, [reply](){ if (reply->isRunning()) reply->abort(); }); // never wait more than 10 s
}

void Notifier::_onReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    const QString channel = reply->property("channel").toString();
    const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    // the address and the token are secrets: only the kind of error is written, never reply->errorString() (it has the address)
    if (reply->error() != QNetworkReply::NoError)
    {
        if (status)
            emit log(tr("Notification (%1) failed: the server answered %2").arg(channel).arg(status));
        else
            emit log(tr("Notification (%1) failed: no answer from the server").arg(channel));
    }
    reply->deleteLater();
    if (--_pending <= 0)
    {
        _pending = 0;
        emit allSent();
    }
}
