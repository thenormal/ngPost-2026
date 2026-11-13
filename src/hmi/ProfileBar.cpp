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

#include "ProfileBar.h"
#include "NgPost.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>

ProfileBar::ProfileBar(NgPost *ngPost, QWidget *parent) :
    QWidget(parent), _ngPost(ngPost), _busy(false)
{
    QHBoxLayout *h = new QHBoxLayout(this);
    h->setContentsMargins(2, 0, 2, 0);
    h->setSpacing(8);
    _label = new QLabel(this);
    _combo = new QComboBox(this);
    _combo->setMinimumWidth(200);
    _saveButton   = new QPushButton(this);
    _deleteButton = new QPushButton(this);
    h->addWidget(_label);
    h->addWidget(_combo);
    h->addWidget(_saveButton);
    h->addWidget(_deleteButton);
    h->addStretch();

    connect(_combo, QOverload<int>::of(&QComboBox::activated), this, &ProfileBar::_onActivated);
    connect(_saveButton,   &QAbstractButton::clicked, this, &ProfileBar::_onSave);
    connect(_deleteButton, &QAbstractButton::clicked, this, &ProfileBar::_onDelete);
    connect(_ngPost, &NgPost::profilesChanged, this, &ProfileBar::_reload);
    _fill(QString());
    retranslate();
}

void ProfileBar::setHandlers(std::function<PostProfile()> current, std::function<void(const PostProfile &)> apply)
{
    _current = current;
    _apply   = apply;
}

void ProfileBar::retranslate()
{
    _label->setText(tr("Profile:"));
    _saveButton->setText(tr("Save as profile..."));
    _saveButton->setToolTip(tr("Save the current options (compression, random name and password, par2, keep the archives) under a name"));
    _deleteButton->setText(tr("Delete"));
    _deleteButton->setToolTip(tr("Delete the chosen profile"));
    QString sel = _combo->currentIndex() >= 2 ? _combo->currentText() : QString();
    int idx = _combo->currentIndex();
    _fill(sel);
    if (sel.isEmpty() && idx >= 0 && idx < 2)
        _combo->setCurrentIndex(idx);
}

void ProfileBar::_fill(const QString &select)
{
    QSignalBlocker block(_combo);
    int keep = _combo->currentIndex();
    _combo->clear();
    _combo->addItem(tr("Custom"));
    _combo->addItem(tr("Recommended (compression, random name and password, par2 10 %)"));
    for (const PostProfile &p : _ngPost->_profiles)
        _combo->addItem(p.name);
    int idx = select.isEmpty() ? qMax(0, keep) : _combo->findText(select);
    _combo->setCurrentIndex(idx < 0 || idx >= _combo->count() ? 0 : idx);
    _deleteButton->setEnabled(_combo->currentIndex() >= 2);
}

void ProfileBar::_reload()
{
    // another bar added or deleted a profile: keep my choice if it still exists
    QString sel = _combo->currentIndex() >= 2 ? _combo->currentText() : QString();
    int idx = _combo->currentIndex();
    _fill(sel);
    if (sel.isEmpty() && idx < 2)
        _combo->setCurrentIndex(idx);
}

void ProfileBar::markCustom()
{
    if (_busy)
        return;
    QSignalBlocker block(_combo);
    _combo->setCurrentIndex(0);
    _deleteButton->setEnabled(false);
}

void ProfileBar::applyRecommendedIfNew()
{
    if (!_ngPost->_freshInstall)
        return;
    _combo->setCurrentIndex(1);
    _applyProfile(PostProfile::recommended());
}

void ProfileBar::_applyProfile(const PostProfile &profile)
{
    if (!_apply)
        return;
    _busy = true;
    _apply(profile);
    _busy = false;
}

void ProfileBar::_onActivated(int index)
{
    _deleteButton->setEnabled(index >= 2);
    if (index == 1)
        _applyProfile(PostProfile::recommended());
    else if (index >= 2 && index - 2 < _ngPost->_profiles.size())
        _applyProfile(_ngPost->_profiles.at(index - 2));
}

void ProfileBar::_onSave()
{
    if (!_current)
        return;
    bool ok = false;
    QString name = PostProfile::cleanName(QInputDialog::getText(this, tr("Save as profile"), tr("Name of the profile:"),
                                                                 QLineEdit::Normal, _combo->currentIndex() >= 2 ? _combo->currentText() : QString(), &ok));
    if (!ok || name.isEmpty())
        return;
    if (name.compare(tr("Custom"), Qt::CaseInsensitive) == 0 || name.compare("Custom", Qt::CaseInsensitive) == 0
            || name.compare("Recommended", Qt::CaseInsensitive) == 0 || name.startsWith("Recommended ", Qt::CaseInsensitive))
    {
        QMessageBox::information(this, tr("Save as profile"), tr("This name is reserved: choose another one."));
        return;
    }
    PostProfile p = _current();
    p.name = name;
    bool replaced = false;
    for (PostProfile &old : _ngPost->_profiles)
        if (old.name.compare(name, Qt::CaseInsensitive) == 0)
        {
            old = p;
            replaced = true;
        }
    if (!replaced)
        _ngPost->_profiles.append(p);
    _fill(name);
    emit _ngPost->profilesChanged();
}

void ProfileBar::_onDelete()
{
    int idx = _combo->currentIndex();
    if (idx < 2 || idx - 2 >= _ngPost->_profiles.size())
        return;
    QString name = _ngPost->_profiles.at(idx - 2).name;
    if (QMessageBox::question(this, tr("Delete"), tr("Delete the profile \"%1\"?").arg(name)) != QMessageBox::Yes)
        return;
    _ngPost->_profiles.removeAt(idx - 2);
    _combo->setCurrentIndex(0);
    emit _ngPost->profilesChanged();
}
