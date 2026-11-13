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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "PostingWidget.h"
#include "AutoPostWidget.h"
#include "ServersWidget.h"
#include "HistoryWidget.h"
#include "HelpWidget.h"
#include "NgPost.h"
#include "nntp/NntpServerParams.h"
#include "nntp/NntpArticle.h"

#include <QDebug>
#include <QProgressBar>
#include "GroupsEdit.h"
#include <QRegularExpression>
#include <QStyle>
#include <QLabel>
#include <QScreen>
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QSystemTrayIcon>
#include <QSpinBox>
#include <QComboBox>
#include <QFileDialog>
#include <QStorageInfo>
#include <QDateTime>
#include <QSysInfo>
#include <QThread>
#include <QSaveFile>
#include <QStackedWidget>
#include "utils/Diagnostics.h"
#include "utils/LatencyStats.h"
#include "PostingJob.h"
#include "PostQueue.h"
#include "RepostInfo.h"


QColor        MainWindow::sPostingColor = QColor(255,162, 0); // gold (#FFA200)
const QString MainWindow::sPostingIcon  = ":/icons/uploading.png";
QColor        MainWindow::sPendingColor = Qt::darkBlue;
const QString MainWindow::sPendingIcon  = ":/icons/pending.png";
QColor        MainWindow::sDoneOKColor  = Qt::darkGreen;
const QString MainWindow::sDoneOKIcon   = ":/icons/ok.png";
QColor        MainWindow::sDoneKOColor  = Qt::darkRed;
const QString MainWindow::sDoneKOIcon   = ":/icons/ko.png";
QColor        MainWindow::sArticlesFailedColor  = Qt::darkYellow;

void MainWindow::setThemeColors(bool dark)
{
    sPostingColor        = dark ? QColor("#ffb900") : QColor("#c87800");
    sPendingColor        = dark ? QColor("#6cb8ff") : QColor("#0050a0");
    sDoneOKColor         = dark ? QColor("#6ccb5f") : QColor("#107c10");
    sDoneKOColor         = dark ? QColor("#ff99a4") : QColor("#c42b1c");
    sArticlesFailedColor = dark ? QColor("#fce100") : QColor("#9d5d00");
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow),
    _ngPost(nullptr),
    _state(STATE::IDLE),
    _quickJobTab(nullptr),
    _autoPostTab(nullptr),
    _servers(nullptr),
    _autoSaveTimer(new QTimer(this)),
    _autoSaveReady(false),
    _autoSaveQuickTab(false),
    _autoSaveAutoTab(false),
    _pages(nullptr), _nav(nullptr), _navButtons{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    _history(nullptr), _help(nullptr), _jobNumber(-1), _pauseIcon(true), _historyCB(nullptr), _duplicatesCB(nullptr), _historyPathEdit(nullptr), _historyPathButton(nullptr), _historyHint(nullptr), _queueCB(nullptr), _queueHint(nullptr), _supportHint(nullptr), _supportButton(nullptr),
    _themeButton(nullptr), _aboutButton(nullptr),
    _postsTitle(nullptr), _serversTitle(nullptr), _serversSubtitle(nullptr),
    _settingsTitle(nullptr), _settingsSubtitle(nullptr), _appVersion(nullptr),
    _themeCB(nullptr), _themeLbl(nullptr), _autoSaveHint(nullptr), _cardTitles()
{
    _autoSaveTimer->setSingleShot(true);
    _autoSaveTimer->setInterval(sAutoSaveDelayMs);
    connect(_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSave);
    setAcceptDrops(true);

    _ui->setupUi(this);
    _buildShell();

    _ui->postSplitter->setStretchFactor(0, 3);
    _ui->postSplitter->setStretchFactor(1, 1);
    _ui->postSplitter->setCollapsible(0, false);


    _ui->progressBar->setRange(0, 100);
    updateProgress(ProgressInfo());

    QSize screenSize = qApp->screens()[0]->size();
    resize(screenSize * 0.8);
    setWindowIcon(QIcon(":/icons/ngPost.png"));
    setGeometry((screenSize.width() - width())/2,  (screenSize.height() - height())/2, width(), height());

    connect(_ui->clearLogButton, &QAbstractButton::clicked, _ui->logBrowser, &QTextEdit::clear);
    connect(_ui->debugBox,       &QAbstractButton::toggled, this,            &MainWindow::onDebugToggled);
    connect(_ui->pauseButton,    &QAbstractButton::clicked, this,            &MainWindow::onPauseClicked);

#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    connect(_ui->debugSB, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onDebugValue);
#else
    connect(_ui->debugSB,   qOverload<int>(&QSpinBox::valueChanged),   this,    &MainWindow::onDebugValue);
#endif
}

MainWindow::~MainWindow()
{
    delete _ui;
}

void MainWindow::init(NgPost *ngPost)
{
    _ngPost = ngPost;

    _quickJobTab = new PostingWidget(ngPost, this, 1);
    _autoPostTab = new AutoPostWidget(ngPost, this);

    _ui->debugBox->setChecked(_ngPost->debugMode());
    _ui->debugSB->setEnabled(_ngPost->debugMode());

    QTabBar *tabBar = _ui->postTabWidget->tabBar();
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    tabBar->setElideMode(Qt::TextElideMode::ElideNone);
    tabBar->setIconSize({18, 18});

    _ui->postTabWidget->clear();
    _ui->postTabWidget->setUsesScrollButtons(true);
    _ui->postTabWidget->addTab(_quickJobTab, _tabIcon(":/icons/quick.png"), _ngPost->quickJobName());
    tabBar->setTabData(0, ":/icons/quick.png");
    tabBar->setTabToolTip(0, tr("Default %1").arg(_ngPost->quickJobName()));
    _ui->postTabWidget->addTab(_autoPostTab, _tabIcon(":/icons/auto.png"), _ngPost->folderMonitoringName());
    tabBar->setTabData(1, ":/icons/auto.png");
    tabBar->setTabToolTip(1, _ngPost->folderMonitoringName());
    _ui->postTabWidget->addTab(new QWidget(_ui->postTabWidget), _tabIcon(":/icons/plus.png"), tr("New"));
    tabBar->setTabData(2, ":/icons/plus.png");
    tabBar->setTabToolTip(2, QString("Create a new %1").arg(_ngPost->quickJobName()));

//    connect(_ui->postTabWidget,           &QTabWidget::currentChanged, this, &MainWindow::onJobTabClicked);
    connect(tabBar, &QTabBar::tabBarClicked,              this, &MainWindow::onJobTabClicked);
    connect(tabBar, &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);
    connect(tabBar, &QTabBar::tabCloseRequested,          this, &MainWindow::onCloseJob);
    _ui->postTabWidget->setTabsClosable(true);
    // the Quick Post, Auto Posting and New tabs can't be closed: no close button
    for (int i = 0; i < 3; ++i)
    {
        if (QWidget *closeButton = tabBar->tabButton(i, QTabBar::RightSide))
        {
            tabBar->setTabButton(i, QTabBar::RightSide, nullptr);
            closeButton->deleteLater();
        }
    }
    _ui->postTabWidget->installEventFilter(this);
//    _ui->postTabWidget->setCurrentIndex(1);

    setJobLabel(-1);


    for (const QString &lang : _ngPost->languages())
        _ui->langCB->addItem(QIcon(QString(":/icons/flag_%1.png").arg(lang.toUpper())), lang.toUpper(), lang);
    _ui->langCB->setCurrentText(_ngPost->_lang.toUpper());
    connect(_ui->langCB, &QComboBox::currentTextChanged, this, &MainWindow::onLangChanged);

    _initServerBox();
    _initPostingBox();
    _quickJobTab->init();
    _autoPostTab->init();

    // autosave: watch the global settings, the servers, the default Quick Post and the Auto Posting tabs
    _watchForAutoSave(_pages->widget(PAGE_SETTINGS), -1);
    _watchForAutoSave(_quickJobTab, 0);
    _watchForAutoSave(_autoPostTab, 1);
    QTimer::singleShot(0, this, [this](){ _autoSaveReady = true; }); // after the initialisation of the widgets

    _ui->goCmdButton->hide();

    // history page
    _history = new HistoryWidget(_ngPost, _pages);
    QWidget *placeholder = _pages->widget(PAGE_HISTORY);
    _pages->insertWidget(PAGE_HISTORY, _history);
    _pages->removeWidget(placeholder);
    placeholder->deleteLater();
    // help page (offline pages embedded in the program)
    _help = new HelpWidget(_pages);
    _pages->insertWidget(PAGE_HELP, _help);
    _help->setLanguage(_ngPost->_lang);
    _historyCB->setChecked(_ngPost->_historyEnabled);
    _updateHistorySettings();
    connect(_historyCB,         &QAbstractButton::toggled, this, &MainWindow::onHistoryToggled);
    _duplicatesCB->setChecked(_ngPost->_checkDuplicates);
    connect(_duplicatesCB,      &QAbstractButton::toggled, this, [this](bool checked){ _ngPost->_checkDuplicates = checked; });
    {
        Notifier::Settings &ns = _ngPost->_notifier.settings();
        _notifyOkCB->setChecked(ns.onSuccess);
        _notifyErrCB->setChecked(ns.onErrors);
        _notifyTrayCB->setChecked(ns.tray);
        _notifyWhenCB->setCurrentIndex(ns.allDone ? 0 : 1);
        _notifyDiscordEdit->setText(ns.discord);
        _notifyTgTokenEdit->setText(ns.telegramToken);
        _notifyTgChatEdit->setText(ns.telegramChat);
        _notifyWebhookEdit->setText(ns.webhook);
        connect(_notifyOkCB,   &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_notifier.settings().onSuccess = on; });
        connect(_notifyErrCB,  &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_notifier.settings().onErrors = on; });
        connect(_notifyTrayCB, &QAbstractButton::toggled, this, [this](bool on){ _ngPost->_notifier.settings().tray = on; });
        connect(_notifyWhenCB, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
                [this](int idx){ _ngPost->_notifier.settings().allDone = (idx == 0); });
        connect(_notifyDiscordEdit,   &QLineEdit::textChanged, this, [this](const QString &t){ _ngPost->_notifier.settings().discord = t.trimmed(); });
        connect(_notifyTgTokenEdit,   &QLineEdit::textChanged, this, [this](const QString &t){ _ngPost->_notifier.settings().telegramToken = t.trimmed(); });
        connect(_notifyTgChatEdit,    &QLineEdit::textChanged, this, [this](const QString &t){ _ngPost->_notifier.settings().telegramChat = t.trimmed(); });
        connect(_notifyWebhookEdit,   &QLineEdit::textChanged, this, [this](const QString &t){ _ngPost->_notifier.settings().webhook = t.trimmed(); });
        connect(_notifyTestButton,    &QAbstractButton::clicked, this, [this](){
            QMessageBox::information(this, tr("Notifications"), _ngPost->_notifier.sendTest(_ngPost->appName()));
        });
        connect(&_ngPost->_notifier, &Notifier::trayMessage, this, &MainWindow::_showTrayMessage);
    }
    _queueCB->setChecked(_ngPost->_queueEnabled);
    connect(_queueCB,           &QAbstractButton::toggled, this, &MainWindow::onQueueToggled);
    QTimer::singleShot(1500, this, &MainWindow::_offerQueue); // after the windows of the start
    connect(_historyPathButton, &QAbstractButton::clicked, this, &MainWindow::onHistoryPathClicked);
    if (_ngPost->_historyEnabled && !_ngPost->_historyKeyInConfig)
    {
        // first start with the history: tell the user once (the key "history" is then saved in the config)
        QTimer::singleShot(600, this, [this](){
            QMessageBox::information(this, tr("History of the uploads"),
                tr("ngPost Deluxe now keeps the history of your uploads (History page).\n\n"
                   "Be careful: the history file also records the password of each archive in clear text, "
                   "so you can always open your releases again. It stays on this computer and is never sent "
                   "anywhere, but anyone using this computer can read it.\n\n"
                   "File: %1\n\nYou can turn the history off in the Settings.").arg(
                    QDir::toNativeSeparators(_ngPost->historyFile())));
            _ngPost->_historyKeyInConfig = true;
            scheduleAutoSave();
        });
    }

    _themeCB->setCurrentIndex(_ngPost->theme() == "light" ? 1 : _ngPost->theme() == "dark" ? 2 : 0);
    connect(_themeCB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onThemeSelected);
    connect(_ui->postTabWidget, &QTabWidget::currentChanged, this, &MainWindow::onPostTabChanged);
    _appVersion->setText(QString("v%1").arg(NgPost::versionLabel()));
    _retranslateShell();
    onThemeApplied();
    _fixIconButtons();
    _ui->postSplitter->setSizes({3000, 1000}); // 3/4 for the post, 1/4 for the log
