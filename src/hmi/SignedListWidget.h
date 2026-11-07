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

#ifndef SIGNEDLISTWIDGET_H
#define SIGNEDLISTWIDGET_H

#include <QListWidget>
class QLabel;
class QPushButton;
class SignedListWidget : public QListWidget
{
    Q_OBJECT

public:
    SignedListWidget(QWidget *parent = nullptr);
    ~SignedListWidget() override;

    //! screen shown in the middle of the list when it is empty (logo, name, help and two buttons)
    void setEmptyState(const QString &hint, const QString &button1, const QString &button2);

    void addPath(const QString &path, bool isDir = false);
    bool addPathIfNotInList(const QString &path, int lastIndexToCheck, bool isDir = false);

    void removeItemWidget2(QListWidgetItem *item);

signals:
    void rightClick();
    void empty();
    void emptyAction1(); //!< first button of the empty screen
    void emptyAction2(); //!< second button of the empty screen

public slots:
    void clear2();
    void onDeleteSelectedItems();



protected:
    void resizeEvent(QResizeEvent *e) override;
    void mousePressEvent(QMouseEvent * e) override;    
    bool eventFilter(QObject *obj, QEvent *event) override;


private:
    void _updateEmptyState();
    void _placeEmptyState();

    QWidget     *_emptyState;
    QLabel      *_emptyLogo;
    QLabel      *_emptyTitle;
    QLabel      *_emptyHint;
    QPushButton *_emptyButton1;
    QPushButton *_emptyButton2;
    QIcon   _fileIcon;
    QIcon   _folderIcon;
};



#endif // SIGNEDLISTWIDGET_H
