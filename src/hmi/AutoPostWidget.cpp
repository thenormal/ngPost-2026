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

#include "AutoPostWidget.h"
#include "ProfileBar.h"
#include "NfoCard.h"
#include "utils/PostProfile.h"
#include "ui_AutoPostWidget.h"
#include "PostingWidget.h"
#include "MainWindow.h"
#include "NgPost.h"
#include "FoldersMonitorForNewFiles.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMimeData>
#include <QTimer>
#include <QComboBox>
#include <QLineEdit>
#include <QStorageInfo>
#include <QDirIterator>
#include <QButtonGroup>
#include <QStyledItemDelegate>
#include <QPainter>
#include "PostCheckDialog.h"
#include "DuplicatesDialog.h"
#include "utils/NfoWriter.h"
#include "utils/MediaInfoReader.h"
#include "nntp/NntpServerParams.h"

AutoPostWidget::AutoPostWidget(NgPost *ngPost, MainWindow *hmi) :
    QWidget(hmi),
    _ui(new Ui::AutoPostWidget),
    _hmi(hmi),
    _ngPost(ngPost),
    _isMonitoring(false),
    _currentPostIdx(1),
    _filesTitle(nullptr), _sourceTitle(nullptr), _archiveTitle(nullptr), _advancedTitle(nullptr),
    _monitorTitle(nullptr), _advancedCard(nullptr), _profileBar(nullptr), _nfoCard(nullptr), _advancedToggle(nullptr), _showAdvanced(false),
    _mode(Mode::Choose), _sourceCard(nullptr), _monitorCard(nullptr), _summary(nullptr), _afterTitle(nullptr), _afterCombo(nullptr), _postedEdit(nullptr), _postedButton(nullptr), _afterCard(nullptr), _sizeTimer(nullptr)
{
    _ui->setupUi(this);
    _buildLayout();
    connect(_ui->filesList, &SignedListWidget::emptyAction1, this, &AutoPostWidget::onSelectFilesClicked);
    connect(_ui->filesList, &SignedListWidget::emptyAction2, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select a Folder"), _ngPost->_inputDir, QFileDialog::ShowDirsOnly);
        if (!dir.isEmpty())
            _ui->filesList->addPathIfNotInList(dir, _ui->filesList->count(), true);
    });
    connect(_ui->filesList, &SignedListWidget::rightClick, this, &AutoPostWidget::onSelectFilesClicked);
}

AutoPostWidget::~AutoPostWidget()
{
    delete _ui;
}

void AutoPostWidget::init()
{
    _ui->rarMaxCB->setChecked(_ngPost->_useRarMax);

    _ui->keepRarCB->setChecked(_ngPost->_keepRar);

    _ui->compressPathEdit->setText(_ngPost->_tmpPath);
    _ui->rarEdit->setText(_ngPost->_rarPath);

    _ui->rarSizeEdit->setText(QString::number(_ngPost->_rarSize));
    _ui->rarSizeEdit->setValidator(new QIntValidator(1, 1000000, _ui->rarSizeEdit));

    _ui->redundancySB->setRange(0, 100);
    _ui->redundancySB->setValue(static_cast<int>(_ngPost->_par2Pct));
    _ui->redundancySB->setEnabled(_ngPost->_par2Args.isEmpty());

    _ui->autoDirEdit->setText(_ngPost->_inputDir);
    _ui->nameLengthSB->setRange(5, 50);
    _ui->nameLengthSB->setValue(static_cast<int>(_ngPost->_lengthName));
    _ui->passLengthSB->setRange(5, 50);
    _ui->passLengthSB->setValue(static_cast<int>(_ngPost->_lengthPass));

    _ui->filesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(_ui->compressPathButton,&QAbstractButton::clicked, this, &AutoPostWidget::onCompressPathClicked);
    connect(_ui->rarPathButton,     &QAbstractButton::clicked, this, &AutoPostWidget::onRarPathClicked);
    connect(_ui->autoDirButton,     &QAbstractButton::clicked, this, &AutoPostWidget::onSelectAutoDirClicked);
    connect(_ui->scanAutoDirButton, &QAbstractButton::clicked, this, &AutoPostWidget::onScanAutoDirClicked);
    connect(_ui->compressCB,        &QAbstractButton::toggled, this, &AutoPostWidget::onCompressToggled);
    connect(_ui->par2CB,            &QAbstractButton::toggled, this, &AutoPostWidget::onGenPar2Toggled);

    setPackingAuto(_ngPost->_packAuto, _ngPost->_packAutoKeywords);
    if (_ngPost->_keepRar)
        _ui->keepRarCB->setChecked(true);
    _ui->startJobsCB->setChecked(true);

    _ui->latestFilesFirstCB->setChecked(true);

    connect(_ui->postButton,   &QAbstractButton::clicked, this,    &AutoPostWidget::onGenQuickPosts);
    connect(_ui->aboutButton,  &QAbstractButton::clicked, _ngPost, &NgPost::onAboutClicked);
    _ui->aboutButton->hide(); // About is in the side bar
    connect(_ui->removeCompletedButton, &QAbstractButton::clicked, this, &AutoPostWidget::onRemoveCompleted);
    connect(_ui->removeAllButton,       &QAbstractButton::clicked, this, &AutoPostWidget::onRemoveAll);
    connect(_ui->closeTabsButton,       &QAbstractButton::clicked, _hmi, &MainWindow::onCloseAllFinishedQuickTabs);

    connect(_ui->monitorButton, &QAbstractButton::clicked, this, &AutoPostWidget::onMonitoringClicked);



    _ui->addMonitoringFolderButton->setEnabled(false);
    connect(_ui->addMonitoringFolderButton, &QAbstractButton::clicked, this, &AutoPostWidget::onAddMonitoringFolder);


    _ui->extensionFilterEdit->setText(_ngPost->_monitorExtensions.isEmpty()? "" : _ngPost->_monitorExtensions.join(","));
    _ui->dirAllowedCB->setChecked(!_ngPost->_monitorIgnoreDir);

    if (!_ngPost->_movePosted.isEmpty())
    {
        _postedEdit->setText(_ngPost->_movePosted);
        _afterCombo->setCurrentIndex(1);
        _postedEdit->setVisible(true);
        _postedButton->setVisible(true);
    }
    _sizeTimer = new QTimer(this);
    _sizeTimer->setSingleShot(true);
    connect(_sizeTimer, &QTimer::timeout, this, &AutoPostWidget::_computeNextSize);
    QAbstractItemModel *model = _ui->filesList->model();
    connect(model, &QAbstractItemModel::rowsInserted, this, [this](){ QTimer::singleShot(0, this, [this](){ _decorateItems(); }); });
    connect(model, &QAbstractItemModel::rowsRemoved,  this, [this](){ _updateSummary(); });
    connect(model, &QAbstractItemModel::modelReset,   this, [this](){ _updateSummary(); });
    connect(_ui->filesList, &QListWidget::itemChanged, this, [this](){ _updateSummary(); });
    for (QAbstractButton *b : {static_cast<QAbstractButton*>(_ui->compressCB), static_cast<QAbstractButton*>(_ui->par2CB)})
        connect(b, &QAbstractButton::toggled, this, [this](){ _updateSummary(); });
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ _updateSummary(); });

    _profileBar->applyRecommendedIfNew(); // first start of a new installation

    Mode mode = _ngPost->_autoMode == "scan" ? Mode::Scan : _ngPost->_autoMode == "watch" ? Mode::Watch : Mode::Choose;
    _setMode(mode);
}