//    connect(_ui->goCmdButton, &QAbstractButton::clicked, _ngPost, &NgPost::onGoCMD, Qt::QueuedConnection);

    updateProgress(ProgressInfo());
}


void MainWindow::updateProgress(const ProgressInfo &info)
{
    QProgressBar *bar = _ui->progressBar;

    // bar: busy (animated) while the files are packed, then the Articles
    if (info.phase == ProgressInfo::COMPRESSING || info.phase == ProgressInfo::PAR2
            || (info.phase == ProgressInfo::UPLOADING && info.nbTotal == 0))
        bar->setRange(0, 0);
    else
    {
        bar->setRange(0, static_cast<int>(qMax(info.nbTotal, 1u)));
        bar->setValue(static_cast<int>(info.nbUploaded));
    }

    // color of the bar (cf theme.qss)
    QString state;
    if (info.paused)
        state = "paused";
    else if (info.phase == ProgressInfo::DONE)
        state = info.nbFailed ? "warn" : "done";
    else if (info.phase == ProgressInfo::ABORTED)
        state = "error";
    if (bar->property("state").toString() != state)
    {
        bar->setProperty("state", state);
        bar->style()->unpolish(bar);
        bar->style()->polish(bar);
    }

    // text: the least important details are dropped when the window is narrow
    QStringList parts = NgPost::progressParts(info);
    const QString sep = QString::fromUtf8("  \u00b7  ");
    int maxWidth = qMax(260, width() * 45 / 100);
    QString text;
    for (int nb = parts.size() ; nb > 0 ; --nb)
    {
        text = QStringList(parts.mid(0, nb)).join(sep);
        if (_ui->uploadLbl->fontMetrics().horizontalAdvance(text) <= maxWidth)
            break;
    }
    _ui->uploadLbl->setText(text);
    _ui->uploadLbl->setToolTip(parts.join("\n"));

    // percentage in the title of the window (visible when it is minimized)
    QString title = windowTitle();
    title.remove(QRegularExpression(QString::fromUtf8(" \u2013 \\d+%$")));
    if (info.phase == ProgressInfo::UPLOADING && info.nbTotal)
        title += QString::fromUtf8(" \u2013 %1%").arg(static_cast<int>(100. * info.nbUploaded / info.nbTotal));
    if (title != windowTitle())
        setWindowTitle(title);
}

void MainWindow::log(const QString &aMsg, bool newline, int level) const
{
    if (newline && level != NgPost::LOG_INFO)
    {
        // colors readable on both the light and the dark theme
        bool dark = _ui->logBrowser->palette().base().color().lightness() < 128;
        QString color;
        bool bold = false;
        switch (level)
        {
        case NgPost::LOG_PHASE: color = dark ? "#6cb0ff" : "#1a5fb4"; bold = true; break;
        case NgPost::LOG_OK:    color = dark ? "#5fd38a" : "#1e7e34"; break;
        case NgPost::LOG_WARN:  color = dark ? "#ffb454" : "#b45f06"; break;
        default:                color = dark ? "#9a9a9a" : "#7a7a7a"; break; // LOG_DEBUG
        }
        _ui->logBrowser->append(QString("<span style='color:%1;%2'>%3</span>").arg(
                                    color, bold ? "font-weight:bold;" : "",
                                    aMsg.toHtmlEscaped().replace("\n", "<br/>")));
    }
    else if (newline)
        _ui->logBrowser->append(aMsg);
    else
    {
        _ui->logBrowser->insertPlainText(aMsg);
        _ui->logBrowser->moveCursor(QTextCursor::End);
    }
}

void MainWindow::logError(const QString &error) const
{
    _ui->logBrowser->append(QString("<font color='red'>%1</font><br/>\n").arg(error));
}

bool MainWindow::useFixedPassword() const
{
    return _ui->rarPassCB->isChecked();
}

bool MainWindow::hasAutoCompress() const
{
    return _ui->autoCompressCB->isChecked();
}

#include <QKeyEvent>
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && obj == _ui->postTabWidget)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        qDebug() << "[MainWindow] getting key event: " << keyEvent->key();
        int currentTabIdx = _ui->postTabWidget->currentIndex();
        if (currentTabIdx == 1)
            static_cast<AutoPostWidget*>(_ui->postTabWidget->currentWidget())->handleKeyEvent(keyEvent);
        else if (currentTabIdx < _ui->postTabWidget->count() - 1)
            static_cast<PostingWidget*>(_ui->postTabWidget->currentWidget())->handleKeyEvent(keyEvent);
    }
    return QObject::eventFilter(obj, event);
}

#include <QMimeData>
void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    int currentTabIdx = _ui->postTabWidget->currentIndex();
    if (currentTabIdx == 1)
        _autoPostTab->handleDropEvent(e);
    else if (currentTabIdx < _ui->postTabWidget->count() - 1)
        static_cast<PostingWidget*>(_ui->postTabWidget->currentWidget())->handleDropEvent(e);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (_autoSaveTimer->isActive()) // a change not saved yet
    {
        _autoSaveTimer->stop();
        onAutoSave();
    }
    if (_ngPost->hasPostingJobs())
    {
        int res = QMessageBox::question(this,
                                        tr("close while still posting?"),
                                        tr("ngPost is currently posting.\nAre you sure you want to quit?"),
                                        QMessageBox::Yes,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
        {
            _ngPost->closeAllPostingJobs();
            event->accept();
        }
        else
            event->ignore();

    }
    else
        event->accept();
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event)
    {
        QTabBar *tabBar = _ui->postTabWidget->tabBar();
        int lastTabIdx = tabBar->count() - 1;
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            qDebug() << "MainWindow::changeEvent";
            _ui->retranslateUi(this);
            _ui->shutdownCB->setToolTip(tr("Shutdown computer when all the current Posts are done (with command: %1)").arg(
                                            _ngPost->_shutdownCmd));

            _ui->fileBox->setTitle(QString());
            _ui->logBox->setTitle(tr("Log"));
            _retranslateShell();

            tabBar->setTabText(0, _ngPost->quickJobName());
            tabBar->setTabToolTip(0, tr("Default %1").arg(_ngPost->quickJobName()));
            tabBar->setTabText(1, _ngPost->folderMonitoringName());
            tabBar->setTabToolTip(1, _ngPost->folderMonitoringName());
            for (int i = 2 ; i < lastTabIdx; ++i)
                tabBar->setTabText(i, _ngPost->quickJobName());
            tabBar->setTabText(lastTabIdx, tr("New"));
            tabBar->setTabToolTip(2, QString("Create a new %1").arg(_ngPost->quickJobName()));

            setJobLabel(_jobNumber);

            _servers->retranslate();


            _quickJobTab->retranslate();
            _autoPostTab->retranslate();
            for (int i = 2 ; i < _ui->postTabWidget->count() - 1; ++i)
                _getPostWidget(i)->retranslate();
            _fixIconButtons();
            break;

            // this event is send, if the system, language changes
        default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}



void MainWindow::onObfucateToggled(bool checked)
{
    bool enabled = !checked;
    _ui->fromEdit->setEnabled(enabled);
    _ui->genPoster->setEnabled(enabled);
    _ui->saveFromCB->setEnabled(enabled);
    _ui->uniqueFromCB->setEnabled(enabled);
}

