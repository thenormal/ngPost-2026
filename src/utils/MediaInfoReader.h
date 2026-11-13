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

#ifndef MEDIAINFOREADER_H
#define MEDIAINFOREADER_H

#include <QString>
#include <QStringList>

/*!
 * \brief Reads the technical information of video and audio files with the program MediaInfo
 * (command line, "mediainfo --Output=JSON file"). MediaInfo is an external program: if it is not
 * available nothing is read and the NFO file simply has no media section.
 */
class MediaInfoReader
{
public:
    static constexpr int MAX_FILES       = 20;     //!< files described in the NFO
    static constexpr int FILE_TIMEOUT_MS = 10000;  //!< per file
    static constexpr int TOTAL_BUDGET_MS = 20000;  //!< for all the files

    struct Result {
        QString text;            //!< the block for the NFO (empty if nothing was read)
        int     nbMediaFiles = 0; //!< media files found in the post
        int     nbDescribed  = 0;
        bool    timedOut     = false;
    };

    //! true for the extensions of video and audio files MediaInfo is asked about
    static bool isMediaFile(const QString &fileName);

    //! the program to run: configured path, then MediaInfo/MediaInfo.exe next to ngPost (Windows), then "mediainfo" in the PATH. Empty if not found
    static QString findProgram(const QString &configured);

    //! reads the given files (relative name shown in the NFO, absolute path)
    static Result read(const QString &program, const QList<QPair<QString, QString>> &files);

    //! converts the JSON of MediaInfo to the lines of the NFO ("" if it can't be parsed). Public to be testable
    static QString format(const QByteArray &json, const QString &displayName);
};

#endif // MEDIAINFOREADER_H
