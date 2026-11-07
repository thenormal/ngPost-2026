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
#include "NgPost.h"
#include "nntp/NntpServerParams.h"
#include "nntp/NntpArticle.h"

#include <QDebug>
#include <QProgressBar>
#include <QLabel>
#include <QScreen>
#include <QMessageBox>
#include <QTimer>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QFileDialog>
#include <QStorageInfo>
#include <QDateTime>
#include <QSysInfo>
#include <QThread>
#include <QSaveFile>
#include "utils/Diagnostics.h"
#include "PostingJob.h"


const QColor  MainWindow::sPostingColor = QColor(255,162, 0); // gold (#FFA200)
const QString MainWindow::sPostingIcon  = ":/icons/uploading.png";
const QColor  MainWindow::sPendingColor = Qt::darkBlue;
const QString MainWindow::sPendingIcon  = ":/icons/pending.png";
const QColor  MainWindow::sDoneOKColor  = Qt::darkGreen;
const QString MainWindow::sDoneOKIcon   = ":/icons/ok.png";
const QColor  MainWindow::sDoneKOColor  = Qt::darkRed;
const QString MainWindow::sDoneKOIcon   = ":/icons/ko.png";
const QColor  MainWindow::sArticlesFailedColor  = Qt::darkYellow;


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
    _autoSaveAutoTab(false)
{
    _autoSaveTimer->setSingleShot(true);
    _autoSaveTimer->setInterval(sAutoSaveDelayMs);
    connect(_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSave);
    setAcceptDrops(true);

    _ui->setupUi(this);


    _ui->hSplitter->setStretchFactor(0, 1);
    _ui->hSplitter->setStretchFactor(1, 1);

    _ui->vSplitter->setStretchFactor(0, 1);
    _ui->vSplitter->setStretchFactor(1, 3);
    _ui->vSplitter->setCollapsible(1, false);

    _ui->postSplitter->setStretchFactor(0, 3);
    _ui->postSplitter->setStretchFactor(1, 1);
    _ui->postSplitter->setCollapsible(0, false);


    _ui->progressBar->setRange(0, 100);
    updateProgressBar(0, 0, "");

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
    _ui->postTabWidget->addTab(_quickJobTab, QIcon(":/icons/quick.png"), _ngPost->quickJobName());
    tabBar->setTabToolTip(0, tr("Default %1").arg(_ngPost->quickJobName()));
    _ui->postTabWidget->addTab(_autoPostTab, QIcon(":/icons/auto.png"), _ngPost->folderMonitoringName());
    tabBar->setTabToolTip(1, _ngPost->folderMonitoringName());
    _ui->postTabWidget->addTab(new QWidget(_ui->postTabWidget), QIcon(":/icons/plus.png"), tr("New"));
    tabBar->setTabToolTip(2, QString("Create a new %1").arg(_ngPost->quickJobName()));

//    connect(_ui->postTabWidget,           &QTabWidget::currentChanged, this, &MainWindow::onJobTabClicked);
    connect(tabBar, &QTabBar::tabBarClicked,              this, &MainWindow::onJobTabClicked);
    connect(tabBar, &QWidget::customContextMenuRequested, this, &MainWindow::onTabContextMenu);
    connect(tabBar, &QTabBar::tabCloseRequested,          this, &MainWindow::onCloseJob);
    _ui->postTabWidget->setTabsClosable(true);
    _ui->postTabWidget->installEventFilter(this);
//    _ui->postTabWidget->setCurrentIndex(1);

    setJobLabel(1);


    for (const QString &lang : _ngPost->languages())
        _ui->langCB->addItem(QIcon(QString(":/icons/flag_%1.png").arg(lang.toUpper())), lang.toUpper(), lang);
    _ui->langCB->setCurrentText(_ngPost->_lang.toUpper());
    connect(_ui->langCB, &QComboBox::currentTextChanged, this, &MainWindow::onLangChanged);

    _initServerBox();
    _initPostingBox();
    _quickJobTab->init();
    _autoPostTab->init();

    // autosave: watch the global settings, the servers, the default Quick Post and the Auto Posting tabs
    _watchForAutoSave(_ui->postingBox, -1);
    _watchForAutoSave(_quickJobTab, 0);
    _watchForAutoSave(_autoPostTab, 1);
    QTimer::singleShot(0, this, [this](){ _autoSaveReady = true; }); // after the initialisation of the widgets

    _ui->goCmdButton->hide();
//    connect(_ui->goCmdButton, &QAbstractButton::clicked, _ngPost, &NgPost::onGoCMD, Qt::QueuedConnection);

    updateProgressBar(0, 0);
}


