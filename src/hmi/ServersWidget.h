//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
// ngPost 2026 fork: https://github.com/thenormal/ngPost-2026
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

#ifndef SERVERSWIDGET_H
#define SERVERSWIDGET_H

#include <QWidget>
#include <QVector>
#include "nntp/NntpServerParams.h"

class QListWidget;
class QListWidgetItem;
class QLineEdit;
class QSpinBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QAction;
class QFormLayout;
class QStackedWidget;
class ServerTester;

/*!
 * \brief Server section of the main window: list of the servers on the left
 * (status dot + on/off check box), details of the selected server on the right
 * with a "Test connection" button.
 * The widget works on its own copy of the parameters (cf servers() / setServers())
 */
class ServersWidget : public QWidget
{
    Q_OBJECT

public:
    enum class Status {Unknown, Testing, Ok, Failed};

    explicit ServersWidget(QWidget *parent = nullptr);
    ~ServersWidget() override;

    void setServers(const QList<NntpServerParams*> &servers);
    QVector<NntpServerParams> servers() const; //!< servers with a host (the empty ones are ignored)

    void retranslate();

signals:
    void changed(); //!< a parameter has been modified by the user

private slots:
    void onCurrentRowChanged(int row);
    void onItemChanged(QListWidgetItem *item);
    void onFormEdited();
    void onSslToggled(bool checked);
    void onAdd();
    void onDuplicate();
    void onRemove();
    void onTest();
    void onTestFinished(bool ok, const QString &msg);
    void onTogglePassword();

private:
    struct Entry {
        NntpServerParams params;
        Status           status = Status::Unknown;
        QString          statusMsg;
    };

    QVector<Entry>  _entries;
    bool            _loading; //!< true while the form is filled by the code

    QListWidget    *_list;
    QPushButton    *_addButton, *_dupButton, *_delButton;
    QStackedWidget *_stack;    //!< page 0: no server, page 1: form
    QLabel         *_emptyLbl;
    QFormLayout    *_form;
    QLineEdit      *_hostEdit;
    QSpinBox       *_portSB;
    QCheckBox      *_sslCB;
    QLineEdit      *_userEdit;
    QLineEdit      *_passEdit;
    QAction        *_showPassAction;
    QSpinBox       *_consSB;
    QCheckBox      *_nzbCheckCB;
    QPushButton    *_testButton;
    QLabel         *_testLbl;
    QLabel         *_hostLbl, *_portLbl, *_userLbl, *_passLbl, *_consLbl;
    ServerTester   *_tester;
    int             _testedRow;

    void _addEntry(const NntpServerParams &params, bool select);
    void _refreshItem(int row);
    void _loadForm(int row);
    void _updateButtons();
    static QIcon _statusIcon(Status status, bool enabled);
};

#endif // SERVERSWIDGET_H