void AutoPostWidget::onGenQuickPosts()
{
    bool hasFolder = false;
    QFileInfoList files = _checkedFiles(&hasFolder);
    if (files.isEmpty())
    {
        QMessageBox::warning(nullptr,
                             tr("Nothing to post..."),
                             tr("There is nothing to post!\nAdd files or folders, or scan a folder, and keep checked what you want to post."));
        return;
    }

    if (!_hmi->checkGroups())
        return;
    if (!_checkBeforeStart(files, hasFolder)) // the same checks as Quick Post, once for the whole group
        return;
    _hmi->updateServers();
    _hmi->updateParams();
    udatePostingParams();

    // items already in the history: one window for all of them (nothing is posted again unless it is checked)
    if (_ngPost->_checkDuplicates)
    {
        QList<DuplicatesDialog::Item> dups;
        for (const QFileInfo &file : files)
        {
            _ngPost->setNzbName(file);
            QString posted = _ngPost->duplicateInPost(QString("%1.nzb").arg(_ngPost->_nzbName), {file});
            if (!posted.isEmpty())
                dups.append({file.filePath(), file.fileName(), NgPost::recursiveSize(file), posted});
        }
        if (!dups.isEmpty())
        {
            QSet<QString> again;
            if (!DuplicatesDialog::run(this, dups, again))
                return;
            QFileInfoList kept;
            for (const QFileInfo &file : files)
                kept << file;
            files.clear();
            for (const QFileInfo &file : kept)
            {
                const DuplicatesDialog::Item *dup = nullptr;
                for (const DuplicatesDialog::Item &d : dups)
                    if (d.path == file.filePath())
                        dup = &d;
                if (!dup || again.contains(file.filePath()))
                {
                    files << file;
                    continue;
                }
                _ngPost->_log(tr("%1 is already in the history (posted on %2): not posted again.").arg(file.fileName(), dup->postedOn));
                for (int i = 0; i < _ui->filesList->count(); ++i)
                {
                    QListWidgetItem *item = _ui->filesList->item(i);
                    if (item->text() == file.filePath())
                    {
                        item->setCheckState(Qt::Unchecked);
                        item->setForeground(QColor(Qt::gray));
                        item->setToolTip(tr("Already posted on %1").arg(dup->postedOn));
                        break;
                    }
                }
            }
            if (files.isEmpty())
                return;
        }
    }

    const QString afterMoveTo = movePostedTo();
    const bool afterAct = deleteFilesOncePosted() || !afterMoveTo.isEmpty();
    bool startPost = _ui->startJobsCB->isChecked(),
         useRarMax = _ui->rarMaxCB->isChecked();
    for (const QFileInfo &file : files)
    {
        PostingWidget *quickPostWidget = _hmi->addNewQuickTab(0, {file});
        quickPostWidget->init();
        quickPostWidget->setFromAutoPosting();
        quickPostWidget->setAfterPost(afterAct, afterMoveTo);
        quickPostWidget->genNameAndPassword(_ngPost->_genName, _ngPost->_genPass, _ngPost->_doPar2, useRarMax);

        // keep track of the result to be able to "Remove completed"
        const QString srcPath = file.filePath();
        connect(quickPostWidget, &PostingWidget::postingDone, this, [this, srcPath](bool uploadedOk) {
            for (int i = 0; i < _ui->filesList->count(); ++i)
            {
                QListWidgetItem *item = _ui->filesList->item(i);
                if (item->text() == srcPath)
                {
                    item->setForeground(uploadedOk ? _hmi->sDoneOKColor : _hmi->sDoneKOColor);
                    item->setData(sCompletedRole, uploadedOk);
                    break;
                }
            }
        });

        if (startPost)
            quickPostWidget->postFiles(false);
    }
}


void AutoPostWidget::onCompressPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->compressPathEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
        _ui->compressPathEdit->setText(path);
}

void AutoPostWidget::onRarPathClicked()
{
    QString path = QFileDialog::getOpenFileName(
                this,
                tr("Select rar executable"),
                QFileInfo(_ngPost->_rarPath).absolutePath()
                );

    if (!path.isEmpty())
    {
        QFileInfo fi(path);
        if (fi.isFile() && fi.isExecutable())
            _ui->rarEdit->setText(path);
        else
            _ngPost->error(tr("the selected file is not executable..."));
    }
}

void AutoPostWidget::onSelectAutoDirClicked()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->autoDirEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
        _ui->autoDirEdit->setText(path);
}