void MainWindow::onTabContextMenu(const QPoint &point)
{
//    qDebug() << "MainWindow::onTabContextMenu: " << point;
    if (point.isNull())
        return;

//    QTabBar *tabBar = _ui->postTabWidget->tabBar();
//    int tabIndex = tabBar->tabAt(point);
//    PostingWidget *currentPostWidget = _getPostWidget(tabIndex);
    QMenu menu(tr("Quick Tabs Menu"), this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QAction *action = menu.addAction(QIcon(":/icons/clear.png"), tr("Close All finished Tabs"), this, &MainWindow::onCloseAllFinishedQuickTabs);
#else
    QAction *action = menu.addAction(QIcon(":/icons/clear.png"), tr("Close All finished Tabs"), this, SLOT(onCloseAllFinishedQuickTabs));
#endif
    action->setEnabled(hasFinishedPosts());
    menu.exec(QCursor::pos());
}

bool MainWindow::hasFinishedPosts() const
{
    for (int idx = 2 ; idx < _ui->postTabWidget->count() - 2 ; ++idx)
    {
        PostingWidget *postWidget = _getPostWidget(idx);
        if (postWidget && postWidget->isPostingFinished())
            return true;
    }
    return false;
}

void MainWindow::onCloseAllFinishedQuickTabs()
{
    // go backwards as we may delete the current tab ;)
    for (int idx = _ui->postTabWidget->count() - 2 ; idx > 1  ; --idx)
    {
        PostingWidget *postWidget = _getPostWidget(idx);
        if (postWidget && postWidget->isPostingFinished())
            onCloseJob(idx);
    }
}

void MainWindow::onSetProgressBarRange(int nbArticles)
{
    qDebug() << "MainWindow::onSetProgressBarRange: " << nbArticles;
    _ui->progressBar->setRange(0, nbArticles);
}


void MainWindow::_initServerBox()
{
    _servers = new ServersWidget(_ui->serverBox);
    _ui->serverBox->layout()->addWidget(_servers);
    QList<NntpServerParams*> servers;
    for (NntpServerParams *srv : _ngPost->_nntpServers)
        servers << srv;
    _servers->setServers(servers);
    connect(_servers, &ServersWidget::changed, this, &MainWindow::scheduleAutoSave);
}


void MainWindow::_initPostingBox()
{
    connect(_ui->shutdownCB,        &QAbstractButton::toggled, this, &MainWindow::onShutdownToggled);
    connect(_ui->saveButton,        &QAbstractButton::clicked, this, &MainWindow::onSaveConfig);
    connect(_ui->diagReportButton,  &QAbstractButton::clicked, this, &MainWindow::onDiagnosticReport);
    _ui->diagReportButton->setToolTip(tr("Save a report of what ngPost did (durations, speeds, errors, anomalies) "
                                         "to analyse a problem. Passwords, usernames and file names are masked."));

    connect(_ui->genPoster,         &QAbstractButton::clicked, this, &MainWindow::onGenPoster);
    connect(_ui->obfuscateMsgIdCB,  &QAbstractButton::toggled, this, &MainWindow::onObfucateToggled);
    connect(_ui->uniqueFromCB,      &QAbstractButton::toggled, this, &MainWindow::onUniqueFromToggled);
    connect(_ui->rarPassCB,         &QAbstractButton::toggled, this, &MainWindow::onRarPassToggled);
    connect(_ui->genPass,           &QAbstractButton::clicked, this, &MainWindow::onArchivePass);
    connect(_ui->autoCompressCB,    &QAbstractButton::toggled, this, &MainWindow::onAutoCompressToggled);
    connect(_ui->rarPassEdit,       &QLineEdit::textChanged,   this, &MainWindow::onRarPassUpdated);

    _ui->fromEdit->setText(_ngPost->xml2txt(_ngPost->_from.c_str()));
    _groupsEdit->setGroups(_ngPost->_grpList);
    _groupPolicyCB->setCurrentIndex(qMax(0, _groupPolicyCB->findData(static_cast<int>(_ngPost->_groupPolicy))));
    _ui->uniqueFromCB->setChecked(_ngPost->_genFrom);
    _ui->saveFromCB->setChecked(_ngPost->_saveFrom);

    _ui->rarLengthSB->setValue(static_cast<int>(_ngPost->_lengthPass));
    if (_ngPost->_rarPassFixed.isEmpty())
    {
        _ui->rarPassCB->setChecked(false);
        onRarPassToggled(false);
    }
    else
    {
	// Issue #48 we should set the text first!
        _ui->rarPassEdit->setText(_ngPost->_rarPassFixed);
        _ui->rarPassCB->setChecked(true);
    }

    _ui->autoCompressCB->setChecked(_ngPost->_packAuto);
    _ui->autoCloseCB->setChecked(_ngPost->_autoCloseTabs);

    _ui->obfuscateMsgIdCB->setChecked(_ngPost->_obfuscateArticles);
    _ui->obfuscateFileNameCB->setChecked(_ngPost->_obfuscateFileName);

    _ui->articleSizeEdit->setText(QString::number(_ngPost->articleSize()));
    _ui->articleSizeEdit->setValidator(new QIntValidator(100000, 10000000, _ui->articleSizeEdit));

    _ui->nbRetrySB->setRange(0, 15);
    _ui->nbRetrySB->setValue(NntpArticle::nbMaxTrySending());

    _ui->threadSB->setRange(0, 50);
    _ui->threadSB->setValue(_ngPost->_nbThreads);

    _ui->nzbPathEdit->setText(_ngPost->_nzbPath);
    connect(_ui->nzbPathButton, &QAbstractButton::clicked, this, &MainWindow::onNzbPathClicked);
}

void MainWindow::updateServers()
{
    // The NntpServerParams are updated in place: the connections of an ongoing post keep a reference on them,
    // so they must never be deleted while ngPost is running (the removed ones are kept in _removedServers)
    const QVector<NntpServerParams> servers = _servers->servers();
    QList<NntpServerParams*> &current = _ngPost->_nntpServers;
    int idx = 0;
    for (const NntpServerParams &srv : servers)
    {
        if (idx < current.size())
            *current[idx] = srv;
        else
            current << new NntpServerParams(srv);
        ++idx;
    }
    while (current.size() > idx)
        _ngPost->_removedServers << current.takeLast();
}

void MainWindow::updateParams()
{
    QString from = _ui->fromEdit->text();
    if (!from.isEmpty())
    {
        QRegularExpression email("\\w+@\\w+\\.\\w+");
        if (!email.match(from).hasMatch())
            from += QString("@%1.com").arg(_ngPost->aticleSignature().c_str());
        _ngPost->_from   = _ngPost->escapeXML(from).toStdString();
    }
    _ngPost->_genFrom  = _ui->uniqueFromCB->isChecked();
    _ngPost->_saveFrom = _ui->saveFromCB->isChecked();

    if (_ui->rarPassCB->isChecked())
    {
        _ngPost->_rarPassFixed = _ui->rarPassEdit->text();
        _ngPost->_rarPass      = _ngPost->_rarPassFixed;
    }

    _ngPost->enableAutoPacking(_ui->autoCompressCB->isChecked());
    _ngPost->_autoCloseTabs = _ui->autoCloseCB->isChecked();

    _ngPost->updateGroups(_groupsEdit->groups().join(","));
    _ngPost->_groupPolicy = static_cast<NgPost::GROUP_POLICY>(_groupPolicyCB->currentData().toInt());

    _ngPost->_obfuscateArticles = _ui->obfuscateMsgIdCB->isChecked();
    _ngPost->_obfuscateFileName = _ui->obfuscateFileNameCB->isChecked();

    bool ok = false;
    uint articleSize = _ui->articleSizeEdit->text().toUInt(&ok);
    if (ok)
        NgPost::sArticleSize = articleSize;

    NntpArticle::setNbMaxRetry(static_cast<ushort>(_ui->nbRetrySB->value()));

    _ngPost->_nbThreads = _ui->threadSB->value();
    if (_ngPost->_nbThreads < 1)
        _ngPost->_nbThreads = 1;

    QFileInfo nzbPath(_ui->nzbPathEdit->text());
    if (nzbPath.exists() && nzbPath.isDir() && nzbPath.isWritable())
        _ngPost->_nzbPath = nzbPath.absoluteFilePath();
}

void MainWindow::updateAutoPostingParams()
{
    updateServers();
    updateParams();
    _autoPostTab->udatePostingParams();
}

QString MainWindow::fixedArchivePassword() const
{
    if (_ui->rarPassCB->isChecked())
    {
        QString pass = _ui->rarPassEdit->text();
        if (!pass.isEmpty())
            return pass;
    }
    return QString();
}

PostingWidget *MainWindow::addNewQuickTab(int lastTabIdx, const QFileInfoList &files)
{
    if (!lastTabIdx)
        lastTabIdx = _ui->postTabWidget->count() -1;
    PostingWidget *newPostingWidget = new PostingWidget(_ngPost, this, static_cast<uint>(lastTabIdx));
    newPostingWidget->init();
    QString tabName = QString("%1 #%2").arg(_ngPost->quickJobName()).arg(lastTabIdx);
    _ui->postTabWidget->insertTab(lastTabIdx,
                                  newPostingWidget ,
                                  _tabIcon(":/icons/quick.png"),
                                  tabName);
    _ui->postTabWidget->tabBar()->setTabData(lastTabIdx, ":/icons/quick.png");
    _ui->postTabWidget->setTabToolTip(lastTabIdx, tabName);

    for (const QFileInfo &file : files)
        newPostingWidget->addPath(file.absoluteFilePath(), 0, file.isDir());

    _fixIconButtons();
    return newPostingWidget;
}

void MainWindow::setTab(QWidget *postWidget)
{
    int nbJob = _ui->postTabWidget->count() -1;
    for (int i = 0 ; i < nbJob ; ++i)
    {
        if (_ui->postTabWidget->widget(i) == postWidget)
        {
            _ui->postTabWidget->setCurrentIndex(i);
            break;
        }
    }
}

void MainWindow::clearJobTab(QWidget *postWidget)
{
    int nbJob = _ui->postTabWidget->count() -1;
    for (int i = 0 ; i < nbJob ; ++i)
    {
        if (_ui->postTabWidget->widget(i) == postWidget)
        {
            QTabBar *bar = _ui->postTabWidget->tabBar();
            bar->setTabToolTip(i, "");
            bar->setTabTextColor(i, _tabNormalColor());
            bar->setTabIcon(i, _tabIcon(":/icons/quick.png"));
            bar->setTabData(i, ":/icons/quick.png");
        }
    }
}

void MainWindow::updateJobTab(QWidget *postWidget, const QColor &color, const QIcon &icon, const QString &tooltip)
{
    int nbJob = _ui->postTabWidget->count() -1;
    for (int i = 0 ; i < nbJob ; ++i)
    {
        if (_ui->postTabWidget->widget(i) == postWidget)
        {
            QTabBar *bar = _ui->postTabWidget->tabBar();
            if (!tooltip.isEmpty())
                bar->setTabToolTip(i, tooltip);
            bar->setTabTextColor(i, color);
            bar->setTabIcon(i, icon);
            bar->setTabData(i, QVariant());
            break;
        }
    }
}

void MainWindow::setJobLabel(int jobNumber)
{
    _jobNumber = jobNumber;
    if (jobNumber < 0)
        _ui->jobLabel->setText(tr("Ready"));
    else if (jobNumber <= 1)
        _ui->jobLabel->setText(_ngPost->quickJobName());
    else
        _ui->jobLabel->setText(QString("%1 #%2").arg(_ngPost->quickJobName()).arg(jobNumber));
}


PostingWidget *MainWindow::_getPostWidget(int tabIndex) const
{
    if(tabIndex > 1 && tabIndex < _ui->postTabWidget->count() - 1)
        return static_cast<PostingWidget*>(_ui->postTabWidget->widget(tabIndex));
    else
        return nullptr;
}

int MainWindow::_getPostWidgetIndex(PostingWidget *postWidget) const
{
    int nbJob = _ui->postTabWidget->count() -1;
    for (int i = 2; i < nbJob ; ++i)
    {
        if (_ui->postTabWidget->widget(i) == postWidget)
            return i;
    }
    return 0;
}



void MainWindow::onGenPoster()
{
    _ui->fromEdit->setText(_ngPost->randomFrom());
}

void MainWindow::onUniqueFromToggled(bool checked)
{
    bool enabled = !checked;
    _ui->genPoster->setEnabled(enabled);
    _ui->fromEdit->setEnabled(enabled);
    _ui->saveFromCB->setEnabled(enabled);
}

void MainWindow::onRarPassToggled(bool checked)
{
    _ui->rarPassEdit->setEnabled(checked);
    _ui->rarLengthSB->setEnabled(checked);
    _ui->genPass->setEnabled(checked);
    if (checked)
        onRarPassUpdated(_ui->rarPassEdit->text());
}

void MainWindow::onRarPassUpdated(const QString &fixedPass)
{
    _ngPost->_rarPassFixed = fixedPass;
//    if (!_quickJobTab->isPosting())
        _quickJobTab->setNzbPassword(fixedPass);
    PostingWidget *currentQuickPost = _getPostWidget(_ui->postTabWidget->currentIndex());
    if (currentQuickPost) //&& !currentQuickPost->isPosting())
        currentQuickPost->setNzbPassword(fixedPass);
}

void MainWindow::onArchivePass()
{
    _ui->rarPassEdit->setText(_ngPost->randomPass(static_cast<uint>(_ui->rarLengthSB->value())));
}

void MainWindow::onAutoCompressToggled(bool checked)
{
    _ngPost->enableAutoPacking(checked);
    _autoPostTab->setPackingAuto(checked, _ngPost->_packAutoKeywords);
    if (!_quickJobTab->isPosting())
        _quickJobTab->setPackingAuto(checked, _ngPost->_packAutoKeywords);
    PostingWidget *currentQuickPost = _getPostWidget(_ui->postTabWidget->currentIndex());
    if (currentQuickPost && !currentQuickPost->isPosting())
        currentQuickPost->setPackingAuto(checked, _ngPost->_packAutoKeywords);
}

void MainWindow::onDebugToggled(bool checked)
{
#ifdef __DEBUG__
    qDebug() << "Debug mode: " << checked;
#endif
    if (checked)
    {
        if (!_ui->debugSB->value())
            _ui->debugSB->setValue(1);
        _ngPost->setDebug(static_cast<ushort>(_ui->debugSB->value()));
    }
    else
        _ngPost->setDebug(0);
    _ui->debugSB->setEnabled(checked);
}

void MainWindow::onDebugValue(int value)
{
    _ngPost->setDebug(static_cast<ushort>(value));
}


void MainWindow::scheduleAutoSave()
{
    if (_autoSaveReady)
        _autoSaveTimer->start(); // restarted at each change: we save 1 sec after the last one
}

void MainWindow::onAutoSave()
{
    _saveConfig(_autoSaveQuickTab, _autoSaveAutoTab);
    _autoSaveQuickTab = false;
    _autoSaveAutoTab  = false;

    // small feedback on the save button
    _ui->saveButton->setText(tr("saved"));
    QTimer::singleShot(1500, this, [this](){ _ui->saveButton->setText(tr("save")); });
}

void MainWindow::_saveConfig(bool quickTab, bool autoTab)
{
    updateServers();
    updateParams();
    if (quickTab)
        _quickJobTab->udatePostingParams();
    if (autoTab)
        _autoPostTab->udatePostingParams();
    _ngPost->saveConfig(true);
}

void MainWindow::onDiagnosticReport()
{
    QString defaultPath = QString("%1/ngPost_report_%2.txt").arg(
                QDir::homePath(), QDateTime::currentDateTime().toString("yyyyMMdd_hhmm"));
    QString path = QFileDialog::getSaveFileName(this, tr("Save the diagnostic report"), defaultPath,
                                                tr("Text files (*.txt)"));
    if (path.isEmpty())
        return;

    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(_diagnosticReport().toUtf8());
        if (file.commit())
        {
            QMessageBox::information(this, tr("Diagnostic report"),
                                     tr("The report has been saved:\n%1\n\nPasswords, usernames and file names are masked.").arg(
                                         QDir::toNativeSeparators(path)));
            return;
        }
    }
    QMessageBox::critical(this, tr("Diagnostic report"), tr("Couldn't write the file %1").arg(QDir::toNativeSeparators(path)));
}

