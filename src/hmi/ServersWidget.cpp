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

#include "ServersWidget.h"

#include <QAction>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QPushButton>
#include <QSpinBox>
#include <QSslSocket>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>

//========================================================================
// ServerTester: opens one NNTP connection, reads the greeting and authenticates
//========================================================================
class ServerTester : public QObject
{
    Q_OBJECT
public:
    explicit ServerTester(QObject *parent = nullptr) : QObject(parent), _socket(nullptr), _step(0)
    {
        _timer.setSingleShot(true);
        connect(&_timer, &QTimer::timeout, this, [this](){ _finish(false, tr("no answer from the server (timeout)")); });
    }
    ~ServerTester() override { _cleanSocket(); }

    bool isRunning() const { return _socket != nullptr; }

    void start(const NntpServerParams &params)
    {
        _cleanSocket();
        _params = params;
        _step   = 0;
        _buffer.clear();
        QSslSocket *sock = new QSslSocket(this);
        _socket = sock;
        connect(sock, &QSslSocket::readyRead, this, &ServerTester::onReadyRead);
        connect(sock, &QAbstractSocket::errorOccurred,
                this, [this](QAbstractSocket::SocketError){
            if (_socket)
                _finish(false, _socket->errorString());
        });
        connect(sock, static_cast<void (QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),
                this, [this](const QList<QSslError> &errors){
            QStringList errs;
            for (const QSslError &e : errors)
                errs << e.errorString();
            _finish(false, tr("SSL error: %1").arg(errs.join(", ")));
        });
        _timer.start(15000);
        if (params.useSSL)
            sock->connectToHostEncrypted(params.host, params.port);
        else
            sock->connectToHost(params.host, params.port);
    }

signals:
    void finished(bool ok, const QString &msg);

private slots:
    void onReadyRead()
    {
        if (!_socket)
            return;
        _buffer += _socket->readAll();
        int idx;
        while (_socket && (idx = _buffer.indexOf("\r\n")) >= 0)
        {
            QString line = QString::fromLatin1(_buffer.left(idx));
            _buffer.remove(0, idx + 2);
            _handleLine(line);
        }
    }

private:
    QSslSocket      *_socket;
    NntpServerParams _params;
    int              _step; //!< 0: greeting, 1: AUTHINFO USER sent, 2: AUTHINFO PASS sent
    QByteArray       _buffer;
    QTimer           _timer;

    void _send(const std::string &cmd)
    {
        _socket->write(cmd.c_str());
        _socket->write("\r\n");
    }

    void _handleLine(const QString &line)
    {
        int code = line.left(3).toInt();
        if (_step == 0)
        {
            if (code != 200 && code != 201)
                return _finish(false, tr("unexpected answer (%1)").arg(code));
            if (!_params.auth || _params.user.empty())
                return _finish(true, tr("connected (no authentication)"));
            _step = 1;
            _send("AUTHINFO USER " + _params.user);
        }
        else if (_step == 1)
        {
            if (code == 281)
                return _finish(true, tr("connected and authenticated"));
            if (code != 381)
                return _finish(false, tr("user refused (%1)").arg(code));
            _step = 2;
            _send("AUTHINFO PASS " + _params.pass);
        }
        else if (_step == 2)
        {
            if (code == 281)
                _finish(true, tr("connected and authenticated"));
            else
                _finish(false, tr("wrong username or password (%1)").arg(code));
        }
    }

    void _finish(bool ok, const QString &msg)
    {
        _timer.stop();
        if (_socket && _socket->state() == QAbstractSocket::ConnectedState)
        {
            _socket->write("QUIT\r\n");
            _socket->flush();
        }
        _cleanSocket();
        emit finished(ok, msg);
    }

    void _cleanSocket()
    {
        if (_socket)
        {
            QSslSocket *sock = _socket;
            _socket = nullptr;
            sock->disconnect(this);
            sock->abort();
            sock->deleteLater();
        }
    }
};

