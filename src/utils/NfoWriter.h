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

#ifndef NFOWRITER_H
#define NFOWRITER_H

#include <QString>
#include <QList>
#include <QDateTime>

/*!
 * \brief Builds the text of the NFO file of a post: name, date, files, how it was prepared
 * (archive, par2), optionally the password, and a free text of the user.
 * No access to the program state: everything comes from NfoInfo, so it is easy to test.
 */
class NfoWriter
{
public:
    struct File {
        QString name;   //!< path relative to the post (folders are listed file by file)
        qint64  size = 0;
    };

    struct NfoInfo {
        QString        postName;
        QString        appName;
        QString        appVersion;
        QDateTime      date;
        QList<File>    files;          //!< the original files (not the archives)
        bool           hideNames = false; //!< names are obfuscated: no file name is written
        bool           compressed = false;
        QString        archiveFormat;  //!< "RAR" or "7z"
        int            nbVolumes = 0;
        qint64         uploadedSize = 0; //!< size of what is really uploaded (archives and par2)
        bool           par2 = false;
        uint           par2Pct = 0;
        QString        password;       //!< empty = not written
        QString        freeText;
        bool           mediaEnabled = false; //!< "Include media information" is on
        QString        media;          //!< text read by MediaInfo (empty = nothing read)
        int            mediaFiles = 0;     //!< media files in the post
        int            mediaDescribed = 0; //!< files described in \a media
        bool           mediaTimedOut = false; //!< the time limit stopped the reading
    };

    static QString build(const NfoInfo &info);
    static QString humanSize(qint64 bytes);
};

#endif // NFOWRITER_H
