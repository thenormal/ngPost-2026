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

#include "NfoWriter.h"
#include <QStringList>

QString NfoWriter::humanSize(qint64 bytes)
{
    const char *units[] = {"B", "KB", "MB", "GB", "TB"};
    double v = static_cast<double>(bytes);
    int u = 0;
    while (v >= 1024. && u < 4)
    {
        v /= 1024.;
        ++u;
    }
    return u == 0 ? QString("%1 B").arg(bytes) : QString("%1 %2").arg(v, 0, 'f', 2).arg(units[u]);
}

QString NfoWriter::build(const NfoInfo &info)
{
    const QString line(70, QChar('='));
    QString out;
    qint64 total = 0;
    for (const File &f : info.files)
        total += f.size;

    out += line + "\n";
    out += QString(" %1\n").arg(info.postName);
    out += line + "\n\n";
    out += QString(" Posted with : %1 %2\n").arg(info.appName, info.appVersion);
    out += QString(" Date        : %1\n").arg(info.date.toString("yyyy-MM-dd hh:mm"));
    out += QString(" Content     : %1 file(s), %2\n").arg(info.files.size()).arg(humanSize(total));
    if (info.compressed)
        out += QString(" Archive     : %1, %2 volume(s)\n").arg(info.archiveFormat).arg(info.nbVolumes);
    if (info.par2)
        out += QString(" par2        : %1 %% redundancy\n").arg(info.par2Pct);
    if (info.compressed || info.par2)
        out += QString(" Uploaded    : %1\n").arg(humanSize(info.uploadedSize));
    if (!info.password.isEmpty())
        out += QString(" Password    : %1\n").arg(info.password);
    out += "\n";

    out += " Files\n -----\n";
    if (info.hideNames)
        out += " The names of the files are hidden (obfuscation is on).\n";
    else
    {
        int width = 0;
        for (const File &f : info.files)
            width = qMax(width, f.name.size());
        width = qMin(width, 52);
        for (const File &f : info.files)
            out += QString("   %1  %2\n").arg(f.name, -width).arg(humanSize(f.size), 10);
    }

    if (info.mediaEnabled && info.hideNames)
        out += "\n Media information\n -----------------\n The media information is not written (obfuscation is on).\n";
    else if (info.mediaEnabled && !info.media.isEmpty())
    {
        out += "\n Media information\n -----------------\n" + info.media;
        if (info.mediaTimedOut)
            out += "   (reading stopped after 20 seconds: the other files are not described)\n";
        else if (info.mediaFiles > info.mediaDescribed)
            out += QString("   ... and %1 more media file(s)\n").arg(info.mediaFiles - info.mediaDescribed);
    }

    if (!info.freeText.trimmed().isEmpty())
    {
        out += "\n" + QString(70, QChar('-')) + "\n";
        out += info.freeText.trimmed() + "\n";
    }
    out += "\n" + line + "\n";
    return out;
}
