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
//
//========================================================================

#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QList>

class NgPost;
class QTableWidget;
class QLineEdit;
class QComboBox;
class QPushButton;
class QLabel;

//! History page: the uploads recorded in the history file (POST_HISTORY or ngPost_history.csv)
class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    struct Entry {
        QDateTime date;
        QString   name;
        QString   size;
        double    bytes      = 0;
        QString   speed;
        double    speedBps   = 0;
        QString   archive;
        QString   pass;
        QString   groups;
        QString   from;
        QString   status;    //!< OK, ERRORS (n) or empty for the lines written by the original ngPost
        int       nbFiles    = -1;
        QString   duration;
        qint64    durationSec = -1;
        QString   nzbPath;
    };

    HistoryWidget(NgPost *ngPost, QWidget *parent = nullptr);

    void reload();       //!< read the history file again
    void retranslate();

    static double parseSize(const QString &str);  //!< "12.34 MB" => bytes
    static double parseSpeed(const QString &str); //!< "62.45 MB/s" => bytes per second

private slots:
    void onFilter();
    void onShowPasswords(bool show);
    void onContextMenu(const QPoint &pos);
    void onExport();
    void onOpenFile();

private:
    void _fillTable();
    QList<int> _visibleEntries() const;
    const Entry *_entryAt(int row) const;

    NgPost        *_ngPost;
    QList<Entry>   _entries;
    bool           _showPasswords;

    QLabel        *_title;
    QLabel        *_subtitle;
    QLabel        *_warning;
    QLineEdit     *_search;
    QComboBox     *_period;
    QPushButton   *_passButton;
    QPushButton   *_exportButton;
    QPushButton   *_openButton;
    QLabel        *_summary;
    QTableWidget  *_table;
    QLabel        *_emptyLbl;

    enum Column {COL_DATE = 0, COL_NAME, COL_SIZE, COL_SPEED, COL_STATUS, COL_FILES, COL_DURATION,
                 COL_ARCHIVE, COL_PASS, COL_GROUPS, COL_FROM, NB_COLUMNS};
};

#endif // HISTORYWIDGET_H
