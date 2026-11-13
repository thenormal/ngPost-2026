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

#include "MediaInfoReader.h"
#include "NfoWriter.h"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QProcess>
#include <QStandardPaths>

namespace {
const QStringList sExtensions = {
    "mkv", "mp4", "avi", "mov", "m4v", "ts", "m2ts", "wmv", "flv", "webm", "mpg", "mpeg",
    "mp3", "flac", "ogg", "m4a", "aac", "wav", "opus", "ac3", "dts"};

QString language(const QString &code)
{
    static const QMap<QString, QString> names = {
        {"en", "English"}, {"it", "Italian"}, {"fr", "French"}, {"de", "German"}, {"es", "Spanish"},
        {"pt", "Portuguese"}, {"nl", "Dutch"}, {"ja", "Japanese"}, {"zh", "Chinese"}, {"ru", "Russian"},
        {"ko", "Korean"}, {"pl", "Polish"}, {"sv", "Swedish"}, {"da", "Danish"}, {"no", "Norwegian"},
        {"fi", "Finnish"}, {"tr", "Turkish"}, {"ar", "Arabic"}, {"he", "Hebrew"}, {"hi", "Hindi"},
        {"cs", "Czech"}, {"hu", "Hungarian"}, {"el", "Greek"}, {"ro", "Romanian"}, {"uk", "Ukrainian"}};
    if (code.isEmpty())
        return QString();
    QString key = code.left(2).toLower();
    if (names.contains(key) && (code.size() == 2 || code.at(2) == '-'))
        return names.value(key);
    return code;
}

QString channels(const QString &n)
{
    // "6" or "6 / 2" (several layers): the first value counts
    int c = n.split(QRegExp("[^0-9]"), QString::SkipEmptyParts).value(0).toInt();
    switch (c)
    {
    case 0: return QString();
    case 1: return "1.0";
    case 2: return "2.0";
    case 3: return "2.1";
    case 6: return "5.1";
    case 8: return "7.1";
    default: return QString("%1 ch").arg(c);
    }
}

QString bitrate(const QString &bps)
{
    bool ok = false;
    double v = bps.split(QRegExp("[^0-9.]"), QString::SkipEmptyParts).value(0).toDouble(&ok);
    if (!ok || v <= 0)
        return QString();
    if (v >= 1e6)
        return QString("%1 Mb/s").arg(v / 1e6, 0, 'f', 1);
    return QString("%1 kb/s").arg(qRound(v / 1000.));
}

QString duration(const QString &seconds)
{
    bool ok = false;
    double v = seconds.toDouble(&ok);
    if (!ok || v <= 0)
        return QString();
    qint64 s = qRound64(v);
    return QString("%1:%2:%3").arg(s / 3600, 2, 10, QChar('0')).arg((s / 60) % 60, 2, 10, QChar('0')).arg(s % 60, 2, 10, QChar('0'));
}

QString join(const QStringList &parts)
{
    QStringList ok;
    for (const QString &p : parts)
        if (!p.trimmed().isEmpty())
            ok << p.trimmed();
    return ok.join(", ");
}

QString str(const QJsonObject &o, const char *key)
{
    QJsonValue v = o.value(QLatin1String(key));
    return v.isString() ? v.toString() : (v.isDouble() ? QString::number(v.toDouble()) : QString());
}
} // namespace

bool MediaInfoReader::isMediaFile(const QString &fileName)
{
    return sExtensions.contains(QFileInfo(fileName).suffix().toLower());
}

QString MediaInfoReader::findProgram(const QString &configured)
{
    if (!configured.trimmed().isEmpty())
    {
        QFileInfo fi(configured.trimmed());
        return fi.isFile() ? fi.absoluteFilePath() : QString();
    }
#ifdef Q_OS_WIN
    QFileInfo bundled(QCoreApplication::applicationDirPath() + "/MediaInfo/MediaInfo.exe");
    if (bundled.isFile())
        return bundled.absoluteFilePath();
#endif
    return QStandardPaths::findExecutable("mediainfo");
}

