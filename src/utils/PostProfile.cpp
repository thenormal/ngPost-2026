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

#include "PostProfile.h"
#include <QStringList>

PostProfile PostProfile::recommended()
{
    return PostProfile(); // the defaults of the struct
}

QString PostProfile::cleanName(const QString &name)
{
    QString n = name;
    n.remove('|').remove('=').remove('#').remove('\n').remove('\r');
    return n.trimmed();
}

QString PostProfile::toLine() const
{
    return QString("%1|%2|%3|%4|%5|%6|%7").arg(cleanName(name)).arg(compress).arg(genName).arg(genPass).arg(par2).arg(par2Pct).arg(keepRar);
}

bool PostProfile::fromLine(const QString &line, PostProfile &profile)
{
    QStringList f = line.split('|');
    if (f.size() < 7 || cleanName(f[0]).isEmpty())
        return false;
    bool ok = true;
    uint pct = f[5].toUInt(&ok);
    if (!ok || pct > 100)
        return false;
    profile.name     = cleanName(f[0]);
    profile.compress = f[1] == "1";
    profile.genName  = f[2] == "1";
    profile.genPass  = f[3] == "1";
    profile.par2     = f[4] == "1";
    profile.par2Pct  = pct;
    profile.keepRar  = f[6] == "1";
    return true;
}
