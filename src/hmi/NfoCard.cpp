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

#include "NfoCard.h"
#include "NgPost.h"
#include "utils/MediaInfoReader.h"
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

// the texts keep the context "MainWindow": they were written there (Settings) and are already translated
static QString mw(const char *text) { return QCoreApplication::translate("MainWindow", text); }

NfoCard::NfoCard(NgPost *ngPost, QWidget *parent) :
    QFrame(parent), _ngPost(ngPost)
{
    setObjectName("card");
    setAttribute(Qt::WA_StyledBackground, true);
    QVBoxLayout *v = new QVBoxLayout(this);
    v->setContentsMargins(16, 12, 16, 14);
    v->setSpacing(10);
    _title = new QLabel(this);
    _title->setObjectName("cardTitle");
    v->addWidget(_title);

    QGridLayout *g = new QGridLayout();
    g->setHorizontalSpacing(10);
    g->setVerticalSpacing(8);
    v->addLayout(g);

    _nfoCB     = new QCheckBox(this);
    _inPostCB  = new QCheckBox(this);
    _passCB    = new QCheckBox(this);
    _mediaCB   = new QCheckBox(this);
    _mediaPath = new QLineEdit(this);
    _mediaButton = new QPushButton(" ... ", this);
    _textLbl   = new QLabel(this);
    _text      = new QPlainTextEdit(this);
    _text->setMaximumHeight(48);

    // compact: the card is shown in Quick Post and Auto Posting, under the other options
    g->addWidget(_nfoCB, 0, 0, 1, 3);
    g->addWidget(_inPostCB, 1, 0);
    g->addWidget(_passCB, 1, 1);
    g->addWidget(_mediaCB, 1, 2);
    QHBoxLayout *pathRow = new QHBoxLayout();
    pathRow->addWidget(_mediaPath, 1);
    pathRow->addWidget(_mediaButton);
    g->addLayout(pathRow, 2, 0, 1, 3);
    g->addWidget(_textLbl, 3, 0, Qt::AlignTop);
    g->addWidget(_text, 3, 1, 1, 2);
    g->setColumnStretch(1, 1);
    g->setColumnStretch(2, 1);

    connect(_nfoCB,    &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_genNfo = on; _updateEnabled(); emit _ngPost->nfoOptionsChanged(); });
    connect(_inPostCB, &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_nfoInPost = on; emit _ngPost->nfoOptionsChanged(); });
    connect(_passCB,   &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_nfoPassword = on; emit _ngPost->nfoOptionsChanged(); });
    connect(_mediaCB,  &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_nfoMedia = on; _updateEnabled(); emit _ngPost->nfoOptionsChanged(); });
    connect(_mediaPath, &QLineEdit::editingFinished, this, [this](){
        _ngPost->_mediaInfoPath = QDir::fromNativeSeparators(_mediaPath->text().trimmed());
        _updateMediaStatus();
        emit _ngPost->nfoOptionsChanged();
    });
    connect(_mediaButton, &QAbstractButton::clicked, this, [this](){
        QString file = QFileDialog::getOpenFileName(this, mw("Choose the MediaInfo program"), _mediaPath->text());
        if (!file.isEmpty())
        {
            _mediaPath->setText(QDir::toNativeSeparators(file));
            _ngPost->_mediaInfoPath = file;
            _updateMediaStatus();
            emit _ngPost->nfoOptionsChanged();
        }
    });
    connect(_text, &QPlainTextEdit::textChanged, this, [this](){
        if (_ngPost->_nfoText != _text->toPlainText())
        {
            _ngPost->_nfoText = _text->toPlainText();
            emit _ngPost->nfoOptionsChanged();
        }
    });
    connect(_ngPost, &NgPost::nfoOptionsChanged, this, &NfoCard::refresh);

    refresh();
    retranslate();
}

void NfoCard::refresh()
{
    // the options may have been changed by another card: no signal while the controls are updated
    QSignalBlocker b1(_nfoCB), b2(_inPostCB), b3(_passCB), b4(_mediaCB), b5(_text), b6(_mediaPath);
    _nfoCB->setChecked(_ngPost->_genNfo);
    _inPostCB->setChecked(_ngPost->_nfoInPost);
    _passCB->setChecked(_ngPost->_nfoPassword);
    _mediaCB->setChecked(_ngPost->_nfoMedia);
    if (_mediaPath->text() != QDir::toNativeSeparators(_ngPost->_mediaInfoPath))
        _mediaPath->setText(QDir::toNativeSeparators(_ngPost->_mediaInfoPath));
    if (_text->toPlainText() != _ngPost->_nfoText)
        _text->setPlainText(_ngPost->_nfoText);
    _updateEnabled();
    _updateMediaStatus();
}

void NfoCard::_updateEnabled()
{
    // the details are only shown when the NFO is on: the card stays small (it is under the other options)
    bool on = _ngPost->_genNfo;
    for (QWidget *w : {static_cast<QWidget*>(_inPostCB), static_cast<QWidget*>(_passCB), static_cast<QWidget*>(_mediaCB),
                       static_cast<QWidget*>(_mediaPath), static_cast<QWidget*>(_mediaButton),
                       static_cast<QWidget*>(_textLbl), static_cast<QWidget*>(_text)})
        w->setVisible(on);
    _inPostCB->setEnabled(on);
    _passCB->setEnabled(on);
    _mediaCB->setEnabled(on);
    _text->setEnabled(on);
    _textLbl->setEnabled(on);
    bool media = on && _ngPost->_nfoMedia;
    _mediaPath->setEnabled(media);
    _mediaButton->setEnabled(media);
}

void NfoCard::_updateMediaStatus()
{
    // the field shows what is used (when no path is chosen) and is red when MediaInfo can't be found
    QString program = MediaInfoReader::findProgram(_ngPost->_mediaInfoPath);
    QString status = program.isEmpty()
        ? mw("MediaInfo was not found: the NFO file will have no media information.")
        : mw("MediaInfo found: %1").arg(QDir::toNativeSeparators(program));
    _mediaPath->setToolTip(status);
    _mediaPath->setPlaceholderText(program.isEmpty() ? status
                                                     : mw("Path of MediaInfo (empty = automatic)") + " - " + status);
    _mediaPath->setStyleSheet(program.isEmpty() && _ngPost->_nfoMedia ? "QLineEdit { color: #c0392b; }" : QString());
}

void NfoCard::retranslate()
{
    _title->setText(mw("NFO file"));
    _nfoCB->setText(mw("Write an NFO file next to the nzb (name, date, files, archive, par2)"));
    _inPostCB->setText(mw("Also post the NFO file with the files"));
    _passCB->setText(mw("Write the password of the archive in the NFO file"));
    _mediaCB->setText(mw("Include media information (MediaInfo)"));
    _mediaCB->setToolTip(mw("Include media information (MediaInfo): duration, video, audio and subtitles"));
    _mediaButton->setToolTip(mw("Choose the MediaInfo program"));
    _textLbl->setText(mw("Text added at the end of the NFO file:"));
    _passCB->setToolTip(mw("Careful: a password in a posted NFO is readable by everyone who downloads the post."));
    _title->setToolTip(mw("When the names of the files are hidden (obfuscation) the NFO that is posted lists no file name."));
    _updateMediaStatus();
}
