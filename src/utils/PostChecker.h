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

#ifndef POSTCHECKER_H
#define POSTCHECKER_H

#include <QList>
#include <QString>

/*!
 * \brief Checks of a Quick Post before it starts, and the summary line (size, time).
 * No access to the program or to the GUI: the caller gives what it knows (Input), so the logic is easy to test.
 */
class PostChecker
{
public:
    enum class Code {
        NoServer, NoGroup, FoldersNoCompress, RarMissing, Par2Missing, PasswordEmpty, NzbFolderNotWritable,
        TmpNotWritable, LowSpace, MediaInfoMissing
    };

    struct Issue {
        Code    code;
        bool    blocking;   //!< an error: the post can't start until it is fixed (else a warning: "Post anyway")
        QString text;
        QString fixLabel;   //!< label of the button that fixes it (empty = no fix)
    };

    struct Input {
        int     nbServers = 0;          //!< enabled servers
        bool    hasGroups = true;
        QString invalidGroups;          //!< not empty: the groups that are not valid
        bool    hasFolder = false;
        bool    compress = false;
        QString rarPath;
        QString tmpPath;
        bool    par2 = false;
        QString par2Path;
        bool    passwordChecked = false;
        QString password;
        QString nzbFile;                //!< path of the nzb that will be written
        qint64  sourceBytes = 0;
        uint    par2Pct = 0;
        qint64  freeBytesTmp = -1;      //!< free space of the temporary folder (-1: unknown)
        bool    mediaInfoWanted = false;//!< NFO with media information is on
        bool    hasMediaFiles = false;
        bool    mediaInfoFound = true;
    };

    static QList<Issue> check(const Input &in);

    //! bytes to upload for \a sourceBytes: the archive (same size, the media are not compressible) and par2
    static qint64 uploadBytes(qint64 sourceBytes, bool par2, uint par2Pct);

    //! "about 1 h 05 min" ("" if the speed is not known)
    static QString durationText(qint64 bytes, double bytesPerSec);
};

#endif // POSTCHECKER_H
