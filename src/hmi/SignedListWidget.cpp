//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
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
//
//========================================================================

#include "SignedListWidget.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QIcon>

SignedListWidget::SignedListWidget(QWidget *parent) :
    QListWidget(parent),
    _emptyState(new QWidget(viewport())),
    _emptyLogo(new QLabel(_emptyState)),
    _emptyTitle(new QLabel("ngPost Deluxe", _emptyState)),
    _emptyHint(new QLabel(_emptyState)),
    _emptyButton1(new QPushButton(_emptyState)),
    _emptyButton2(new QPushButton(_emptyState)),
    _fileIcon(":/icons/file.png"),
    _folderIcon(":/icons/folder.png")
{
    _emptyState->setObjectName("emptyState");
    _emptyLogo->setPixmap(QIcon(":/icons/ngPost.png").pixmap(72, 72));
    _emptyLogo->setAlignment(Qt::AlignCenter);
    _emptyTitle->setObjectName("emptyTitle");
    _emptyTitle->setAlignment(Qt::AlignCenter);
    _emptyHint->setObjectName("hint");
    _emptyHint->setAlignment(Qt::AlignCenter);
    _emptyHint->setWordWrap(true);
    for (QLabel *lbl : {_emptyLogo, _emptyTitle, _emptyHint})
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents); // a right click still opens the file selection
    _emptyButton1->setObjectName("accentButton");
    _emptyButton1->setCursor(Qt::PointingHandCursor);
    _emptyButton2->setCursor(Qt::PointingHandCursor);

    QVBoxLayout *layout = new QVBoxLayout(_emptyState);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);
    layout->addWidget(_emptyLogo);
    layout->addSpacing(4);
    layout->addWidget(_emptyTitle);
    layout->addWidget(_emptyHint);
    layout->addSpacing(10);
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setSpacing(10);
    buttons->addStretch();
    buttons->addWidget(_emptyButton1);
    buttons->addWidget(_emptyButton2);
    buttons->addStretch();
    layout->addLayout(buttons);

    connect(_emptyButton1, &QAbstractButton::clicked, this, &SignedListWidget::emptyAction1);
    connect(_emptyButton2, &QAbstractButton::clicked, this, &SignedListWidget::emptyAction2);

    // the empty screen follows the content of the list, whatever the way items are added or removed
    connect(model(), &QAbstractItemModel::rowsInserted, this, &SignedListWidget::_updateEmptyState);
    connect(model(), &QAbstractItemModel::rowsRemoved,  this, &SignedListWidget::_updateEmptyState);
    connect(model(), &QAbstractItemModel::modelReset,   this, &SignedListWidget::_updateEmptyState);
}

SignedListWidget::~SignedListWidget()
{
    // the model is destroyed with the list: its signals must not reach the empty screen (already deleted) anymore
    if (model())
        disconnect(model(), nullptr, this, nullptr);
}

void SignedListWidget::setEmptyState(const QString &hint, const QString &button1, const QString &button2)
{
    _emptyHint->setText(hint);
    _emptyButton1->setText(button1);
    _emptyButton1->setVisible(!button1.isEmpty());
    _emptyButton2->setText(button2);
    _emptyButton2->setVisible(!button2.isEmpty());
    _placeEmptyState();
    _updateEmptyState();
}

void SignedListWidget::_updateEmptyState()
{
    _emptyState->setVisible(count() == 0);
}

void SignedListWidget::_placeEmptyState()
{
    // when the list is small, the logo then the title are hidden so the help and the buttons stay readable
    QSize area = viewport()->size();
    int width  = qMax(0, qMin(area.width() - 20, 460));
    _emptyLogo->show();
    _emptyTitle->show();
    auto neededHeight = [this, width]() {
        int h = _emptyState->layout()->hasHeightForWidth() ? _emptyState->layout()->heightForWidth(width) : -1;
        return h > 0 ? h : _emptyState->layout()->sizeHint().height();
    };
    if (neededHeight() > area.height())
        _emptyLogo->hide();
    if (neededHeight() > area.height())
        _emptyTitle->hide();
    int height = qMin(area.height(), neededHeight());
    _emptyState->setGeometry((area.width() - width) / 2, qMax(0, (area.height() - height) / 2), width, height);
}

void SignedListWidget::addPath(const QString &path, bool isDir)
{
    QListWidgetItem *item = new QListWidgetItem(isDir ? _folderIcon : _fileIcon, path);
    addItem(item);
}

bool SignedListWidget::addPathIfNotInList(const QString &path, int lastIndexToCheck, bool isDir)
{
    for (int i = 0 ; i < lastIndexToCheck ; ++i)
    {
        if (item(i)->text() == path)
            return false;
    }

    addPath(path, isDir);
    return true;
}

void SignedListWidget::removeItemWidget2(QListWidgetItem *item)
{
    bool last = count() == 1;
    removeItemWidget(item);
    if (last)
        emit empty();
}

void SignedListWidget::clear2()
{
    clear();
}

void SignedListWidget::onDeleteSelectedItems()
{
    for (QListWidgetItem *item : selectedItems())
    {
        removeItemWidget2(item);
        delete item;
    }
}

void SignedListWidget::resizeEvent(QResizeEvent *e)
{
    QListWidget::resizeEvent(e);
    _placeEmptyState();
}
void SignedListWidget::mousePressEvent(QMouseEvent *e)
{
    QListWidget::mousePressEvent(e);
    if (e->button() == Qt::RightButton)
    {
        emit rightClick();
        e->accept();
    }
}

bool SignedListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
            onDeleteSelectedItems();
    }
    return QListWidget::eventFilter(obj, event);       
}
