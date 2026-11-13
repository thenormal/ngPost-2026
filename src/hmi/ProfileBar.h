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

#ifndef PROFILEBAR_H
#define PROFILEBAR_H

#include <QWidget>
#include <functional>
#include "utils/PostProfile.h"

class NgPost;
class QComboBox;
class QLabel;
class QPushButton;

/*!
 * \brief "Profile" bar shared by Quick Post and Auto Posting: choose a set of options (Custom, Recommended
 * or one saved by the user), save the current options as a profile, delete a profile.
 * The widget that owns the bar says how to read and apply the options.
 */
class ProfileBar : public QWidget
{
    Q_OBJECT
public:
    ProfileBar(NgPost *ngPost, QWidget *parent = nullptr);

    void setHandlers(std::function<PostProfile()> current, std::function<void(const PostProfile &)> apply);
    void markCustom();         //!< the user changed an option by hand
    void applyRecommendedIfNew(); //!< first start of a new installation: "Recommended"
    void retranslate();

private slots:
    void _onActivated(int index);
    void _onSave();
    void _onDelete();
    void _reload();

private:
    NgPost       *_ngPost;
    QLabel       *_label;
    QComboBox    *_combo;
    QPushButton  *_saveButton;
    QPushButton  *_deleteButton;
    std::function<PostProfile()> _current;
    std::function<void(const PostProfile &)> _apply;
    bool          _busy;

    void _fill(const QString &select);
    void _applyProfile(const PostProfile &profile);
};

#endif // PROFILEBAR_H