void AutoPostWidget::onScanAutoDirClicked()
{
    QDir autoDir(_ui->autoDirEdit->text());
    if (autoDir.exists())
    {
        _ui->filesList->clear2();
        QDir::SortFlags sort = _ui->latestFilesFirstCB->isChecked() ? QDir::Time : QDir::Name;
        for (const QFileInfo &file : autoDir.entryInfoList(QDir::Files|QDir::Dirs|QDir::NoDotAndDotDot, sort))
            if (!_ngPost->isPostedFolder(file))
                _ui->filesList->addPathIfNotInList(file.absoluteFilePath(), 0, file.isDir());
    }
    else
        QMessageBox::warning(nullptr,
                             tr("No auto directory selected..."),
                             tr("There is no auto directory!\nPlease select one."));
}

void AutoPostWidget::onMonitoringClicked()
{
    if (_isMonitoring)
    {
        if (_ngPost->hasMonitoringPostingJobs())
        {
            int res = QMessageBox::question(this,
                                  tr("Ongoing Monitoring post"),
                                  tr("There are still ongoing or pending Monitoring Posts.\n We're going to stop all of them.\nAre you sure you want to proceed?")
                                  );
            if (res == QMessageBox::No)
                return;
            else
                _ngPost->closeAllMonitoringJobs();
        }
        _ui->filesList->clear2();
        _ngPost->_stopMonitoring();
        _ui->monitorButton->setText(tr("Start watching"));
    }
    else
    {
        _currentPostIdx = 1;
        _ui->filesList->clear2();
        QString folderPath = _ui->autoDirEdit->text();
        QFileInfo fi(folderPath);
        if (!fi.exists() || !fi.isDir() || !fi.isReadable())
        {
            QMessageBox::warning(this, tr("Error accessing Auto Dir..."), tr("The auto directory must exist and be readable..."));
            return;
        }
        if (!_ui->compressCB->isChecked()) {
            QMessageBox::warning(this,
                                 tr("To be implemented..."),
                                 tr("You can't monitor a folder without compression using the GUI...\nIt's possible in command line if MONITOR_IGNORE_DIR is enabled in your configuration file."));
            return;
        }
        if (!_checkBeforeStart(QFileInfoList(), false)) // the same checks as Quick Post, before watching
            return;
        _hmi->updateServers();
        _hmi->updateParams();
        udatePostingParams();
        _ui->filesList->addItem(new QListWidgetItem(QIcon(":/icons/monitor.png"), tr("Watching %1").arg(folderPath)));
        _ngPost->_startMonitoring(folderPath);
        _ui->monitorButton->setText(tr("Stop watching"));
    }

    for (QPushButton *b : _modeButtons)
        b->setEnabled(_isMonitoring);
    _ui->compressPathEdit->setEnabled(_isMonitoring);
    _ui->compressPathButton->setEnabled(_isMonitoring);
    _ui->rarEdit->setEnabled(_isMonitoring);
    _ui->rarPathButton->setEnabled(_isMonitoring);
    _ui->rarSizeEdit->setEnabled(_isMonitoring);
    _ui->rarMaxCB->setEnabled(_isMonitoring);
    _ui->redundancySB->setEnabled(_isMonitoring);

    _ui->autoDirEdit->setEnabled(_isMonitoring);
    _ui->autoDirButton->setEnabled(_isMonitoring);
    _ui->latestFilesFirstCB->setEnabled(_isMonitoring);
    _ui->scanAutoDirButton->setEnabled(_isMonitoring);

    _ui->genNameCB->setEnabled(_isMonitoring);
    _ui->nameLengthSB->setEnabled(_isMonitoring);
    _ui->genPassCB->setEnabled(_isMonitoring);
    _ui->passLengthSB->setEnabled(_isMonitoring);

    _ui->par2CB->setEnabled(_isMonitoring);

    _ui->startJobsCB->setEnabled(_isMonitoring);
    _ui->postButton->setEnabled(_isMonitoring);

    _isMonitoring = !_isMonitoring;
    _ui->addMonitoringFolderButton->setEnabled(_isMonitoring);

    if (_isMonitoring)
        _ui->filesList->setSelectionMode(QAbstractItemView::NoSelection);
    else
        _ui->filesList->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void AutoPostWidget::newFileToProcess(const QFileInfo &fileInfo)
{
    QListWidgetItem *newItem = new QListWidgetItem(
                QIcon(fileInfo.isDir()?":/icons/folder.png":":/icons/file.png"),
                QString("- %1").arg(fileInfo.absoluteFilePath()));
    newItem->setForeground(_hmi->sPendingColor);
    _ui->filesList->addItem(newItem);
}

void AutoPostWidget::onAfterPostChanged()
{
    const int mode = _afterCombo->currentIndex();
    _postedEdit->setVisible(mode == 1);
    _postedButton->setVisible(mode == 1);
    if (mode == 2)
        QMessageBox::warning(this,
                         tr("Deleting files/folders once posted"),
                         tr("You're about to delete files from your computer once they've been posted!\nUse it at your own risk!\nIt will be irreversible..."));
    _ngPost->setDelFilesAfterPosted(mode != 0);
    _ngPost->setMovePosted(movePostedTo());
}

void AutoPostWidget::onPostedFolderClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select a Folder"), _ui->autoDirEdit->text(), QFileDialog::ShowDirsOnly);
    if (!path.isEmpty())
    {
        _postedEdit->setText(path);
        onAfterPostChanged();
    }
}

void AutoPostWidget::onAddMonitoringFolder()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Monitoring Folder to add"),
                _ui->autoDirEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
    {
        QFileInfo newDir(path);
        if (newDir.exists() && newDir.isDir() && newDir.isReadable())
        {
            _ui->filesList->addItem(new QListWidgetItem(QIcon(":/icons/monitor.png"), tr("Monitoring %1").arg(newDir.absoluteFilePath())));
            _ngPost->addMonitoringFolder(newDir.absoluteFilePath());
        }
    }
}

void AutoPostWidget::onCompressToggled(bool checked)
{
    _ui->genNameCB->setEnabled(checked);
    _ui->nameLengthSB->setEnabled(checked);
    _ui->genPassCB->setEnabled(checked);
    _ui->passLengthSB->setEnabled(checked);
    _ui->keepRarCB->setEnabled(checked);
    if (!checked && !_ui->par2CB->isChecked())
        _ui->par2CB->setChecked(true);
}

void AutoPostWidget::onGenPar2Toggled(bool checked)
{
    if (!checked && !_ui->compressCB->isChecked())
        _ui->compressCB->setChecked(true);
}

