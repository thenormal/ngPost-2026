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

#ifndef POSTPROFILE_H
#define POSTPROFILE_H

#include <QString>

/*!
 * \brief A set of options for a post (what the user sets in the Archive card). Saved in the config file
 * with the key PROFILE, one line per profile: name|compress|genName|genPass|par2|par2Pct|keepRar
 */
struct PostProfile
{
    QString name;
    bool    compress = true;
    bool    genName  = true;   //!< random name of the archive
    bool    genPass  = true;   //!< random password
    bool    par2     = true;
    uint    par2Pct  = 10;
    bool    keepRar  = false;  //!< keep the archives after the post

    //! "Recommended": compression, random name and password, par2 with 10 %
    static PostProfile recommended();

    //! the name without the characters used by the config file
    static QString cleanName(const QString &name);

    QString toLine() const;
    static bool fromLine(const QString &line, PostProfile &profile);
};

#endif // POSTPROFILE_H