QString MediaInfoReader::format(const QByteArray &json, const QString &displayName)
{
    QJsonObject media = QJsonDocument::fromJson(json).object().value("media").toObject();
    QJsonArray tracks = media.value("track").toArray();
    if (tracks.isEmpty())
        return QString();

    QString head;
    QStringList video, audio, subs;
    for (const QJsonValue &tv : tracks)
    {
        QJsonObject t = tv.toObject();
        QString type = str(t, "@type");
        if (type == "General")
        {
            head = join({duration(str(t, "Duration")), NfoWriter::humanSize(str(t, "FileSize").toLongLong())});
            if (str(t, "FileSize").isEmpty())
                head = duration(str(t, "Duration"));
        }
        else if (type == "Video")
        {
            QString fmt = str(t, "Format");
            if (!str(t, "Format_Profile").isEmpty())
                fmt += " " + str(t, "Format_Profile").section('@', 0, 0);
            QString size = (str(t, "Width").isEmpty() || str(t, "Height").isEmpty()) ? QString()
                                                                                      : str(t, "Width") + "x" + str(t, "Height");
            double fv = str(t, "FrameRate").toDouble();
            QString fps = fv > 0 ? QString::number(fv, 'f', 3).remove(QRegExp("\\.?0+$")) + " fps" : QString();
            QString hdr = str(t, "HDR_Format");
            if (!hdr.isEmpty())
                hdr = hdr.contains("Dolby Vision") ? "Dolby Vision" : (str(t, "HDR_Format_Compatibility").contains("HDR10") ? "HDR10" : "HDR");
            video << join({fmt, size, fps, hdr});
        }
        else if (type == "Audio")
            audio << join({language(str(t, "Language")), str(t, "Format"), channels(str(t, "Channels")), bitrate(str(t, "BitRate"))});
        else if (type == "Text")
        {
            QString fmt = str(t, "Format");
            if (fmt == "UTF-8")
                fmt = "SRT";
            QString l = language(str(t, "Language"));
            if (l.isEmpty())
                l = "?";
            subs << (fmt.isEmpty() ? l : QString("%1 (%2)").arg(l, fmt));
        }
    }

    QString out = QString("   %1%2\n").arg(displayName, head.isEmpty() ? QString() : " - " + head);
    for (const QString &v : video)
        out += QString("     Video     : %1\n").arg(v);
    for (const QString &a : audio)
        out += QString("     Audio     : %1\n").arg(a);
    if (!subs.isEmpty())
        out += QString("     Subtitles : %1\n").arg(subs.join(", "));
    return out;
}

MediaInfoReader::Result MediaInfoReader::read(const QString &program, const QList<QPair<QString, QString>> &files)
{
    Result res;
    QList<QPair<QString, QString>> media;
    for (const auto &f : files)
        if (isMediaFile(f.first))
            media << f;
    res.nbMediaFiles = media.size();
    if (program.isEmpty())
        return res;

    QElapsedTimer total;
    total.start();
    for (const auto &f : media)
    {
        if (res.nbDescribed >= MAX_FILES)
            break;
        qint64 left = TOTAL_BUDGET_MS - total.elapsed();
        if (left <= 0)
        {
            res.timedOut = true;
            break;
        }
        QProcess p;
        p.start(program, {"--Output=JSON", f.second});
        if (!p.waitForFinished(static_cast<int>(qMin<qint64>(left, FILE_TIMEOUT_MS))))
        {
            p.kill();
            p.waitForFinished(1000);
            res.timedOut = true;
            break;
        }
        QString block = format(p.readAllStandardOutput(), f.first);
        if (!block.isEmpty())
        {
            res.text += block;
            ++res.nbDescribed;
        }
    }
    return res;
}