#include "PostingJob.h"
void AutoPostWidget::onMonitorJobStart()
{
    PostingJob *job = static_cast<PostingJob*>(sender());

    QString srcPath = QString("- %1").arg(job->getFirstOriginalFile());
    int nbFiles = _ui->filesList->count();
    for (int i = _currentPostIdx; i < nbFiles; ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->text() == srcPath)
        {
            item->setForeground(_hmi->sPostingColor);
            _currentPostIdx = i;
            break;
        }
    }
}

void AutoPostWidget::onSelectFilesClicked()
{
    if (!_isMonitoring)
    {
        QStringList files = QFileDialog::getOpenFileNames(
                    this,
                    tr("Select one or more files"),
                    _ngPost->_inputDir);

        int currentNbFiles = _ui->filesList->count();
        for (const QString &file : files)
            _ui->filesList->addPathIfNotInList(file, currentNbFiles);
    }
}

void AutoPostWidget::udatePostingParams()
{
    // fetch compression settings
    _ngPost->_doCompress = _ui->compressCB->isChecked();
    _ngPost->_genName    = _ngPost->_doCompress ? _ui->genNameCB->isChecked() : false;
    _ngPost->_genPass    = _ngPost->_doCompress ? _ui->genPassCB->isChecked() : false;
    _ngPost->_doPar2     = _ui->par2CB->isChecked();

    _ngPost->_tmpPath    = _ui->compressPathEdit->text();
    _ngPost->_rarPath    = _ui->rarEdit->text();
    _ngPost->_lengthName = static_cast<uint>(_ui->nameLengthSB->value());
    _ngPost->_lengthPass = static_cast<uint>(_ui->passLengthSB->value());
    uint val = 0;
    bool ok  = true;
    _ngPost->_rarSize = 0;
    if (!_ui->rarSizeEdit->text().isEmpty())
    {
        val = _ui->rarSizeEdit->text().toUInt(&ok);
        if (ok)
            _ngPost->_rarSize = val;
    }

    // fetch par2 settings
    if (_ngPost->_par2Args.isEmpty())
        _ngPost->_par2Pct = static_cast<uint>(_ui->redundancySB->value());

    QFileInfo inputDir(_ui->autoDirEdit->text());
    if (inputDir.exists() && inputDir.isDir() && inputDir.isWritable())
        _ngPost->_inputDir = inputDir.absoluteFilePath();


    _ngPost->_monitorExtensions.clear();
    if (!_ui->extensionFilterEdit->text().isEmpty())
    {
        for (QString &extension : _ui->extensionFilterEdit->text().split(","))
        {
            if (!extension.trimmed().isEmpty())
                _ngPost->_monitorExtensions << extension.trimmed();
        }
    }

    _ngPost->_monitorIgnoreDir = !_ui->dirAllowedCB->isChecked();

    _ngPost->_keepRar = _ui->keepRarCB->isChecked();
}

void AutoPostWidget::updateFinishedJob(const QString &path, uint nbArticles, uint nbUploaded, uint nbFailed)
{
    QString srcPath = QString("- %1").arg(path);
    int nbFiles = _ui->filesList->count();
    for (int i = 1; i < nbFiles; ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->text() == srcPath)
        {
            QColor color(_hmi->sDoneOKColor);
            if (nbUploaded > 0 && nbFailed == 0)
                item->setText(QString("%1 [%2 ok]").arg(srcPath).arg(nbArticles));
            else
            {
                item->setText(QString("%1 [%2 err / %3]").arg(srcPath).arg(nbFailed).arg(nbArticles));
                if (nbFailed == nbArticles)
                    color = _hmi->sDoneKOColor;
                else
                    color = _hmi->sArticlesFailedColor;
            }
            item->setForeground(color);
            if (nbUploaded > 0 && nbFailed == 0)
                item->setData(sCompletedRole, true); // cf onRemoveCompleted
            break;
        }
    }
}

void AutoPostWidget::onRemoveCompleted()
{
    // only the uploads finished without any error are removed: the failed ones stay in the list (to be seen or retried)
    for (int i = _ui->filesList->count() - 1; i >= 0; --i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->data(sCompletedRole).toBool())
        {
            _ui->filesList->removeItemWidget2(item);
            delete item;
        }
    }
}

void AutoPostWidget::onRemoveAll()
{
    // while monitoring, the first item is the "Monitoring <folder>" line: we keep it
    int first = _isMonitoring ? 1 : 0;
    if (_ui->filesList->count() <= first)
        return;

    if (QMessageBox::question(this, tr("Remove All"),
                              tr("Remove all the files and folders from the list?\n"
                                 "The posts already started and the files on disk are not touched."))
            != QMessageBox::Yes)
        return;

    if (first == 0)
        _ui->filesList->clear2();
    else
    {
        for (int i = _ui->filesList->count() - 1; i >= first; --i)
            delete _ui->filesList->takeItem(i);
        _currentPostIdx = 1;
    }
}

bool AutoPostWidget::deleteFilesOncePosted() const { return _afterCombo && _afterCombo->currentIndex() == 2; }

QString AutoPostWidget::movePostedTo() const
{
    if (!_afterCombo || _afterCombo->currentIndex() != 1)
        return QString();
    QString folder = _postedEdit->text().trimmed();
    return folder.isEmpty() ? QStringLiteral("Posted") : folder;
}

void AutoPostWidget::retranslate()
{
    _ui->retranslateUi(this);
    _retranslateLayout();
    if (_profileBar)
        _profileBar->retranslate();
    if (_nfoCard)
        _nfoCard->retranslate();
    _ui->rarMaxCB->setToolTip(tr("limit the number of archive volume to %1 (cf config RAR_MAX)").arg(_ngPost->_rarMax));
    _ui->redundancySB->setToolTip(tr("Using PAR2_ARGS from config file: %1").arg(_ngPost->_par2Args));
    _ui->removeCompletedButton->setToolTip(tr("Remove from the list the files/folders that have been uploaded without error"));
    _ui->removeAllButton->setToolTip(tr("Empty the whole list (the posts already started and the files on disk are not touched)"));
    _ui->closeTabsButton->setToolTip(tr("Close the Quick Post tabs whose upload is finished"));
    _ui->filesList->setToolTip(tr("Uncheck what you don't want to post. To remove items from the list, select them and press DEL."));
}