QString MainWindow::_diagnosticReport() const
{
    Diagnostics &diag = Diagnostics::instance();
    // make sure the credentials of the servers are masked
    for (const NntpServerParams *srv : _ngPost->_nntpServers)
    {
        diag.addSecret(QString::fromStdString(srv->pass), "<server password>");
        diag.addSecret(QString::fromStdString(srv->user), "<server user>");
    }
    diag.addSecret(_ngPost->_rarPassFixed, "<fixed archive password>");

    QString r;
    QTextStream out(&r);
    auto title = [&out](const QString &t){ out << "\n==== " << t << " ====\n"; };
    auto yesNo = [](bool b){ return b ? "yes" : "no"; };

    out << "ngPost diagnostic report\n"
        << "generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n"
        << "(passwords, usernames, file names and the home folder are masked)\n";

    title("System");
    out << "program:   " << _ngPost->appName() << " v" << NgPost::versionLabel() << "\n"
        << "OS:        " << QSysInfo::prettyProductName() << " (kernel " << QSysInfo::kernelVersion()
        << ", " << QSysInfo::currentCpuArchitecture() << ")\n"
        << "CPU cores: " << QThread::idealThreadCount() << "\n"
        << "Qt:        " << qVersion() << "\n"
        << "SSL:       " << PostingJob::sslSupportInfo() << "\n"
        << "log file:  " << diag.mask(diag.logFilePath()) << "\n";

    title("Settings");
    out << "compression: " << yesNo(_ngPost->_doCompress) << ", par2: " << yesNo(_ngPost->_doPar2)
        << ", random name: " << yesNo(_ngPost->_genName) << ", random password: " << yesNo(_ngPost->_genPass)
        << ", keep archives: " << yesNo(_ngPost->_keepRar) << "\n"
        << "RAR_PATH:    " << diag.mask(_ngPost->_rarPath) << "\n"
        << "RAR_EXTRA:   " << (_ngPost->_rarArgs.isEmpty() ? "(default)" : diag.mask(_ngPost->_rarArgs)) << "\n"
        << "RAR_SIZE:    " << _ngPost->_rarSize << " MB, RAR_MAX: " << _ngPost->_rarMax
        << " (limit used: " << yesNo(_ngPost->_useRarMax) << ")\n"
        << "PAR2_PATH:   " << diag.mask(_ngPost->_par2Path) << (_ngPost->useParPar() ? " (ParPar)" : "") << "\n"
        << "PAR2_ARGS:   " << (_ngPost->_par2Args.isEmpty() ? "(default)" : diag.mask(_ngPost->_par2Args))
        << ", PAR2_PCT: " << _ngPost->_par2Pct << "%\n"
        << "threads:     " << _ngPost->_nbThreads << ", article size: " << NgPost::articleSize()
        << ", socket timeout: " << _ngPost->_socketTimeOut / 1000 << " s\n"
        << "obfuscation: articles " << yesNo(_ngPost->_obfuscateArticles)
        << ", file names " << yesNo(_ngPost->_obfuscateFileName) << "\n"
        << "prepare packing: " << yesNo(_ngPost->_preparePacking) << "\n";

    title("Servers");
    int nbCons = 0;
    for (const NntpServerParams *srv : _ngPost->_nntpServers)
    {
        out << "- " << srv->host << ":" << srv->port << (srv->useSSL ? " SSL" : "") << ", "
            << srv->nbCons << " connections, " << (srv->enabled ? "enabled" : "disabled")
            << ", auth: " << yesNo(!srv->user.empty()) << "\n";
        if (srv->enabled)
            nbCons += srv->nbCons;
    }
    out << "total connections: " << nbCons << "\n";

    title("Folders and disks");
    auto folder = [&](const QString &name, const QString &path){
        out << name << ": " << (path.isEmpty() ? "(not set)" : diag.mask(path)) << "\n";
        if (path.isEmpty())
            return;
        QFileInfo fi(path);
        QStorageInfo storage(fi.exists() ? path : fi.absolutePath());
        out << "    exists: " << yesNo(fi.exists()) << ", writable: " << yesNo(fi.isWritable());
        if (storage.isValid())
            out << ", free: " << Diagnostics::humanSize(storage.bytesAvailable()) << " / "
                << Diagnostics::humanSize(storage.bytesTotal()) << ", file system: " << storage.fileSystemType();
        out << "\n";
        QString lower = QDir::fromNativeSeparators(path).toLower();
        if (lower.contains("onedrive") || lower.contains("dropbox") || lower.contains("google drive"))
            out << "    WARNING: synced folder (every file written is uploaded by the sync software)\n";
        else if (lower.endsWith("/desktop") || lower.contains("/desktop/"))
            out << "    note: Desktop folder, it may be synced by OneDrive on Windows 11\n";
        if (lower.startsWith("//"))
            out << "    note: network folder\n";
    };
    folder("TMP_DIR", _ngPost->_tmpPath);
    folder("nzbPath", _ngPost->_nzbPath);

    title("Latency (this session)");
    out << LatencyStats::instance().report();

    title("Anomalies (this session)");
    QStringList anomalies = diag.anomalies();
    if (anomalies.isEmpty())
        out << "none\n";
    for (const QString &a : anomalies)
        out << "- " << a << "\n";

    QStringList events = diag.events();
    QStringList errors = events.filter("[ERROR]");
    title(QString("Errors (this session): %1").arg(errors.size()));
    for (int i = qMax(0, errors.size() - 30); i < errors.size(); ++i)
        out << errors.at(i) << "\n";

    title("Events (this session)");
    const int maxEvents = 600;
    if (events.size() > maxEvents)
        out << "(" << events.size() - maxEvents << " older events not shown)\n";
    for (int i = qMax(0, events.size() - maxEvents); i < events.size(); ++i)
        out << events.at(i) << "\n";

    bool closedProperly = true;
    QString previous = diag.previousSessionLog(&closedProperly);
    title("Previous session");
    if (previous.isEmpty())
        out << "(none)\n";
    else
    {
        if (!closedProperly)
            out << "WARNING: the previous session did not end properly (crash or killed)\n";
        QStringList lines = previous.split('\n');
        const int maxLines = 300;
        if (lines.size() > maxLines)
            out << "(" << lines.size() - maxLines << " older lines not shown)\n";
        for (int i = qMax(0, lines.size() - maxLines); i < lines.size(); ++i)
            out << lines.at(i) << "\n";
    }
    out.flush();
    return r;
}