//========================================================================
// ServersWidget
//========================================================================
static constexpr int sDefaultConnections = 20;

ServersWidget::ServersWidget(QWidget *parent) :
    QWidget(parent),
    _entries(), _loading(false),
    _tester(new ServerTester(this)), _testedRow(-1)
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(12);

    // ---- left: list + buttons
    QVBoxLayout *leftLayout = new QVBoxLayout();
    leftLayout->setSpacing(6);
    _list = new QListWidget(this);
    _list->setObjectName("serversList");
    _list->setIconSize(QSize(12, 12));
    _list->setMinimumWidth(190);
    _list->setMaximumWidth(260);
    _list->setSpacing(2);
    leftLayout->addWidget(_list);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);
    _addButton = new QPushButton(QIcon(":/icons/plus.png"), QString(), this);
    _dupButton = new QPushButton(this);
    _delButton = new QPushButton(QIcon(":/icons/clear.png"), QString(), this);
    btnLayout->addWidget(_addButton);
    btnLayout->addWidget(_dupButton);
    btnLayout->addWidget(_delButton);
    leftLayout->addLayout(btnLayout);
    mainLayout->addLayout(leftLayout);

    // ---- right: details of the selected server
    _stack = new QStackedWidget(this);
    _emptyLbl = new QLabel(this);
    _emptyLbl->setAlignment(Qt::AlignCenter);
    _emptyLbl->setWordWrap(true);
    _emptyLbl->setObjectName("hint");
    _stack->addWidget(_emptyLbl);

    QWidget *formPage = new QWidget(this);
    _form = new QFormLayout(formPage);
    _form->setContentsMargins(0, 0, 0, 0);
    _form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _form->setHorizontalSpacing(10);
    _form->setVerticalSpacing(6);

    _hostEdit = new QLineEdit(formPage);
    _hostEdit->setObjectName("serverHostEdit");
    _hostEdit->setPlaceholderText("news.example.com");
    _hostLbl = new QLabel(formPage);
    _form->addRow(_hostLbl, _hostEdit);

    QHBoxLayout *portLayout = new QHBoxLayout();
    _portSB = new QSpinBox(formPage);
    _portSB->setRange(1, 65535);
    _portSB->setMinimumWidth(80);
    _sslCB = new QCheckBox(formPage);
    portLayout->addWidget(_portSB);
    portLayout->addSpacing(12);
    portLayout->addWidget(_sslCB);
    portLayout->addStretch();
    _portLbl = new QLabel(formPage);
    _form->addRow(_portLbl, portLayout);

    _userEdit = new QLineEdit(formPage);
    _userLbl = new QLabel(formPage);
    _form->addRow(_userLbl, _userEdit);

    _passEdit = new QLineEdit(formPage);
    _passEdit->setEchoMode(QLineEdit::Password);
    _showPassAction = _passEdit->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);
    _passLbl = new QLabel(formPage);
    _form->addRow(_passLbl, _passEdit);

    QHBoxLayout *consLayout = new QHBoxLayout();
    _consSB = new QSpinBox(formPage);
    _consSB->setRange(1, 500);
    _consSB->setMinimumWidth(80);
    _nzbCheckCB = new QCheckBox(formPage);
    consLayout->addWidget(_consSB);
    consLayout->addSpacing(12);
    consLayout->addWidget(_nzbCheckCB);
    consLayout->addStretch();
    _consLbl = new QLabel(formPage);
    _form->addRow(_consLbl, consLayout);

    QHBoxLayout *testLayout = new QHBoxLayout();
    _testButton = new QPushButton(formPage);
    _testLbl = new QLabel(formPage);
    _testLbl->setWordWrap(false);
    _testLbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    testLayout->addWidget(_testButton);
    testLayout->addWidget(_testLbl, 1);
    _form->addRow(QString(), testLayout);

    _stack->addWidget(formPage);
    mainLayout->addWidget(_stack, 1);

    // ---- connections
    connect(_list, &QListWidget::currentRowChanged, this, &ServersWidget::onCurrentRowChanged);
    connect(_list, &QListWidget::itemChanged,       this, &ServersWidget::onItemChanged);
    connect(_addButton, &QAbstractButton::clicked,  this, &ServersWidget::onAdd);
    connect(_dupButton, &QAbstractButton::clicked,  this, &ServersWidget::onDuplicate);
    connect(_delButton, &QAbstractButton::clicked,  this, &ServersWidget::onRemove);
    connect(_testButton, &QAbstractButton::clicked, this, &ServersWidget::onTest);
    connect(_showPassAction, &QAction::triggered,   this, &ServersWidget::onTogglePassword);
    connect(_sslCB, &QAbstractButton::toggled,      this, &ServersWidget::onSslToggled);

    connect(_hostEdit, &QLineEdit::textEdited, this, &ServersWidget::onFormEdited);
    connect(_userEdit, &QLineEdit::textEdited, this, &ServersWidget::onFormEdited);
    connect(_passEdit, &QLineEdit::textEdited, this, &ServersWidget::onFormEdited);
    connect(_portSB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ServersWidget::onFormEdited);
    connect(_consSB, QOverload<int>::of(&QSpinBox::valueChanged), this, &ServersWidget::onFormEdited);
    connect(_nzbCheckCB, &QAbstractButton::toggled, this, &ServersWidget::onFormEdited);

    connect(_tester, &ServerTester::finished, this, &ServersWidget::onTestFinished);

    retranslate();
    _updateButtons();
}