PostProfile AutoPostWidget::currentProfile() const
{
    PostProfile p;
    p.compress = _ui->compressCB->isChecked();
    p.genName  = _ui->genNameCB->isChecked();
    p.genPass  = _ui->genPassCB->isChecked();
    p.par2     = _ui->par2CB->isChecked();
    p.par2Pct  = static_cast<uint>(_ui->redundancySB->value());
    p.keepRar  = _ui->keepRarCB->isChecked();
    return p;
}

void AutoPostWidget::applyProfile(const PostProfile &p)
{
    _ui->compressCB->setChecked(p.compress);
    _ui->genNameCB->setChecked(p.genName);
    _ui->genPassCB->setChecked(p.genPass);
    _ui->par2CB->setChecked(p.par2);
    if (_ngPost->_par2Args.isEmpty() && p.par2Pct > 0)
        _ui->redundancySB->setValue(static_cast<int>(p.par2Pct));
    _ui->keepRarCB->setChecked(p.keepRar);
    onCompressToggled(p.compress);
}

void AutoPostWidget::setPackingAuto(bool enabled, const QStringList &keys){
    bool compress = false, genName = false, genPass = false, doPar2 = false;
    if (enabled)
    {
        for (auto it = keys.cbegin(), itEnd = keys.cend(); it != itEnd; ++it)
        {
            QString keyWord = (*it).toLower();
            if (keyWord == NgPost::optionName(NgPost::Opt::COMPRESS))
                compress = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_NAME))
                genName = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_PASS))
                genPass = true;
            else if (keyWord == NgPost::optionName(NgPost::Opt::GEN_PAR2))
                doPar2 = true;
        }
    }
    _ui->compressCB->setChecked(compress);
    _ui->genNameCB->setChecked(genName);
    _ui->genPassCB->setChecked(genPass);
    _ui->par2CB->setChecked(doPar2);
    onCompressToggled(compress);
}



void AutoPostWidget::handleKeyEvent(QKeyEvent *keyEvent)
{
    if (!_isMonitoring)
    {
        qDebug() << "[AutoPostWidget::handleKeyEvent] key event: " << keyEvent->key();
        if(keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
        {
            for (QListWidgetItem *item : _ui->filesList->selectedItems())
            {
                qDebug() << "[AutoPostWidget::handleKeyEvent] remove item: " << item->text();
                _ui->filesList->removeItemWidget2(item);
                delete item;
            }
        }
    }
}


void AutoPostWidget::handleDropEvent(QDropEvent *e)
{
    if (!_isMonitoring)
    {
        int currentNbFiles = _ui->filesList->count();
        for (const QUrl &url : e->mimeData()->urls())
        {
            QString fileName = url.toLocalFile();
            _ui->filesList->addPathIfNotInList(fileName, currentNbFiles, QFileInfo(fileName).isDir());
        }
    }
}


//========================================================================
// ngPost Deluxe layout: list, Source / Monitoring / Archive cards,
// advanced options hidden by default, Generate Posts button
//========================================================================
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>

static QFrame *newAutoCard(QLabel *&title, QGridLayout *&grid)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    card->setAttribute(Qt::WA_StyledBackground, true);
    QVBoxLayout *v = new QVBoxLayout(card);
    v->setContentsMargins(16, 12, 16, 14);
    v->setSpacing(10);
    title = new QLabel(card);
    title->setObjectName("cardTitle");
    v->addWidget(title);
    grid = new QGridLayout();
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(8);
    v->addLayout(grid);
    v->addStretch();
    return card;
}

static QHBoxLayout *autoRow(std::initializer_list<QWidget*> widgets, bool stretch = false)
{
    QHBoxLayout *h = new QHBoxLayout();
    h->setSpacing(8);
    for (QWidget *w : widgets)
        h->addWidget(w);
    if (stretch)
        h->addStretch();
    return h;
}