void MainWindow::_watchForAutoSave(QWidget *root, int tab)
{
    // only the signals emitted by a user action (not when the code fills the widgets)
    // the fields that only concern one post (nzb name, archive name and password of a post) are not watched
    static const QStringList perPostFields = {"nzbFileEdit", "nzbPassCB", "nzbPassEdit", "compressNameEdit",
                                              "debugBox", "debugSB"};
    auto changed = [this, tab]() {
        if (tab == 0)
            _autoSaveQuickTab = true;
        else if (tab == 1)
            _autoSaveAutoTab = true;
        scheduleAutoSave();
    };
    for (QLineEdit *w : root->findChildren<QLineEdit*>())
    {
        if (perPostFields.contains(w->objectName()) || qobject_cast<QAbstractSpinBox*>(w->parent()))
            continue;
        connect(w, &QLineEdit::textEdited, this, changed);
        connect(w, &QLineEdit::editingFinished, this, changed); // also after a selection with a "..." button
    }
    for (QCheckBox *w : root->findChildren<QCheckBox*>())
        if (!perPostFields.contains(w->objectName()))
            connect(w, &QAbstractButton::toggled, this, changed);
    for (QSpinBox *w : root->findChildren<QSpinBox*>())
        if (!perPostFields.contains(w->objectName()))
            connect(w, QOverload<int>::of(&QSpinBox::valueChanged), this, changed);
    for (QComboBox *w : root->findChildren<QComboBox*>())
        if (!perPostFields.contains(w->objectName()))
            connect(w, QOverload<int>::of(&QComboBox::currentIndexChanged), this, changed);
}

void MainWindow::onSaveConfig()
{
    _autoSaveTimer->stop();
    updateServers();
    updateParams();
    int currentTabIdx = _ui->postTabWidget->currentIndex();
    if (currentTabIdx == 0)
        _quickJobTab->udatePostingParams();
    else if (currentTabIdx == 1)
        _autoPostTab->udatePostingParams();
    else
    {
        PostingWidget *postWidget = _getPostWidget(currentTabIdx);
        if (postWidget)
            postWidget->udatePostingParams();
    }
    _ngPost->saveConfig();
}

void MainWindow::onJobTabClicked(int index)
{
    int nbJob = _ui->postTabWidget->count() -1;
    qDebug() << "Click on tab: " << index << ", count: " << nbJob;
    if (index == nbJob) // click on the last tab
        addNewQuickTab(nbJob);
}

void MainWindow::onCloseJob(int index)
{
    int nbJob = _ui->postTabWidget->count() -1;
    qDebug() << "onCloseJob on tab: " << index << ", count: " << nbJob;
    if (index > 1 && index < nbJob )
    {
        PostingWidget *postWidget = _getPostWidget(index);
        if (postWidget->isPosting())
        {
            QMessageBox::warning(this,
                                 tr("Quick Post is working.."),
                                 tr("The Quick post is currentling uploading.\n Please Stop it before closing it.."));
        }
        else if (postWidget->hasMissingArticles()
                 && QMessageBox::question(this,
                                          tr("Articles are missing"),
                                          tr("%1 Article(s) of this post couldn't be posted and the nzb file points to them.\n"
                                             "If you close this tab you won't be able to post them again from here.\n\n"
                                             "Close the tab anyway?").arg(postWidget->nbMissingArticles()),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No) == QMessageBox::No)
        {
            // the user keeps the tab to post the missing Articles again
        }
        else
        {
            postWidget->forgetQueue(); // the missing Articles of this tab can't be posted again anymore
            _ui->postTabWidget->removeTab(index);
            delete postWidget;

            if (index == nbJob - 1)
                _ui->postTabWidget->setCurrentIndex(_ui->postTabWidget->count() - 2);
        }
    }
}

void MainWindow::closeTab(PostingWidget *postWidget)
{
    int index = _getPostWidgetIndex(postWidget);
    if (index)
    {
        int nbJob = _ui->postTabWidget->count() -1;
        _ui->postTabWidget->removeTab(index);
        delete postWidget;

        if (index == nbJob - 1)
            _ui->postTabWidget->setCurrentIndex(_ui->postTabWidget->count() - 2);
    }
}


void MainWindow::toBeImplemented()
{
    QMessageBox::information(nullptr, "To be implemented...", "To be implemented...", QMessageBox::Ok);
}

#include <QFileDialog>
void MainWindow::onNzbPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->nzbPathEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
        _ui->nzbPathEdit->setText(path);
}

void MainWindow::onLangChanged(const QString &lang)
{
    qDebug() << "Changing lang to " << lang;
    _ngPost->changeLanguage(lang.toLower());
}

void MainWindow::onShutdownToggled(bool checked)
{
    if (checked)
    {
        int res = QMessageBox::question(this,
                                        tr("Automatic Shutdown?"),
                                        QString("%1\n%2").arg(
                                            tr("You're about to schedule the shutdown of the computer once all the current Postings will be finished")).arg(
                                            tr("Are you sure you want to switch off the computer?")),
                                        QMessageBox::Yes,
                                        QMessageBox::No);
        if (res == QMessageBox::Yes)
            _ngPost->_doShutdownWhenDone = checked;
        else
            _ui->shutdownCB->setChecked(false);
    }
    else
        _ngPost->_doShutdownWhenDone = false;
}

void MainWindow::setPauseIcon(bool pause)
{
    _pauseIcon = pause;
    QColor color = _ngPost && _ngPost->isDarkTheme() ? QColor("#d6d6d6") : QColor("#3b3b3b");
    _ui->pauseButton->setIcon(_navIcon(pause ? ":/icons/nav_pause.png" : ":/icons/nav_play.png", color, color));
    _ui->pauseButton->setToolTip(pause ? tr("Pause the uploads") : tr("Resume the uploads"));
}

void MainWindow::onPauseClicked()
{
    if (_ngPost->isPosting())
    {
        if (_ngPost->isPaused())
            _ngPost->resume();
        else
            _ngPost->pause();
    }
}


//========================================================================
// ngPost Deluxe layout: side bar on the left, one page per section on the right
//========================================================================
#include <QStackedWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QScrollArea>
#include <QGridLayout>
#include <QPainter>
#include <QMenuBar>
#include <QDesktopServices>
#include <QUrl>

QIcon MainWindow::_navIcon(const QString &path, const QColor &normal, const QColor &checked)
{
    QPixmap src(path);
    auto tint = [&src](const QColor &color) {
        QPixmap pix(src.size());
        pix.fill(Qt::transparent);
        QPainter p(&pix);
        p.drawPixmap(0, 0, src);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(pix.rect(), color);
        return pix;
    };
    QIcon icon;
    icon.addPixmap(tint(normal),  QIcon::Normal, QIcon::Off);
    icon.addPixmap(tint(checked), QIcon::Normal, QIcon::On);
    icon.addPixmap(tint(checked), QIcon::Active, QIcon::On);
    return icon;
}

QColor MainWindow::_tabNormalColor() const
{
    bool dark = _ngPost && _ngPost->isDarkTheme();
    return dark ? QColor("#d6d6d6") : QColor("#3b3b3b");
}

// black icons of the tabs (quick, auto, new): tinted with the colour of the theme
QIcon MainWindow::_tabIcon(const QString &path) const
{
    QColor c = _tabNormalColor();
    return _navIcon(path, c, c);
}

void MainWindow::_adaptTabIcons()
{
    QTabBar *bar = _ui->postTabWidget->tabBar();
    for (int i = 0; i < bar->count(); ++i)
    {
        QString path = bar->tabData(i).toString();
        if (path.isEmpty())
            continue; // coloured icon of a running / finished post: left as is
        bar->setTabIcon(i, _tabIcon(path));
        if (path == ":/icons/quick.png" && i > 1)
            bar->setTabTextColor(i, _tabNormalColor());
    }
}

static QLabel *newLabel(const QString &objectName, QWidget *parent)
{
    QLabel *lbl = new QLabel(parent);
    lbl->setObjectName(objectName);
    return lbl;
}