void MainWindow::updateProgressBar(uint nbArticlesTotal, uint nbArticlesUploaded, const QString &avgSpeed
                                   #ifdef __COMPUTE_IMMEDIATE_SPEED__
                                       , const QString &immediateSpeed
                                   #endif
                                   )
{
//    qDebug() << "[MainWindow::updateProgressBar] _nbArticlesUploaded: " << nbArticlesUploaded;
    _ui->progressBar->setValue(static_cast<int>(nbArticlesUploaded));

#ifdef __COMPUTE_IMMEDIATE_SPEED__
    _ui->uploadLbl->setText(QString("%5 (%1 / %2) %3: %4").arg(
                                nbArticlesUploaded).arg(
                                nbArticlesTotal).arg(
                                tr("avg speed")).arg(
                                avgSpeed).arg(
                                immediateSpeed));
#else
    _ui->uploadLbl->setText(QString("(%1 / %2) %3: %4").arg(
                                nbArticlesUploaded).arg(
                                nbArticlesTotal).arg(
                                tr("avg speed")).arg(
                                avgSpeed));
#endif
}


void MainWindow::log(const QString &aMsg, bool newline) const
{
    if (newline)
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
#ifdef __COMPUTE_IMMEDIATE_SPEED__
            _ui->uploadLbl->setToolTip(tr("Immediate speed (avg on %1 sec) - (nb Articles uploaded / total number of Articles) - avg speed").arg(NgPost::immediateSpeedDuration()));
#endif
            _ui->shutdownCB->setToolTip(tr("Shutdown computer when all the current Posts are done (with command: %1)").arg(
                                            _ngPost->_shutdownCmd));

            _ui->serverBox->setTitle(tr("Servers"));
            _ui->fileBox->setTitle(tr("Files"));
            _ui->postingBox->setTitle(tr("Parameters"));
            _ui->logBox->setTitle(tr("Posting Log"));

            tabBar->setTabText(0, _ngPost->quickJobName());
            tabBar->setTabToolTip(0, tr("Default %1").arg(_ngPost->quickJobName()));
            tabBar->setTabText(1, _ngPost->folderMonitoringName());
            tabBar->setTabToolTip(1, _ngPost->folderMonitoringName());
            for (int i = 2 ; i < lastTabIdx; ++i)
                tabBar->setTabText(i, _ngPost->quickJobName());
            tabBar->setTabText(lastTabIdx, tr("New"));
            tabBar->setTabToolTip(2, QString("Create a new %1").arg(_ngPost->quickJobName()));

            setJobLabel(_ui->postTabWidget->currentIndex());

            _servers->retranslate();


            _quickJobTab->retranslate();
            _autoPostTab->retranslate();
            for (int i = 2 ; i < _ui->postTabWidget->count() - 1; ++i)
                _getPostWidget(i)->retranslate();
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
    _ui->groupsEdit->setText(_ngPost->groups());
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

    _ngPost->updateGroups(_ui->groupsEdit->toPlainText());

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
                                  QIcon(":/icons/quick.png"),
                                  tabName);
    _ui->postTabWidget->setTabToolTip(lastTabIdx, tabName);

    for (const QFileInfo &file : files)
        newPostingWidget->addPath(file.absoluteFilePath(), 0, file.isDir());

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
            bar->setTabTextColor(i, Qt::black);
            bar->setTabIcon(i, QIcon(":/icons/quick.png"));
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
            break;
        }
    }
}

void MainWindow::setJobLabel(int jobNumber)
{
    _ui->jobLabel->setText(QString("<b><u>Post #%1</u></b>").arg(jobNumber != 1 ? QString::number(jobNumber) : "Auto"));
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
        else
        {
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
    if (pause)
        _ui->pauseButton->setIcon(QIcon(":/icons/pause.png"));
    else
        _ui->pauseButton->setIcon(QIcon(":/icons/play.png"));
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