ServersWidget::~ServersWidget() = default;

void ServersWidget::retranslate()
{
    _addButton->setText(tr("Add"));
    _addButton->setToolTip(tr("Add a server (you can use as many as you want)"));
    _dupButton->setText(tr("Duplicate"));
    _dupButton->setToolTip(tr("Copy the selected server (same provider, other host or port...)"));
    _delButton->setText(tr("Remove"));
    _delButton->setToolTip(tr("Remove the selected server"));
    _list->setToolTip(tr("Tick a server to use it, untick it to keep it without using it"));
    _emptyLbl->setText(tr("No server yet.\nClick on \"Add\" to configure your Usenet provider."));
    _hostLbl->setText(tr("Host"));
    _portLbl->setText(tr("Port"));
    _sslCB->setText(tr("SSL / TLS"));
    _sslCB->setToolTip(tr("Encrypted connection (usually on port 563)"));
    _userLbl->setText(tr("Username"));
    _passLbl->setText(tr("Password"));
    _showPassAction->setToolTip(tr("Show / hide the password"));
    _consLbl->setText(tr("Connections"));
    _consSB->setToolTip(tr("Don't use more connections than your subscription allows"));
    _nzbCheckCB->setText(tr("use to check the nzb"));
    _nzbCheckCB->setToolTip(tr("Server used by the nzb check (--check)"));
    _testButton->setText(tr("Test connection"));
    _testButton->setToolTip(tr("Connect to the server and check the username and password"));
    for (int row = 0; row < _entries.size(); ++row)
        _refreshItem(row);
    if (_list->currentRow() >= 0)
        _loadForm(_list->currentRow());
}

void ServersWidget::setServers(const QList<NntpServerParams *> &servers)
{
    _loading = true;
    _list->clear();
    _entries.clear();
    for (const NntpServerParams *srv : servers)
        _addEntry(*srv, false);
    _loading = false;
    if (!_entries.isEmpty())
        _list->setCurrentRow(0);
    else
        onCurrentRowChanged(-1);
    _updateButtons();
}

QVector<NntpServerParams> ServersWidget::servers() const
{
    QVector<NntpServerParams> res;
    for (const Entry &e : _entries)
    {
        if (e.params.host.trimmed().isEmpty())
            continue;
        NntpServerParams p = e.params;
        p.host = p.host.trimmed();
        p.auth = !p.user.empty();
        res << p;
    }
    return res;
}