void MainWindow::_buildShell()
{
    if (menuBar())
        menuBar()->hide();

    QWidget *central = new QWidget(this);
    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // ---------- side bar ----------
    QWidget *sideBar = new QWidget(central);
    sideBar->setObjectName("sideBar");
    sideBar->setAttribute(Qt::WA_StyledBackground, true);
    sideBar->setFixedWidth(210);
    QVBoxLayout *side = new QVBoxLayout(sideBar);
    side->setContentsMargins(10, 14, 10, 12);
    side->setSpacing(3);

    QHBoxLayout *brand = new QHBoxLayout();
    brand->setContentsMargins(6, 0, 0, 14);
    brand->setSpacing(10);
    QLabel *logo = new QLabel(sideBar);
    logo->setPixmap(QIcon(":/icons/ngPost.png").pixmap(30, 30));
    QVBoxLayout *brandText = new QVBoxLayout();
    brandText->setSpacing(0);
    QLabel *appTitle = newLabel("appTitle", sideBar);
    appTitle->setText("ngPost Deluxe");
    _appVersion = newLabel("appVersion", sideBar);
    brandText->addWidget(appTitle);
    brandText->addWidget(_appVersion);
    brand->addWidget(logo);
    brand->addLayout(brandText);
    brand->addStretch();
    side->addLayout(brand);

    _nav = new QButtonGroup(this);
    _nav->setExclusive(true);
    for (int i = 0; i < NB_NAV; ++i)
    {
        QPushButton *b = new QPushButton(sideBar);
        b->setObjectName("navButton");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        b->setIconSize({20, 20});
        b->setMinimumHeight(38);
        side->addWidget(b);
        _nav->addButton(b, i);
        _navButtons[i] = b;
    }
    _navButtons[NAV_QUICK]->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(_nav, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &MainWindow::onNavClicked);
#else
    connect(_nav, &QButtonGroup::idClicked, this, &MainWindow::onNavClicked);
#endif

    side->addStretch();

    _themeButton = new QPushButton(sideBar);
    _themeButton->setObjectName("navButton");
    _themeButton->setCursor(Qt::PointingHandCursor);
    _themeButton->setIconSize({20, 20});
    _themeButton->setMinimumHeight(38);
    connect(_themeButton, &QAbstractButton::clicked, this, &MainWindow::onThemeButtonClicked);
    side->addWidget(_themeButton);

    _aboutButton = new QPushButton(sideBar);
    _aboutButton->setObjectName("navButton");
    _aboutButton->setCursor(Qt::PointingHandCursor);
    _aboutButton->setIconSize({20, 20});
    _aboutButton->setMinimumHeight(38);
    connect(_aboutButton, &QAbstractButton::clicked, this, [this](){ if (_ngPost) _ngPost->onAboutClicked(); });
    side->addWidget(_aboutButton);

    rootLayout->addWidget(sideBar);

    // ---------- right side: pages + upload bar ----------
    QWidget *right = new QWidget(central);
    QVBoxLayout *rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    _pages = new QStackedWidget(right);
    rightLayout->addWidget(_pages, 1);

    // page 0: posts (Quick Posts tabs + Auto Posting + log)
    QWidget *postsPage = new QWidget(_pages);
    QVBoxLayout *postsLayout = new QVBoxLayout(postsPage);
    postsLayout->setContentsMargins(24, 18, 24, 12);
    postsLayout->setSpacing(10);
    _postsTitle = newLabel("pageTitle", postsPage);
    postsLayout->addWidget(_postsTitle);
    _ui->fileBox->setTitle(QString());
    _ui->fileBox->setObjectName("plainBox"); // the cards are inside the posts
    _ui->fileBox->layout()->setContentsMargins(0, 0, 0, 0);
    _ui->postSplitter->setParent(postsPage);
    _ui->postSplitter->setOrientation(Qt::Vertical); // the post takes the whole width, the log below
    _ui->postSplitter->setHandleWidth(10);
    postsLayout->addWidget(_ui->postSplitter, 1);
    _pages->addWidget(postsPage);

    // page 1: servers
    QWidget *serversPage = new QWidget(_pages);
    QVBoxLayout *serversLayout = new QVBoxLayout(serversPage);
    serversLayout->setContentsMargins(24, 18, 24, 18);
    serversLayout->setSpacing(4);
    _serversTitle    = newLabel("pageTitle", serversPage);
    _serversSubtitle = newLabel("pageSubtitle", serversPage);
    serversLayout->addWidget(_serversTitle);
    serversLayout->addWidget(_serversSubtitle);
    serversLayout->addSpacing(10);
    _ui->serverBox->setParent(serversPage);
    _ui->serverBox->setTitle(QString());
    _ui->serverBox->setObjectName("plainBox");
    serversLayout->addWidget(_ui->serverBox, 1);
    _pages->addWidget(serversPage);

    // page 2: settings (the widgets of the old "Parameters" box, grouped in cards)
    _pages->addWidget(_buildSettingsPage());

    // page 3: history of the uploads (created in init, it needs NgPost)
    _pages->addWidget(new QWidget(_pages));

    // upload bar always visible at the bottom
    _ui->uploadFrame->setParent(right);
    _ui->uploadFrame->setObjectName("uploadFrame");
    _ui->uploadFrame->setAttribute(Qt::WA_StyledBackground, true);
    _ui->uploadFrame->setFrameShape(QFrame::NoFrame);
    _ui->uploadFrame->layout()->setContentsMargins(24, 8, 24, 8);
    _ui->uploadFrame->layout()->setSpacing(12);
    _ui->jobLabel->setObjectName("jobLabel");
    _ui->jobLabel->setMinimumWidth(110);
    _ui->pauseButton->setObjectName("iconButton");
    _ui->pauseButton->setIconSize({16, 16});
    _ui->pauseButton->setFixedSize(34, 30);
    _ui->uploadLbl->setObjectName("hint");
    rightLayout->addWidget(_ui->uploadFrame);

    rootLayout->addWidget(right, 1);

    // the old containers (splitters, Parameters box) are not used anymore
    _ui->postingBox->hide();
    _ui->postingBox->setParent(_pages->widget(PAGE_SETTINGS));
    setCentralWidget(central); // deletes the old central widget (vSplitter, hSplitter...)
}

QFrame *MainWindow::_card(const QString &key, QGridLayout *&grid)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground, true);
    QVBoxLayout *v = new QVBoxLayout(card);
    v->setContentsMargins(18, 14, 18, 16);
    v->setSpacing(10);
    QLabel *title = newLabel("cardTitle", card);
    _cardTitles.insert(key, title);
    v->addWidget(title);
    grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    v->addLayout(grid);
    return card;
}

QWidget *MainWindow::_buildSettingsPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    QWidget *content = new QWidget();
    content->setObjectName("pageContent");
    scroll->setWidget(content);

    QVBoxLayout *page = new QVBoxLayout(content);
    page->setContentsMargins(24, 18, 24, 18);
    page->setSpacing(4);
    _settingsTitle    = newLabel("pageTitle", content);
    _settingsSubtitle = newLabel("pageSubtitle", content);
    page->addWidget(_settingsTitle);
    page->addWidget(_settingsSubtitle);
    page->addSpacing(10);

    QHBoxLayout *columns = new QHBoxLayout();
    columns->setSpacing(16);
    QVBoxLayout *left  = new QVBoxLayout();
    QVBoxLayout *right = new QVBoxLayout();
    left->setSpacing(16);
    right->setSpacing(16);
    columns->addLayout(left, 1);
    columns->addLayout(right, 1);
    page->addLayout(columns);
    page->addStretch();

    QGridLayout *g = nullptr;
    auto hbox = [](std::initializer_list<QWidget*> widgets, bool stretch = true) {
        QHBoxLayout *h = new QHBoxLayout();
        h->setSpacing(8);
        for (QWidget *w : widgets)
            h->addWidget(w);
        if (stretch)
            h->addStretch();
        return h;
    };

    // poster
    QFrame *poster = _card("poster", g);
    g->addWidget(_ui->fromLbl, 0, 0);
    g->addLayout(hbox({_ui->fromEdit, _ui->genPoster}, false), 0, 1);
    g->addLayout(hbox({_ui->saveFromCB, _ui->uniqueFromCB}), 1, 1);
    g->setColumnStretch(1, 1);
    left->addWidget(poster);

    // archives
    QFrame *archives = _card("archives", g);
    g->addWidget(_ui->rarPassCB, 0, 0);
    g->addLayout(hbox({_ui->rarPassEdit, _ui->rarLengthSB, _ui->genPass}, false), 0, 1);
    g->addWidget(_ui->autoCompressCB, 1, 0, 1, 2);
    g->setColumnStretch(1, 1);
    left->addWidget(archives);

    // newsgroups
    QFrame *groups = _card("groups", g);
    _ui->groupsLbl->hide();
    _ui->groupsEdit->hide(); // replaced by the chips
    _groupsEdit = new GroupsEdit(groups);
    _groupPolicyCB = new QComboBox(groups);
    _groupPolicyCB->setObjectName("groupPolicyCB");
    _groupPolicyCB->addItem(QString(), static_cast<int>(NgPost::GROUP_POLICY::ALL));
    _groupPolicyCB->addItem(QString(), static_cast<int>(NgPost::GROUP_POLICY::EACH_POST));
    _groupPolicyCB->addItem(QString(), static_cast<int>(NgPost::GROUP_POLICY::EACH_FILE));
    g->addWidget(_groupsEdit, 0, 0);
    g->addWidget(_groupPolicyCB, 1, 0);
    _retranslateGroups();
    connect(_groupsEdit,    &GroupsEdit::changed, this, &MainWindow::scheduleAutoSave);
    connect(_groupPolicyCB, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::scheduleAutoSave);
    left->addWidget(groups);
    left->addStretch();

    // upload
    QFrame *upload = _card("upload", g);
    g->addWidget(_ui->articleSizeLbl, 0, 0);
    g->addWidget(_ui->articleSizeEdit, 0, 1);
    g->addWidget(_ui->nbRetryLbl, 1, 0);
    g->addWidget(_ui->nbRetrySB, 1, 1);
    g->addWidget(_ui->threadLbl, 2, 0);
    g->addWidget(_ui->threadSB, 2, 1);
    g->addWidget(_ui->autoCloseCB, 3, 0, 1, 3);
    g->addWidget(_ui->shutdownCB, 4, 0, 1, 3);
    _ui->articleSizeEdit->setMaximumWidth(110);
    _ui->nbRetrySB->setMinimumWidth(80);
    _ui->threadSB->setMinimumWidth(80);
    g->setColumnStretch(2, 1);
    right->addWidget(upload);

    // nzb
    QFrame *nzb = _card("nzb", g);
    g->addWidget(_ui->nzbPathLbl, 0, 0);
    g->addLayout(hbox({_ui->nzbPathEdit, _ui->nzbPathButton}, false), 0, 1);
    g->setColumnStretch(1, 1);
    _ui->nzbPathEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    right->addWidget(nzb);

    // privacy
    QFrame *privacy = _card("privacy", g);
    g->addWidget(_ui->obfuscateMsgIdCB, 0, 0);
    g->addWidget(_ui->obfuscateFileNameCB, 1, 0);
    right->addWidget(privacy);

    // history of the uploads
    QFrame *history = _card("history", g);
    _historyCB = new QCheckBox(history);
    _historyCB->setObjectName("historyCB");
    _historyPathEdit = new QLineEdit(history);
    _historyPathEdit->setObjectName("historyPathEdit");
    _historyPathEdit->setReadOnly(true);
    _historyPathButton = new QPushButton(" ... ", history);
    _historyHint = newLabel("hint", history);
    _historyHint->setWordWrap(true);
    g->addWidget(_historyCB, 0, 0, 1, 2);
    g->addLayout(hbox({_historyPathEdit, _historyPathButton}, false), 1, 0, 1, 2);
    g->addWidget(_historyHint, 2, 0, 1, 2);
    _duplicatesCB = new QCheckBox(history);
    _duplicatesCB->setObjectName("duplicatesCB");
    g->addWidget(_duplicatesCB, 3, 0, 1, 2);
    g->setColumnStretch(1, 1);
    left->insertWidget(left->count() - 1, history); // before the stretch of the left column


    // notifications
    QFrame *notif = _card("notify", g);
    _notifyOkCB    = new QCheckBox(notif);
    _notifyErrCB   = new QCheckBox(notif);
    _notifyTrayCB  = new QCheckBox(notif);
    _notifyWhenLbl = new QLabel(notif);
    _notifyWhenCB  = new QComboBox(notif);
    _notifyWhenCB->addItems({QString(), QString()}); // texts set in retranslate()
    _notifyDiscordLbl = new QLabel(notif);
    _notifyTgTokenLbl = new QLabel(notif);
    _notifyTgChatLbl  = new QLabel(notif);
    _notifyWebhookLbl = new QLabel(notif);
    for (QLineEdit **e : {&_notifyDiscordEdit, &_notifyTgTokenEdit, &_notifyTgChatEdit, &_notifyWebhookEdit})
    {
        *e = new QLineEdit(notif);
        (*e)->setEchoMode(QLineEdit::Password); // addresses and tokens are secrets, like the passwords
    }
    _notifyTestButton = new QPushButton(notif);
    _notifyHint = newLabel("hint", notif);
    _notifyHint->setWordWrap(true);
    g->addWidget(_notifyOkCB, 0, 0, 1, 2);
    g->addWidget(_notifyErrCB, 1, 0, 1, 2);
    g->addWidget(_notifyTrayCB, 2, 0, 1, 2);
    g->addWidget(_notifyWhenLbl, 3, 0);
    g->addWidget(_notifyWhenCB, 3, 1);
    g->addWidget(_notifyDiscordLbl, 4, 0);
    g->addWidget(_notifyDiscordEdit, 4, 1);
    g->addWidget(_notifyTgTokenLbl, 5, 0);
    g->addWidget(_notifyTgTokenEdit, 5, 1);
    g->addWidget(_notifyTgChatLbl, 6, 0);
    g->addWidget(_notifyTgChatEdit, 6, 1);
    g->addWidget(_notifyWebhookLbl, 7, 0);
    g->addWidget(_notifyWebhookEdit, 7, 1);
    g->addWidget(_notifyTestButton, 8, 0, 1, 2);
    g->addWidget(_notifyHint, 9, 0, 1, 2);
    g->setColumnStretch(1, 1);
    left->insertWidget(left->count() - 1, notif);

    // queue of the posts
    QFrame *queue = _card("queue", g);
    _queueCB = new QCheckBox(queue);
    _queueCB->setObjectName("queueCB");
    _queueHint = newLabel("hint", queue);
    _queueHint->setWordWrap(true);
    g->addWidget(_queueCB, 0, 0);
    g->addWidget(_queueHint, 1, 0);
    g->setColumnStretch(0, 1);
    left->insertWidget(left->count() - 1, queue);

    // interface
    QFrame *gui = _card("interface", g);
    _themeLbl = new QLabel(gui);
    _themeCB  = new QComboBox(gui);
    _themeCB->setObjectName("themeCB");
    _themeCB->addItems({QString(), QString(), QString()});
    _themeCB->setMinimumWidth(140);
    _ui->langCB->setMinimumWidth(140);
    _autoSaveHint = newLabel("hint", gui);
    _autoSaveHint->setWordWrap(true);
    g->addWidget(_ui->langLbl, 0, 0);
    g->addWidget(_ui->langCB, 0, 1);
    g->addWidget(_themeLbl, 1, 0);
    g->addWidget(_themeCB, 1, 1);
    g->addWidget(_autoSaveHint, 2, 0, 1, 3);
    g->addLayout(hbox({_ui->saveButton}), 3, 0, 1, 3);
    g->setColumnStretch(2, 1);
    right->addWidget(gui);

    // support of the project (Ko-fi page opened in the browser)
    QFrame *support = _card("support", g);
    _supportHint = newLabel("hint", support);
    _supportHint->setWordWrap(true);
    _supportButton = new QPushButton(support);
    _supportButton->setObjectName("supportButton");
    _supportButton->setCursor(Qt::PointingHandCursor);
    connect(_supportButton, &QAbstractButton::clicked, this, [](){ QDesktopServices::openUrl(QUrl(NgPost::supportUrl())); });
    g->addWidget(_supportHint, 0, 0, 1, 2);
    g->addLayout(hbox({_supportButton}), 1, 0, 1, 2);
    left->insertWidget(left->count() - 1, support); // before the stretch of the left column
    right->addStretch();

    return scroll;
}

