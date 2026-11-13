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


#ifndef DUPLICATESDIALOG_H
#define DUPLICATESDIALOG_H

#include <QList>
#include <QSet>
#include <QString>

class QWidget;

namespace DuplicatesDialog
{
struct Item {
    QString path;       //!< the file or folder
    QString name;
    qint64  size = 0;
    QString postedOn;   //!< date of the previous post (from the history)
};

/*!
 * \brief One window for all the items of a scan that are already in the history (instead of one question per post).
 * Each item has a box "Post again", off by default.
 * \param again  the paths the user wants to post again
 * \return false if the user cancelled (nothing must be posted)
 */
bool run(QWidget *parent, const QList<Item> &items, QSet<QString> &again);
}

#endif // DUPLICATESDIALOG_H
