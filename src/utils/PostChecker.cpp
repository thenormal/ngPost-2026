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

#include "PostChecker.h"
#include "NfoWriter.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

static QString tr(const char *text) { return QCoreApplication::translate("PostChecker", text); }

QList<PostChecker::Issue> PostChecker::check(const Input &in)
{
    QList<Issue> issues;

    if (in.nbServers == 0)
        issues << Issue{Code::NoServer, true, tr("No server is enabled: add or enable one in Servers."), tr("Open Servers")};
    if (!in.hasGroups)
        issues << Issue{Code::NoGroup, true, tr("There is no newsgroup: add at least one in Settings."), tr("Open Settings")};
    else if (!in.invalidGroups.isEmpty())
        issues << Issue{Code::NoGroup, true, tr("These newsgroups are not valid: %1. Fix them in Settings.").arg(in.invalidGroups), tr("Open Settings")};

    if (in.hasFolder && !in.compress)
        issues << Issue{Code::FoldersNoCompress, true, tr("The list has folders, and folders can only be posted with compression."), tr("Turn compression on")};

    bool needsTmp = in.compress || in.par2;
    if (in.compress)
    {
        QFileInfo rar(in.rarPath);
        if (in.rarPath.isEmpty() || !rar.isFile() || !rar.isExecutable())
            issues << Issue{Code::RarMissing, true, tr("Compression is on but the rar / 7z program was not found."), tr("Choose the program...")};
    }
    if (in.par2)
    {
        QFileInfo par2(in.par2Path);
        if (in.par2Path.isEmpty() || !par2.isFile())
            issues << Issue{Code::Par2Missing, true, tr("par2 is on but the par2 program was not found."), tr("Turn par2 off")};
    }
    if (in.compress && in.passwordChecked && in.password.isEmpty())
        issues << Issue{Code::PasswordEmpty, true, tr("The password is turned on but it is empty."), tr("Generate a password")};

    QFileInfo nzbDir(QFileInfo(in.nzbFile).absolutePath());
    if (!in.nzbFile.isEmpty() && (!nzbDir.exists() || !nzbDir.isDir() || !nzbDir.isWritable()))
        issues << Issue{Code::NzbFolderNotWritable, true, tr("The folder of the NZB file does not exist or is not writable."), tr("Choose another file...")};

    if (needsTmp)
    {
        QFileInfo tmp(in.tmpPath);
        if (in.tmpPath.isEmpty() || !tmp.isDir() || !tmp.isWritable())
            issues << Issue{Code::TmpNotWritable, false, tr("The temporary folder does not exist or is not writable: the post will fail."), QString()};
        else if (in.freeBytesTmp >= 0)
        {
            qint64 need = (in.compress ? in.sourceBytes : 0) + (in.par2 ? in.sourceBytes * in.par2Pct / 100 : 0);
            if (need > 0 && in.freeBytesTmp < need + need / 20)
                issues << Issue{Code::LowSpace, false,
                                tr("The temporary folder may not have enough free space: about %1 needed, %2 free.").arg(
                                    NfoWriter::humanSize(need), NfoWriter::humanSize(in.freeBytesTmp)), QString()};
        }
    }

    if (in.mediaInfoWanted && in.hasMediaFiles && !in.mediaInfoFound)
        issues << Issue{Code::MediaInfoMissing, false, tr("MediaInfo was not found: the NFO file will have no media information."), tr("Turn media information off")};

    return issues;
}

qint64 PostChecker::uploadBytes(qint64 sourceBytes, bool par2, uint par2Pct)
{
    return sourceBytes + (par2 ? sourceBytes * par2Pct / 100 : 0);
}

QString PostChecker::durationText(qint64 bytes, double bytesPerSec)
{
    if (bytesPerSec <= 0 || bytes <= 0)
        return QString();
    qint64 sec = static_cast<qint64>(bytes / bytesPerSec);
    if (sec < 60)
        return tr("less than a minute");
    qint64 min = (sec + 30) / 60;
    if (min < 60)
        return tr("%1 min").arg(min);
    return tr("%1 h %2 min").arg(min / 60).arg(min % 60, 2, 10, QChar('0'));
}
