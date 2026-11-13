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

#ifndef POSTCHECKDIALOG_H
#define POSTCHECKDIALOG_H

#include <functional>
#include "utils/PostChecker.h"

class QWidget;

namespace PostCheckDialog
{
/*!
 * \brief Checks a post before it starts (Quick Post and Auto Posting). While there is something to say a window
 * lists the problems in plain words, each with a button that fixes it. After a fix the checks run again.
 * \param makeInput gives the current state (called before each round)
 * \param applyFix  does the fix; false = the user has to go elsewhere (Servers, Settings): the post is cancelled
 * \return true: the post can start (no problem, or only warnings and "Post anyway")
 */
bool run(QWidget *parent, const std::function<PostChecker::Input()> &makeInput,
         const std::function<bool(PostChecker::Code)> &applyFix);
}

#endif // POSTCHECKDIALOG_H