void MainWindow::_retranslateShell()
{
    if (!_pages)
        return;
    // same names as the tabs
    _navButtons[NAV_QUICK]->setText(_ngPost ? _ngPost->quickJobName() : tr("Quick Post"));
    _navButtons[NAV_AUTO]->setText(_ngPost ? _ngPost->folderMonitoringName() : tr("Auto Posting"));
    _navButtons[NAV_SERVERS]->setText(tr("Servers"));
    _navButtons[NAV_SETTINGS]->setText(tr("Settings"));
    _aboutButton->setText(tr("About"));
    _updateThemeButton();

    _postsTitle->setText(_navButtons[NAV_AUTO]->isChecked() ? _navButtons[NAV_AUTO]->text() : _navButtons[NAV_QUICK]->text());
    _serversTitle->setText(tr("Servers"));
    _serversSubtitle->setText(tr("The posts are uploaded on all the enabled servers. Test a server before using it."));
    _settingsTitle->setText(tr("Settings"));
    _settingsSubtitle->setText(tr("Default parameters of the posts and of the application."));

    _cardTitles["poster"]->setText(tr("Poster"));
    _cardTitles["archives"]->setText(tr("Archives and password"));
    _cardTitles["groups"]->setText(tr("Newsgroups"));
    _retranslateGroups();
    _cardTitles["upload"]->setText(tr("Upload"));
    _cardTitles["nzb"]->setText(tr("NZB files"));
    _cardTitles["privacy"]->setText(tr("Privacy"));
    _cardTitles["interface"]->setText(tr("Interface"));
    _cardTitles["history"]->setText(tr("History of the uploads"));
    _cardTitles["notify"]->setText(tr("Notifications"));
    _notifyOkCB->setText(tr("Notify when a post is finished"));
    _notifyErrCB->setText(tr("Notify when a post has errors or fails"));
    _notifyTrayCB->setText(tr("System notification (icon in the notification area)"));
    _notifyWhenLbl->setText(tr("Send:"));
    _notifyWhenCB->setItemText(0, tr("one message when all the posts are done"));
    _notifyWhenCB->setItemText(1, tr("a message after each post"));
    _notifyDiscordLbl->setText(tr("Discord webhook:"));
    _notifyTgTokenLbl->setText(tr("Telegram bot token:"));
    _notifyTgChatLbl->setText(tr("Telegram chat id:"));
    _notifyWebhookLbl->setText(tr("Other webhook:"));
    _notifyTestButton->setText(tr("Send a test message"));
    _notifyHint->setText(tr("The messages never contain the password or the names of the files. The addresses and the token are saved "
                            "in clear text in the configuration file (like the other passwords): keep it private."));
    _cardTitles["queue"]->setText(tr("Queue of the posts"));
    _queueCB->setText(tr("Save the posts that are not finished, to post them again after a restart"));
    _queueHint->setText(tr("The queue also records the password of each archive in clear text (like the history). "
                           "Turning it off deletes the saved queue."));
    _cardTitles["support"]->setText(tr("Support the project"));
    _supportHint->setText(tr("%1 is free. If it is useful to you, you can support its development with a donation on Ko-fi.").arg(_ngPost ? _ngPost->appName() : "ngPost Deluxe"));
    _supportButton->setText(tr("Support on Ko-fi"));
    _supportButton->setToolTip(NgPost::supportUrl());
    _navButtons[NAV_HISTORY]->setText(tr("History"));
    _navButtons[NAV_HELP]->setText(tr("Help"));
    if (_help && _ngPost)
        _help->setLanguage(_ngPost->_lang);
    _historyCB->setText(tr("Save the history of the uploads"));
    _duplicatesCB->setText(tr("Warn when a post is already in the history (same name and size)"));
    _duplicatesCB->setToolTip(tr("Quick Post asks before posting it again; the auto posting skips it."));
    _historyPathButton->setToolTip(tr("Choose the history file"));
    _historyHint->setText(tr("The history also records the password of each archive in clear text: "
                             "it stays on this computer, but anyone using it can read the file."));
    if (_history)
        _history->retranslate();

    _ui->langLbl->setText(tr("Language"));
    _ui->nzbPathLbl->setText(tr("Folder"));
    _themeLbl->setText(tr("Theme"));
    _themeCB->setItemText(0, tr("Automatic (like Windows)"));
    _themeCB->setItemText(1, tr("Light"));
    _themeCB->setItemText(2, tr("Dark"));
    _autoSaveHint->setText(tr("The settings are saved automatically after each change."));
    _ui->saveButton->setText(tr("Save now"));
    _ui->logBox->setTitle(tr("Log"));
    setJobLabel(_jobNumber);
}

void MainWindow::_retranslateGroups()
{
    if (!_groupsEdit || !_groupPolicyCB)
        return;
    _groupsEdit->retranslate();
    _groupPolicyCB->setItemText(0, tr("All the groups in every post"));
    _groupPolicyCB->setItemText(1, tr("One random group for each post"));
    _groupPolicyCB->setItemText(2, tr("One random group for each file (with Obfuscate Articles)"));
    _groupPolicyCB->setToolTip(tr("When there are several groups: where the posts go. \"For each file\" needs the obfuscation of the Articles."));
}

QStringList MainWindow::invalidGroups() const { return _groupsEdit->invalidGroups(); }
bool MainWindow::hasGroups() const { return !_groupsEdit->groups().isEmpty(); }
void MainWindow::showServersPage()  { _navButtons[NAV_SERVERS]->click(); }
void MainWindow::showSettingsPage() { _navButtons[NAV_SETTINGS]->click(); }

bool MainWindow::checkGroups()
{
    QStringList bad = _groupsEdit->invalidGroups();
    if (_groupsEdit->groups().isEmpty())
    {
        logError(tr("There is no newsgroup: add at least one in the Settings (Newsgroups)."));
        return false;
    }
    if (!bad.isEmpty())
    {
        logError(tr("These newsgroups are not valid: %1. Fix them in the Settings (Newsgroups).").arg(bad.join(", ")));
        return false;
    }
    return true;
}