void AutoPostWidget::_buildLayout()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(page);
    root->setContentsMargins(2, 12, 2, 2);
    root->setSpacing(12);

    // the three ways of posting
    QHBoxLayout *modes = new QHBoxLayout();
    modes->setSpacing(0);
    QButtonGroup *group = new QButtonGroup(this);
    for (int i = 0; i < 3; ++i)
    {
        _modeButtons[i] = new QPushButton(page);
        _modeButtons[i]->setObjectName("modeButton");
        _modeButtons[i]->setCheckable(true);
        _modeButtons[i]->setCursor(Qt::PointingHandCursor);
        _modeButtons[i]->setMinimumHeight(40);
        group->addButton(_modeButtons[i], i);
        modes->addWidget(_modeButtons[i], 1);
        const Mode m = static_cast<Mode>(i);
        connect(_modeButtons[i], &QAbstractButton::clicked, this, [this, m](){ _setMode(m); });
    }
    root->addLayout(modes);

    // list: title + list actions
    QHBoxLayout *head = new QHBoxLayout();
    head->setSpacing(8);
    _filesTitle = new QLabel(page);
    _filesTitle->setObjectName("cardTitle");
    head->addWidget(_filesTitle);
    head->addStretch();
    head->addWidget(_ui->removeCompletedButton);
    head->addWidget(_ui->removeAllButton);
    head->addWidget(_ui->closeTabsButton);
    root->addLayout(head);
    _ui->filesList->setMinimumHeight(150);
    root->addWidget(_ui->filesList, 1);
    _summary = new QLabel(page);
    _summary->setObjectName("hint");
    _summary->hide();
    root->addWidget(_summary);

    // profiles of options (shared with Quick Post)
    _profileBar = new ProfileBar(_ngPost, page);
    _profileBar->setHandlers([this](){ return currentProfile(); }, [this](const PostProfile &p){ applyProfile(p); });
    root->addWidget(_profileBar);

    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(12);
    QGridLayout *g = nullptr;

    // source folder: scan
    QFrame *source = newAutoCard(_sourceTitle, g);
    g->addWidget(_ui->autoDirLbl, 0, 0);
    g->addLayout(autoRow({_ui->autoDirEdit, _ui->autoDirButton, _ui->scanAutoDirButton}), 0, 1);
    g->addWidget(_ui->extentionFilterLbl, 1, 0);
    g->addWidget(_ui->extensionFilterEdit, 1, 1);
    g->addWidget(_ui->dirAllowedCB, 2, 1);
    g->addWidget(_ui->latestFilesFirstCB, 3, 1);
    g->setColumnStretch(1, 1);
    _ui->autoDirEdit->setMinimumWidth(120);
    _ui->autoDirEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->extensionFilterEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    cards->addWidget(source, 3);
    _sourceCard = source;

    // monitoring
    QFrame *monitor = newAutoCard(_monitorTitle, g);
    g->addLayout(autoRow({_ui->monitorButton}, true), 0, 0);
    g->addLayout(autoRow({_ui->addMonitoringFolderButton}, true), 1, 0);
    _ui->delFilesCB->hide(); // replaced by the "After posting" card
    _ui->addMonitoringFolderButton->setMinimumSize(0, 0);
    _ui->addMonitoringFolderButton->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    cards->addWidget(monitor, 2);
    _monitorCard = monitor;

    // archive
    QFrame *archive = newAutoCard(_archiveTitle, g);
    g->addWidget(_ui->compressCB, 0, 0);
    g->addWidget(_ui->keepRarCB, 0, 1);
    g->addWidget(_ui->par2CB, 0, 2);
    g->addWidget(_ui->redundancySB, 0, 3);
    g->addWidget(_ui->genNameCB, 1, 0);
    g->addWidget(_ui->nameLengthSB, 1, 1);
    g->addWidget(_ui->genPassCB, 1, 2);
    g->addWidget(_ui->passLengthSB, 1, 3);
    _ui->redundancyLbl->hide();
    _ui->redundancySB->setSuffix(" %");
    g->setColumnStretch(4, 1);
    cards->addWidget(archive, 3);
    root->addLayout(cards);

    // NFO file (shared with Quick Post)
    _nfoCard = new NfoCard(_ngPost, page);
    root->addWidget(_nfoCard);

    // after posting: nothing (default), move to a "Posted" folder, or delete (one row, like the profile)
    QWidget *afterRow = new QWidget(page);
    _afterCard = afterRow;
    QHBoxLayout *afterLayout = new QHBoxLayout(afterRow);
    afterLayout->setContentsMargins(2, 0, 2, 0);
    afterLayout->setSpacing(8);
    _afterTitle = new QLabel(afterRow);
    _afterCombo = new QComboBox(afterRow);
    _afterCombo->addItems({QString(), QString(), QString()});
    _postedEdit = new QLineEdit(afterRow);
    _postedEdit->setText("Posted");
    _postedButton = new QPushButton(afterRow);
    _postedButton->setText("...");
    _postedButton->setFixedWidth(40);
    _postedEdit->setMaximumWidth(420);
    afterLayout->addWidget(_afterTitle);
    afterLayout->addWidget(_afterCombo);
    afterLayout->addWidget(_postedEdit, 1);
    afterLayout->addWidget(_postedButton);
    afterLayout->addStretch(1);
    _postedEdit->hide();
    _postedButton->hide();
    connect(_afterCombo, QOverload<int>::of(&QComboBox::activated), this, [this](){ onAfterPostChanged(); });
    connect(_postedEdit, &QLineEdit::editingFinished, this, [this](){ onAfterPostChanged(); });
    connect(_postedButton, &QAbstractButton::clicked, this, &AutoPostWidget::onPostedFolderClicked);
    root->addWidget(_afterCard);

    // advanced options
    _advancedCard = newAutoCard(_advancedTitle, g);
    g->addWidget(_ui->compressPathLbl, 0, 0);
    g->addLayout(autoRow({_ui->compressPathEdit, _ui->compressPathButton}), 0, 1);
    g->addWidget(_ui->rarLbl, 1, 0);
    g->addLayout(autoRow({_ui->rarEdit, _ui->rarPathButton}), 1, 1);
    g->addWidget(_ui->rarSizeLbl, 2, 0);
    g->addLayout(autoRow({_ui->rarSizeEdit, _ui->rarMaxCB}, true), 2, 1);
    g->setColumnStretch(1, 1);
    _ui->compressPathEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarSizeEdit->setMaximumWidth(100);
    _advancedCard->hide();
    root->addWidget(_advancedCard);

    // a change made by hand: the profile is now "Custom"
    for (QAbstractButton *b : {static_cast<QAbstractButton*>(_ui->compressCB), static_cast<QAbstractButton*>(_ui->genNameCB),
                               static_cast<QAbstractButton*>(_ui->genPassCB), static_cast<QAbstractButton*>(_ui->par2CB),
                               static_cast<QAbstractButton*>(_ui->keepRarCB)})
        connect(b, &QAbstractButton::clicked, _profileBar, &ProfileBar::markCustom);
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), _profileBar, &ProfileBar::markCustom);

    // bottom: advanced toggle, start option and Generate Posts
    QGridLayout *bottom = new QGridLayout();
    bottom->setHorizontalSpacing(12);
    _advancedToggle = new QPushButton(page);
    _advancedToggle->setObjectName("linkButton");
    _advancedToggle->setCursor(Qt::PointingHandCursor);
    connect(_advancedToggle, &QAbstractButton::clicked, this, [this](){
        _showAdvanced = !_showAdvanced;
        _advancedCard->setVisible(_showAdvanced);
        _retranslateLayout();
    });
    bottom->addWidget(_advancedToggle, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    _ui->postButton->setObjectName("accentButton");
    _ui->postButton->setIcon(QIcon());
    _ui->postButton->setMinimumSize(170, 38);
    bottom->addWidget(_ui->postButton, 0, 1, Qt::AlignCenter);
    bottom->addWidget(_ui->startJobsCB, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    bottom->setColumnStretch(0, 1);
    bottom->setColumnStretch(1, 0);
    bottom->setColumnStretch(2, 1);
    root->addLayout(bottom);

    _ui->aboutButton->hide(); // About is in the side bar

    // the .ui limited the width of some labels and check boxes (they were on one long line)
    for (QWidget *w : page->findChildren<QWidget*>())
        if (qobject_cast<QLabel*>(w) || qobject_cast<QCheckBox*>(w))
            w->setMaximumWidth(QWIDGETSIZE_MAX);

    delete layout();
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(page);

    _retranslateLayout();
}

void AutoPostWidget::_retranslateLayout()
{
    if (!_filesTitle)
        return;
    _modeButtons[0]->setText(tr("Post what I choose"));
    _modeButtons[1]->setText(tr("Scan a folder once"));
    _modeButtons[2]->setText(tr("Watch a folder and post what arrives"));
    _modeButtons[0]->setToolTip(tr("Add the files and folders you want, then post them."));
    _modeButtons[1]->setToolTip(tr("List what is in a folder, uncheck what you don't want, then post."));
    _modeButtons[2]->setToolTip(tr("Every new file or folder that arrives in the folder is posted automatically."));
    _monitorTitle->setText(tr("Watching"));
    _filesTitle->setText(tr("Files and folders to post"));
    _archiveTitle->setText(tr("Archive"));
    _advancedTitle->setText(tr("Advanced options"));
    _afterTitle->setText(tr("After posting:"));
    _afterCombo->setItemText(0, tr("Do nothing with the files"));
    _afterCombo->setItemText(1, tr("Move them to a \"Posted\" folder"));
    _afterCombo->setItemText(2, tr("Delete them"));
    _postedEdit->setToolTip(tr("A name like Posted creates this folder next to each file or folder. A full path is used as it is. The files are moved only when everything is posted without error."));
    _advancedToggle->setText(_showAdvanced ? tr("▾ Hide advanced options") : tr("▸ Advanced options (temporary folder, rar / 7z, volumes)"));

    _ui->autoDirLbl->setText(tr("Folder"));
    _ui->extentionFilterLbl->setText(tr("Extensions"));
    _ui->extensionFilterEdit->setPlaceholderText(tr("all (ex: mkv,mp4,iso)"));
    _ui->scanAutoDirButton->setText(tr("Scan"));
    _ui->dirAllowedCB->setText(tr("post the folders"));
    _ui->addMonitoringFolderButton->setText(tr("Add another folder"));
    _ui->addMonitoringFolderButton->setToolTip(tr("Watch one more folder while the watching is running"));
    _ui->monitorButton->setText(_isMonitoring ? tr("Stop watching") : tr("Start watching"));
    _ui->compressCB->setText(tr("Compress"));
    _ui->keepRarCB->setText(tr("keep the archives"));
    _ui->genNameCB->setText(tr("random name"));
    _ui->genPassCB->setText(tr("random password"));
    _ui->par2CB->setText(tr("par2"));
    _ui->redundancyLbl->setText(tr("redundancy (%)"));
    _ui->compressPathLbl->setText(tr("Temporary folder"));
    _ui->rarLbl->setText(tr("rar / 7z program"));
    _ui->rarSizeLbl->setText(tr("Volume size (MB)"));
    _ui->rarMaxCB->setText(tr("limit the number of volumes"));
    _ui->startJobsCB->setText(tr("start the posts directly"));
    _applyMode();
}

//========================================================================
// the three modes, the check boxes and sizes of the list, the checks
//========================================================================
namespace {
//! draws the size of the item at the right of its name
class SizeDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        QVariant size = index.data(Qt::UserRole + 2);
        if (!size.isValid())
        {
            QStyledItemDelegate::paint(painter, option, index);
            return;
        }
        const int w = 90;
        QStyleOptionViewItem opt(option);
        opt.rect.setRight(option.rect.right() - w);
        QStyledItemDelegate::paint(painter, opt, index);
        qint64 bytes = size.toLongLong();
        painter->save();
        painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
        painter->drawText(QRect(option.rect.right() - w, option.rect.top(), w - 8, option.rect.height()),
                          Qt::AlignRight | Qt::AlignVCenter, bytes < 0 ? QStringLiteral("...") : NfoWriter::humanSize(bytes));
        painter->restore();
    }
};
}