void ServersWidget::_addEntry(const NntpServerParams &params, bool select)
{
    Entry e;
    e.params = params;
    _entries << e;

    bool loading = _loading;
    _loading = true;
    QListWidgetItem *item = new QListWidgetItem(_list);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    _refreshItem(_entries.size() - 1);
    _loading = loading;

    if (select)
        _list->setCurrentRow(_entries.size() - 1);
}

QIcon ServersWidget::_statusIcon(Status status, bool enabled)
{
    QColor color("#9e9e9e"); // unknown / disabled: grey
    if (enabled)
    {
        switch (status) {
        case Status::Ok:      color = QColor("#2e9e44"); break; // green
        case Status::Failed:  color = QColor("#d13438"); break; // red
        case Status::Testing: color = QColor("#e8a33d"); break; // orange
        default: break;
        }
    }
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    if (enabled)
        p.drawEllipse(QRectF(3, 3, 18, 18));
    else
    {
        p.setPen(QPen(color, 3));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QRectF(4.5, 4.5, 15, 15));
    }
    return QIcon(pix);
}

void ServersWidget::_refreshItem(int row)
{
    QListWidgetItem *item = _list->item(row);
    if (!item || row >= _entries.size())
        return;
    const Entry &e = _entries.at(row);
    bool loading = _loading;
    _loading = true;

    QString host = e.params.host.trimmed().isEmpty() ? tr("(new server)") : e.params.host.trimmed();
    QStringList details;
    if (!e.params.user.empty())
        details << QString::fromStdString(e.params.user);
    details << tr("%1 conn.").arg(e.params.nbCons);
    if (e.params.useSSL)
        details << "SSL";
    item->setText(QString("%1\n%2").arg(host, details.join(" · ")));
    item->setCheckState(e.params.enabled ? Qt::Checked : Qt::Unchecked);
    item->setIcon(_statusIcon(e.status, e.params.enabled));

    QString tip = e.params.enabled ? tr("used for posting") : tr("disabled (not used)");
    if (!e.statusMsg.isEmpty())
        tip += QString("\n%1").arg(e.statusMsg);
    item->setToolTip(tip);

    _loading = loading;
}

void ServersWidget::_loadForm(int row)
{
    if (row < 0 || row >= _entries.size())
        return;
    const Entry &e = _entries.at(row);
    _loading = true;
    _hostEdit->setText(e.params.host);
    _portSB->setValue(e.params.port);
    _sslCB->setChecked(e.params.useSSL);
    _userEdit->setText(QString::fromStdString(e.params.user));
    _passEdit->setText(QString::fromStdString(e.params.pass));
    _passEdit->setEchoMode(QLineEdit::Password);
    _consSB->setValue(e.params.nbCons);
    _nzbCheckCB->setChecked(e.params.nzbCheck);
    switch (e.status) {
    case Status::Ok:      _testLbl->setText(QString("<span style='color:#2e9e44;'>✔ %1</span>").arg(e.statusMsg.toHtmlEscaped())); break;
    case Status::Failed:  _testLbl->setText(QString("<span style='color:#d13438;'>✖ %1</span>").arg(e.statusMsg.toHtmlEscaped())); break;
    case Status::Testing: _testLbl->setText(tr("testing...")); break;
    default:              _testLbl->clear(); break;
    }
    _testButton->setEnabled(e.status != Status::Testing);
    _loading = false;
}

void ServersWidget::_updateButtons()
{
    bool hasSelection = _list->currentRow() >= 0;
    _dupButton->setEnabled(hasSelection);
    _delButton->setEnabled(hasSelection);
}

void ServersWidget::onCurrentRowChanged(int row)
{
    _stack->setCurrentIndex(row >= 0 ? 1 : 0);
    _loadForm(row);
    _updateButtons();
}

void ServersWidget::onItemChanged(QListWidgetItem *item)
{
    if (_loading)
        return;
    int row = _list->row(item);
    if (row < 0 || row >= _entries.size())
        return;
    bool enabled = item->checkState() == Qt::Checked;
    if (enabled != _entries[row].params.enabled)
    {
        _entries[row].params.enabled = enabled;
        _refreshItem(row);
        emit changed();
    }
}