void MainWindow::_updateThemeButton()
{
    if (!_themeButton || !_ngPost)
        return;
    QString mode = _ngPost->theme() == "light" ? tr("Light") : _ngPost->theme() == "dark" ? tr("Dark") : tr("Auto");
    _themeButton->setText(tr("Theme: %1").arg(mode));
}

void MainWindow::onThemeApplied()
{
    if (!_pages || !_ngPost)
        return;
    bool dark = _ngPost->isDarkTheme();
    QColor normal  = dark ? QColor("#d6d6d6") : QColor("#3b3b3b");
    QColor checked = dark ? QColor("#4cc2ff") : QColor("#0067c0");
    static const char *icons[NB_NAV] = {":/icons/nav_quick.png", ":/icons/nav_auto.png",
                                        ":/icons/nav_servers.png", ":/icons/nav_settings.png",
                                        ":/icons/nav_history.png", ":/icons/nav_help.png"};
    for (int i = 0; i < NB_NAV; ++i)
        _navButtons[i]->setIcon(_navIcon(icons[i], normal, checked));
    _themeButton->setIcon(_navIcon(":/icons/nav_theme.png", normal, checked));
    _adaptIcons();
    _adaptTabIcons();
    _aboutButton->setIcon(_navIcon(":/icons/nav_about.png", normal, checked));
    setPauseIcon(_pauseIcon);
    _updateThemeButton();
}

void MainWindow::onNavClicked(int id)
{
    switch (id)
    {
    case NAV_QUICK:
        _pages->setCurrentIndex(PAGE_POSTS);
        if (_ui->postTabWidget->currentIndex() == 1)
            _ui->postTabWidget->setCurrentIndex(0);
        break;
    case NAV_AUTO:
        _pages->setCurrentIndex(PAGE_POSTS);
        _ui->postTabWidget->setCurrentIndex(1);
        break;
    case NAV_SERVERS:
        _pages->setCurrentIndex(PAGE_SERVERS);
        break;
    case NAV_SETTINGS:
        _pages->setCurrentIndex(PAGE_SETTINGS);
        break;
    case NAV_HISTORY:
        _pages->setCurrentIndex(PAGE_HISTORY);
        break;
    case NAV_HELP:
        _pages->setCurrentIndex(PAGE_HELP);
        break;
    }
    _postsTitle->setText(_navButtons[id == NAV_AUTO ? NAV_AUTO : NAV_QUICK]->text());
}

void MainWindow::onPostTabChanged(int index)
{
    if (_pages->currentIndex() != PAGE_POSTS)
        return;
    int nav = index == 1 ? NAV_AUTO : NAV_QUICK;
    _navButtons[nav]->setChecked(true);
    _postsTitle->setText(_navButtons[nav]->text());
}

void MainWindow::onThemeSelected(int index)
{
    static const char *themes[3] = {"auto", "light", "dark"};
    if (index < 0 || index > 2 || !_ngPost)
        return;
    _ngPost->setTheme(themes[index]);
    // the status colours of the existing tabs keep their old colour until their next update
}

void MainWindow::onThemeButtonClicked()
{
    // Auto -> Light -> Dark -> Auto
    _themeCB->setCurrentIndex((_themeCB->currentIndex() + 1) % 3);
}

// the black icons of the original ngPost (trash, pause, file...) are lightened on the dark theme
void MainWindow::_adaptIcons()
{
    if (!_ngPost)
        return;
    bool dark = _ngPost->isDarkTheme();
    for (QAbstractButton *b : findChildren<QAbstractButton*>())
    {
        if (b->objectName() == "navButton")
            continue;
        QVariant orig = b->property("ngOrigIcon");
        if (!orig.isValid())
        {
            if (b->icon().isNull())
                continue;
            // keep only the icons that are black/grey (no colour)
            QImage img = b->icon().pixmap(32, 32).toImage().convertToFormat(QImage::Format_ARGB32);
            bool mono = true, hasPixel = false;
            for (int y = 0; y < img.height() && mono; ++y)
                for (int x = 0; x < img.width(); ++x)
                {
                    QColor c = QColor::fromRgba(img.pixel(x, y));
                    if (c.alpha() < 40)
                        continue;
                    hasPixel = true;
                    if (c.hsvSaturation() > 40 || c.lightness() > 110)
                    {
                        mono = false;
                        break;
                    }
                }
            if (!mono || !hasPixel)
            {
                b->setProperty("ngOrigIcon", QIcon());
                continue;
            }
            b->setProperty("ngOrigIcon", b->icon());
            orig = b->property("ngOrigIcon");
        }
        QIcon icon = orig.value<QIcon>();
        if (icon.isNull())
            continue; // not a black icon
        if (!dark)
            b->setIcon(icon);
        else
        {
            QPixmap pix = icon.pixmap(64, 64);
            QPainter p(&pix);
            p.setCompositionMode(QPainter::CompositionMode_SourceIn);
            p.fillRect(pix.rect(), QColor("#d6d6d6"));
            p.end();
            b->setIcon(QIcon(pix));
        }
    }
}

void MainWindow::_fixIconButtons()
{
    // the small "..." buttons (choose a file or a folder) become folder icons
    for (QPushButton *b : findChildren<QPushButton*>())
    {
        if (b->text().trimmed() == "...")
        {
            b->setText(QString());
            b->setIcon(QIcon(":/icons/folder.png"));
            if (b->objectName() != "iconButton")
            {
                b->setObjectName("iconButton");
                b->style()->unpolish(b);
                b->style()->polish(b);
            }
        }
    }
    _adaptIcons();
}

void MainWindow::_updateHistorySettings()
{
    QString path = _ngPost->historyFile();
    if (path.isEmpty() && !_ngPost->_postHistoryFile.isEmpty())
        path = _ngPost->_postHistoryFile;
    _historyPathEdit->setText(QDir::toNativeSeparators(path));
    _historyPathEdit->setEnabled(_ngPost->_historyEnabled);
    _historyPathButton->setEnabled(_ngPost->_historyEnabled);
}

void MainWindow::onQueueToggled(bool checked)
{
    _ngPost->queueSetEnabled(checked);
    scheduleAutoSave();
}

void MainWindow::_offerQueue()
{
    PostQueue *queue = _ngPost->postQueue();
    if (!queue || queue->size() == 0)
        return;

    // what is waiting
    QStringList lines;
    int nbWaiting = 0, nbMissing = 0;
    for (const QJsonObject &entry : queue->entries())
    {
        if (entry.value("state").toString() == PostQueue::sStateMissing)
        {
            RepostInfo info = PostQueue::repostFromJson(entry.value("missing").toObject());
            lines << tr("%1: %2 Article(s) to post again").arg(QFileInfo(info.nzbPath).fileName()).arg(info.nbArticles());
            ++nbMissing;
        }
        else
        {
            lines << tr("%1: to post again from the beginning").arg(QFileInfo(entry.value("nzb").toString()).fileName());
            ++nbWaiting;
        }
    }

    QMessageBox box(QMessageBox::Question, tr("Posts that were not finished"),
                    tr("%1 post(s) were not finished the last time ngPost was closed.\n\n"
                       "Resume: they are started again (a post that was being sent starts from the beginning).\n"
                       "Discard: they are removed from the queue.\n"
                       "Decide later: they stay in the queue and will be offered again at the next start.").arg(queue->size()),
                    QMessageBox::NoButton, this);
    box.setDetailedText(lines.join("\n"));
    QPushButton *resume  = box.addButton(tr("Resume"), QMessageBox::AcceptRole);
    QPushButton *discard = box.addButton(tr("Discard"), QMessageBox::DestructiveRole);
    box.addButton(tr("Decide later"), QMessageBox::RejectRole);
    box.setDefaultButton(resume);
    box.exec();

    if (box.clickedButton() == discard)
    {
        queue->clear();
        return;
    }
    if (box.clickedButton() != resume)
        return;

    const QList<QJsonObject> entries = queue->entries(); // copy: the entries change while we go
    for (const QJsonObject &entry : entries)
    {
        const QString id = entry.value("id").toString();
        PostingWidget *tab = addNewQuickTab(0);
        QString error;
        if (entry.value("state").toString() == PostQueue::sStateMissing)
        {
            RepostInfo info = PostQueue::repostFromJson(entry.value("missing").toObject());
            info.queueId = id;
            QString problem = NgPost::queueProblemMissing(info);
            if (!problem.isEmpty())
            {
                logError(tr("Queue: can't post again the missing Articles of %1: %2").arg(
                             QFileInfo(info.nzbPath).fileName(), problem));
                queue->remove(id);
                closeTab(tab);
                continue;
            }
            tab->restoreMissing(info); // the "Repost missing Articles" button is there, the user starts it
            _ngPost->_log(tr("Queue: posting again the %1 missing Articles of %2").arg(info.nbArticles()).arg(
                              QFileInfo(info.nzbPath).fileName()));
            tab->onRepostMissing();
        }
        else if (!tab->restoreFromQueue(entry, error))
        {
            logError(tr("Queue: can't post again %1: %2").arg(
                         QFileInfo(entry.value("nzb").toString()).fileName(), error));
            queue->remove(id);
            closeTab(tab);
        }
    }
}

void MainWindow::_showTrayMessage(const QString &title, const QString &text)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;
    if (!_tray)
    {
        _tray = new QSystemTrayIcon(QIcon(":/icons/ngPost.png"), this);
        connect(_tray, &QSystemTrayIcon::messageClicked, this, [this](){ showNormal(); raise(); activateWindow(); });
        _tray->show();
    }
    _tray->showMessage(title, text, QSystemTrayIcon::Information, 10000);
}

void MainWindow::onHistoryToggled(bool checked)
{
    _ngPost->_historyEnabled = checked;
    _updateHistorySettings();
    if (_history)
        _history->reload();
}

void MainWindow::onHistoryPathClicked()
{
    QString current = _ngPost->historyFile();
    QString path = QFileDialog::getSaveFileName(this, tr("History file"), current,
                                                tr("CSV files (*.csv)"), nullptr,
                                                QFileDialog::DontConfirmOverwrite); // an existing history is kept
    if (path.isEmpty())
        return;
    _ngPost->_postHistoryFile = path;
    _updateHistorySettings();
    if (_history)
        _history->reload();
    scheduleAutoSave();
}

void MainWindow::onHistoryUpdated()
{
    if (_history)
        _history->reload();
}