void AutoPostWidget::_setMode(Mode mode)
{
    if (_isMonitoring)
        return;
    _mode = mode;
    _ngPost->_autoMode = mode == Mode::Scan ? "scan" : mode == Mode::Watch ? "watch" : "choose";
    _applyMode();
    _decorateItems();
}

void AutoPostWidget::_applyMode()
{
    if (!_sourceCard)
        return;
    const bool choose = _mode == Mode::Choose, scan = _mode == Mode::Scan, watch = _mode == Mode::Watch;
    _modeButtons[0]->setChecked(choose);
    _modeButtons[1]->setChecked(scan);
    _modeButtons[2]->setChecked(watch);

    _sourceCard->setVisible(!choose);
    _sourceTitle->setText(watch ? tr("Folder to watch") : tr("Folder to scan"));
    _ui->scanAutoDirButton->setVisible(scan);
    _ui->latestFilesFirstCB->setVisible(scan);
    _monitorCard->setVisible(watch);
    _ui->postButton->setVisible(!watch);
    _ui->startJobsCB->setVisible(!watch);
    _ui->postButton->setText(tr("Post the list"));
    _filesTitle->setText(watch ? tr("Posts of the watched folder") : tr("Files and folders to post"));
    _ui->removeCompletedButton->setVisible(true);

    if (choose)
        _ui->filesList->setEmptyState(tr("Drag here the files or folders to post, or choose them: each file or folder becomes a post."),
                                      tr("Select Files"), tr("Select a Folder"));
    else if (scan)
        _ui->filesList->setEmptyState(tr("Choose the folder to scan below and press Scan: what is inside is listed here, then uncheck what you don't want to post."),
                                      QString(), QString());
    else
        _ui->filesList->setEmptyState(tr("Choose the folder to watch below, then press Start watching: every new file or folder that arrives is posted."),
                                      QString(), QString());
    _updateSummary();
}