void ServersWidget::onFormEdited()
{
    if (_loading)
        return;
    int row = _list->currentRow();
    if (row < 0 || row >= _entries.size())
        return;
    Entry &e = _entries[row];
    e.params.host     = _hostEdit->text();
    e.params.port     = static_cast<ushort>(_portSB->value());
    e.params.useSSL   = _sslCB->isChecked();
    e.params.user     = _userEdit->text().toStdString();
    e.params.pass     = _passEdit->text().toStdString();
    e.params.auth     = !e.params.user.empty();
    e.params.nbCons   = _consSB->value();
    e.params.nzbCheck = _nzbCheckCB->isChecked();
    if (e.status != Status::Testing)
    {   // the previous test result is not valid anymore
        e.status = Status::Unknown;
        e.statusMsg.clear();
        _testLbl->clear();
    }
    _refreshItem(row);
    emit changed();
}

void ServersWidget::onSslToggled(bool checked)
{
    if (_loading)
        return;
    // switch the standard ports automatically
    if (checked && _portSB->value() == NntpServerParams::sDefaultPort)
        _portSB->setValue(NntpServerParams::sDefaultSslPort);
    else if (!checked && _portSB->value() == NntpServerParams::sDefaultSslPort)
        _portSB->setValue(NntpServerParams::sDefaultPort);
    onFormEdited();
}

void ServersWidget::onAdd()
{
    NntpServerParams params;
    params.useSSL = true;
    params.port   = NntpServerParams::sDefaultSslPort;
    params.nbCons = sDefaultConnections;
    params.enabled = true;
    _addEntry(params, true);
    _hostEdit->setFocus();
    emit changed();
}

void ServersWidget::onDuplicate()
{
    int row = _list->currentRow();
    if (row < 0 || row >= _entries.size())
        return;
    NntpServerParams params = _entries.at(row).params;
    _addEntry(params, true);
    emit changed();
}

void ServersWidget::onRemove()
{
    int row = _list->currentRow();
    if (row < 0 || row >= _entries.size())
        return;
    if (row == _testedRow)
        _testedRow = -1; // the result of the test will be ignored
    else if (row < _testedRow)
        --_testedRow;
    _loading = true;
    delete _list->takeItem(row);
    _entries.removeAt(row);
    _loading = false;
    onCurrentRowChanged(_list->currentRow());
    emit changed();
}

void ServersWidget::onTest()
{
    int row = _list->currentRow();
    if (row < 0 || row >= _entries.size())
        return;
    Entry &e = _entries[row];
    if (e.params.host.trimmed().isEmpty())
    {
        _testLbl->setText(QString("<span style='color:#d13438;'>✖ %1</span>").arg(tr("the host is empty")));
        return;
    }
    if (_testedRow >= 0 && _testedRow < _entries.size())
    {   // only one test at a time: cancel the previous one
        _entries[_testedRow].status = Status::Unknown;
        _refreshItem(_testedRow);
    }
    _testedRow = row;
    e.status = Status::Testing;
    e.statusMsg.clear();
    _refreshItem(row);
    _loadForm(row);
    NntpServerParams params = e.params;
    params.host = params.host.trimmed();
    params.auth = !params.user.empty();
    _tester->start(params);
}

void ServersWidget::onTestFinished(bool ok, const QString &msg)
{
    int row = _testedRow;
    _testedRow = -1;
    if (row < 0 || row >= _entries.size())
        return;
    _entries[row].status    = ok ? Status::Ok : Status::Failed;
    _entries[row].statusMsg = msg;
    _refreshItem(row);
    if (_list->currentRow() == row)
        _loadForm(row);
}

void ServersWidget::onTogglePassword()
{
    _passEdit->setEchoMode(_passEdit->echoMode() == QLineEdit::Password ? QLineEdit::Normal : QLineEdit::Password);
}

#include "ServersWidget.moc"
