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

#ifndef NFOCARD_H
#define NFOCARD_H

#include <QFrame>

class NgPost;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

/*!
 * \brief The "NFO file" card shared by Quick Post and Auto Posting. The options are global
 * (they are the ones of NgPost): when one card changes them the others are updated.
 */
class NfoCard : public QFrame
{
    Q_OBJECT
public:
    NfoCard(NgPost *ngPost, QWidget *parent = nullptr);
    void retranslate();

private slots:
    void refresh();

private:
    NgPost         *_ngPost;
    QLabel         *_title;
    QCheckBox      *_nfoCB;
    QCheckBox      *_inPostCB;
    QCheckBox      *_passCB;
    QCheckBox      *_mediaCB;
    QLineEdit      *_mediaPath;
    QPushButton    *_mediaButton;
    QLabel         *_textLbl;
    QPlainTextEdit *_text;

    void _updateEnabled();
    void _updateMediaStatus();
};

#endif // NFOCARD_H