void AutoPostWidget::_decorateItems()
{
    if (_isMonitoring || _mode == Mode::Watch)
        return;
    if (!dynamic_cast<SizeDelegate*>(_ui->filesList->itemDelegate()))
        _ui->filesList->setItemDelegate(new SizeDelegate(_ui->filesList));
    bool added = false;
    QSignalBlocker blocker(_ui->filesList);
    for (int i = 0; i < _ui->filesList->count(); ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->data(sSizeRole).isValid())
            continue;
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        item->setData(sSizeRole, qint64(-1));
        added = true;
    }
    blocker.unblock();
    if (added)
        _sizeTimer->start(0);
    _updateSummary();
}

void AutoPostWidget::_computeNextSize()
{
    for (int i = 0; i < _ui->filesList->count(); ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        QVariant size = item->data(sSizeRole);
        if (size.isValid() && size.toLongLong() < 0)
        {
            QSignalBlocker blocker(_ui->filesList);
            item->setData(sSizeRole, NgPost::recursiveSize(QFileInfo(item->text())));
            blocker.unblock();
            _ui->filesList->viewport()->update();
            _updateSummary();
            _sizeTimer->start(0); // one item at a time to keep the window responsive
            return;
        }
    }
    _updateSummary();
}

QFileInfoList AutoPostWidget::_checkedFiles(bool *hasFolder) const
{
    QFileInfoList files;
    if (hasFolder)
        *hasFolder = false;
    for (int i = 0; i < _ui->filesList->count(); ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->checkState() != Qt::Checked)
            continue;
        QFileInfo fi(item->text());
        if (!fi.exists())
            continue;
        files << fi;
        if (hasFolder && fi.isDir())
            *hasFolder = true;
    }
    return files;
}

void AutoPostWidget::_updateSummary()
{
    if (!_summary)
        return;
    if (_isMonitoring || _mode == Mode::Watch || _ui->filesList->count() == 0)
    {
        _summary->hide();
        return;
    }
    int total = _ui->filesList->count(), checked = 0;
    qint64 bytes = 0;
    bool pending = false;
    for (int i = 0; i < total; ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->checkState() != Qt::Checked)
            continue;
        ++checked;
        qint64 size = item->data(sSizeRole).isValid() ? item->data(sSizeRole).toLongLong() : -1;
        if (size < 0)
            pending = true;
        else
            bytes += size;
    }
    QString text = tr("%1 of %2 selected").arg(checked).arg(total);
    if (pending)
        text += QString::fromUtf8("  ·  ") + tr("calculating the size...");
    else
    {
        text += ", " + NfoWriter::humanSize(bytes);
        qint64 up = PostChecker::uploadBytes(bytes, _ui->par2CB->isChecked(), static_cast<uint>(_ui->redundancySB->value()));
        if (_ui->compressCB->isChecked() || _ui->par2CB->isChecked())
            text += QString::fromUtf8(" → ") + tr("about %1 to upload").arg(NfoWriter::humanSize(up));
        QString time = PostChecker::durationText(up, _ngPost->_lastUploadSpeed);
        text += QString::fromUtf8("  ·  ") + (time.isEmpty() ? tr("upload time: shown after the first post") : tr("upload time: about %1").arg(time));
    }
    _summary->setText(text);
    _summary->show();
}

bool AutoPostWidget::_checkBeforeStart(const QFileInfoList &files, bool hasFolder)
{
    auto makeInput = [this, &files, hasFolder]() {
        _hmi->updateServers();
        _hmi->updateParams();
        udatePostingParams();

        PostChecker::Input in;
        for (NntpServerParams *srv : _ngPost->_nntpServers)
            if (srv->enabled)
                ++in.nbServers;
        in.hasGroups     = _hmi->hasGroups();
        in.invalidGroups = _hmi->invalidGroups().join(", ");
        in.hasFolder     = hasFolder;
        in.compress      = _ngPost->_doCompress;
        in.rarPath       = _ngPost->_rarPath;
        in.tmpPath       = _ngPost->_tmpPath;
        in.par2          = _ngPost->_doPar2;
        in.par2Path      = _ngPost->_par2Path;
        in.par2Pct       = _ngPost->_par2Pct;
        in.passwordChecked = false; // each post has its own password
        if (!_ngPost->_nzbPath.isEmpty())
            in.nzbFile = QDir(_ngPost->_nzbPath).filePath("ngpost.nzb");
        for (const QFileInfo &fi : files)
            in.sourceBytes += NgPost::recursiveSize(fi);
        if (in.compress || in.par2)
        {
            QStorageInfo storage(QFileInfo(in.tmpPath).exists() ? in.tmpPath : QDir::tempPath());
            in.freeBytesTmp = storage.isValid() ? storage.bytesAvailable() : -1;
        }
        in.mediaInfoWanted = _ngPost->_genNfo && _ngPost->_nfoMedia;
        if (in.mediaInfoWanted)
        {
            in.mediaInfoFound = !MediaInfoReader::findProgram(_ngPost->_mediaInfoPath).isEmpty();
            if (files.isEmpty())
                in.hasMediaFiles = true; // watching: we don't know yet what will arrive
            for (const QFileInfo &fi : files)
            {
                if (fi.isDir())
                {
                    QDirIterator it(fi.absoluteFilePath(), QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
                    while (!in.hasMediaFiles && it.hasNext())
                        in.hasMediaFiles = MediaInfoReader::isMediaFile(it.next());
                }
                else
                    in.hasMediaFiles = MediaInfoReader::isMediaFile(fi.fileName());
                if (in.hasMediaFiles)
                    break;
            }
        }
        return in;
    };
    return PostCheckDialog::run(this, makeInput, [this](PostChecker::Code code){ return _applyFix(code); });
}

bool AutoPostWidget::_applyFix(PostChecker::Code code)
{
    using Code = PostChecker::Code;
    switch (code)
    {
    case Code::NoServer:
        _hmi->showServersPage();
        return false;
    case Code::NoGroup:
    case Code::NzbFolderNotWritable:
        _hmi->showSettingsPage();
        return false;
    case Code::FoldersNoCompress:
        _ui->compressCB->setChecked(true);
        break;
    case Code::RarMissing:
        onRarPathClicked();
        break;
    case Code::Par2Missing:
        _ui->par2CB->setChecked(false);
        break;
    case Code::MediaInfoMissing:
        _ngPost->_nfoMedia = false;
        emit _ngPost->nfoOptionsChanged();
        break;
    default:
        break;
    }
    return true;
}
