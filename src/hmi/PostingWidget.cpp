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

#include "PostingWidget.h"
#include <QDialog>
#include <QDirIterator>
#include <QStorageInfo>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "ProfileBar.h"
#include "PostCheckDialog.h"
#include "NfoCard.h"
#include "utils/PostProfile.h"
#include "utils/NfoWriter.h"
#include "nntp/NntpServerParams.h"
#include "utils/MediaInfoReader.h"
#include "ui_PostingWidget.h"
#include "MainWindow.h"
#include <QCoreApplication>
#include "AboutNgPost.h"
#include "NgPost.h"
#include "PostingJob.h"
#include "nntp/NntpFile.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QKeyEvent>
#include <QClipboard>
#include <QMimeData>
#include <QJsonArray>



PostingWidget::PostingWidget(NgPost *ngPost, MainWindow *hmi, uint jobNumber) :
    QWidget(hmi),
    _ui(new Ui::PostingWidget),
    _hmi(hmi),
    _ngPost(ngPost),
    _jobNumber(jobNumber),
    _postingJob(nullptr),
    _state(STATE::IDLE),
    _postingFinished(false),
    _filesTitle(nullptr), _releaseInfo(nullptr), _nzbTitle(nullptr), _archiveTitle(nullptr), _advancedTitle(nullptr),
    _advancedCard(nullptr), _profileBar(nullptr), _nfoCard(nullptr), _summary(nullptr), _sourceBytes(0), _advancedToggle(nullptr), _repostButton(nullptr), _repostInfo()
{
    _ui->setupUi(this);
    _buildLayout();

    connect(_ui->postButton, &QAbstractButton::clicked, this, &PostingWidget::onPostFiles);
    connect(_ui->nzbPassCB,  &QAbstractButton::toggled, this, &PostingWidget::onNzbPassToggled);
    connect(_ui->genPass,    &QAbstractButton::clicked, this, &PostingWidget::onGenNzbPassword);

    _ui->filesList->setEmptyState(tr("Drag files or folders here, or choose them with the buttons below."),
                                  tr("Select Files"), tr("Select Folder"));
    connect(_ui->filesList, &SignedListWidget::emptyAction1, this, &PostingWidget::onSelectFilesClicked);
    connect(_ui->filesList, &SignedListWidget::emptyAction2, this, &PostingWidget::onSelectFolderClicked);
    connect(_ui->filesList, &SignedListWidget::rightClick, this, &PostingWidget::onSelectFilesClicked);
}

PostingWidget::~PostingWidget()
{
    delete _ui;
}

void PostingWidget::onFilePosted(QString filePath, uint nbArticles, uint nbFailed)
{
    int nbFiles = _ui->filesList->count();
    for (int i = 0 ; i < nbFiles ; ++i)
    {
        QListWidgetItem *item = _ui->filesList->item(i);
        if (item->text() == filePath)
        {
            QColor color(_hmi->sDoneOKColor);
            if (nbFailed == 0)
                item->setText(QString("%1 [%2 ok]").arg(filePath).arg(nbArticles));
            else
            {
                item->setText(QString("%1 [%2 err / %3]").arg(filePath).arg(nbFailed).arg(nbArticles));
                if (nbFailed == nbArticles)
                    color = _hmi->sDoneKOColor;
                else
                    color = _hmi->sArticlesFailedColor;
            }
            item->setForeground(color);
            break;
        }
    }
}

void PostingWidget::onArchiveFileNames(QStringList paths)
{
    _ui->filesList->clear();
    for (const QString & path : paths)
        _ui->filesList->addPath(path);
}

void PostingWidget::onArticlesNumber(int nbArticles)
{
    Q_UNUSED(nbArticles);
    _hmi->setJobLabel(static_cast<int>(_jobNumber));
}

void PostingWidget::onPostingJobDone()
{
    // we could arrive here twice: from PostingJob::postingFinished or PostingJob::noMoreConnection
    // This could happen especially when we exceed the number of connections allowed by a provider
    if (!_postingJob)
        return;
    _updateSummary(false); // the speed of this upload is known now

    if (_postingJob->nbArticlesTotal() > 0)
    {
        if (_postingJob->nbArticlesFailed() > 0)
            _hmi->updateJobTab(this, _hmi->sDoneKOColor, QIcon(_hmi->sDoneKOIcon));
        else
            _hmi->updateJobTab(this, _hmi->sDoneOKColor, QIcon(_hmi->sDoneOKIcon));
    }
    else
        _hmi->clearJobTab(this);

    bool uploadedOk = _postingJob->nbArticlesTotal() > 0 && _postingJob->nbArticlesFailed() == 0
                      && _postingJob->nbArticlesUploaded() > 0;
    RepostInfo previousInfo = _repostInfo;
    _repostInfo = _postingJob->repostInfo(); // the job will be deleted, we keep what is needed to post the missing Articles again
    if (_postingJob->isRepost())
        _refreshFileItems(previousInfo);
    disconnect(_postingJob);
    _postingJob = nullptr; //!< we don't own it, NgPost will delete it
    _postingFinished = true;
    setIDLE();
    emit postingDone(uploadedOk);
}

bool PostingWidget::_applyFix(PostChecker::Code code)
{
    using Code = PostChecker::Code;
    switch (code)
    {
    case Code::NoServer:
        _hmi->showServersPage();
        return false;
    case Code::NoGroup:
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
    case Code::PasswordEmpty:
        onGenNzbPassword();
        break;
    case Code::NzbFolderNotWritable:
        onNzbFileClicked();
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

bool PostingWidget::_checkBeforePosting(const QFileInfoList &files, bool hasFolder)
{
    auto makeInput = [this, &files, hasFolder]() {
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
        in.passwordChecked = _ui->nzbPassCB->isChecked();
        in.password      = _ui->nzbPassEdit->text();
        in.nzbFile       = _ngPost->nzbPath();
        if (!in.nzbFile.endsWith(".nzb"))
            in.nzbFile += ".nzb";
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

void PostingWidget::_updateSummary(bool recomputeSize)
{
    if (!_summary)
        return;
    if (_ui->filesList->count() == 0)
    {
        _summary->hide();
        return;
    }
    if (recomputeSize)
    {
        QFileInfoList files;
        bool hasFolder = false;
        _buildFilesList(files, hasFolder);
        _sourceBytes = 0;
        for (const QFileInfo &fi : files)
            _sourceBytes += NgPost::recursiveSize(fi);
    }
    bool pack = _ui->compressCB->isChecked() || _ui->par2CB->isChecked();
    QString text = tr("%1 item(s), %2").arg(_ui->filesList->count()).arg(NfoWriter::humanSize(_sourceBytes));
    qint64 up = PostChecker::uploadBytes(_sourceBytes, _ui->par2CB->isChecked(), static_cast<uint>(_ui->redundancySB->value()));
    if (pack)
        text += QString::fromUtf8(" \u2192 ") + tr("about %1 to upload").arg(NfoWriter::humanSize(up));
    QString time = PostChecker::durationText(up, _ngPost->_lastUploadSpeed);
    text += QString::fromUtf8("  \u00b7  ") + (time.isEmpty() ? tr("upload time: shown after the first post") : tr("upload time: about %1").arg(time));
    _summary->setText(text);
    _summary->show();
}

void PostingWidget::onPostFiles()
{
    postFiles(true);
}

void PostingWidget::postFiles(bool updateMainParams)
{
    if (_state == STATE::IDLE)
    {
        if (_ui->filesList->count() == 0)
        {
            _hmi->logError(tr("There are no selected files to post..."));
            return;
        }

        QFileInfoList files;
        bool hasFolder = false;
        _buildFilesList(files, hasFolder);
        if (files.isEmpty())
        {
            _hmi->logError(tr("There are no existing files to post..."));
            return;
        }

        if (updateMainParams)
        {
            // post started with the button: the checks explain what is wrong and fix it with one click
            _hmi->updateServers();
            _hmi->updateParams();
            if (!_checkBeforePosting(files, hasFolder))
                return;
        }
        else if (hasFolder && !_ui->compressCB->isChecked())
        {
            _hmi->logError(tr("You can't post folders without using compression..."));
            return;
        }
        udatePostingParams();

        // check if the nzb file name already exist
        QString nzbPath = _ngPost->nzbPath();
        if (!nzbPath.endsWith(".nzb"))
            nzbPath += ".nzb";
        QFileInfo fiNzb(nzbPath);
        if (!_fromAuto && fiNzb.exists())
        {
            int overwrite = QMessageBox::question(nullptr,
                                                  tr("Overwrite existing nzb file?"),
                                                  tr("The nzb file '%1' already exists.\nWould you like to overwrite it ?").arg(nzbPath),
                                                  QMessageBox::Yes,
                                                  QMessageBox::No);
            if (overwrite == QMessageBox::No)
                return;
        }

        // already posted (same name and size in the history)?
        QString posted = _fromAuto ? QString() : _ngPost->duplicateInPost(fiNzb.fileName(), files);
        if (!posted.isEmpty())
        {
            QMessageBox box(QMessageBox::Question, tr("Already posted?"),
                            tr("This post is already in the history (%1, posted on %2).\nDo you want to post it again?").arg(fiNzb.fileName(), posted),
                            QMessageBox::NoButton, this);
            QPushButton *cancel = box.addButton(tr("Cancel"), QMessageBox::RejectRole);
            box.addButton(tr("Post anyway"), QMessageBox::AcceptRole);
            box.setDefaultButton(cancel);
            box.exec();
            if (box.clickedButton() == cancel)
                return;
        }

        _postingFinished = false;
        _state = STATE::POSTING;
        forgetQueue();              // a new post: the missing Articles of the previous one are forgotten
        _repostInfo = RepostInfo();
        _postingJob = new PostingJob(_ngPost, nzbPath, files, this,
                                     _ngPost->getPostingGroups(),
                                     _ngPost->from(),
                                     _ngPost->_obfuscateArticles, _ngPost->_obfuscateFileName,
                                     _ngPost->_tmpPath, _ngPost->_rarPath, _ngPost->_rarArgs,
                                     _ngPost->_rarSize, _ngPost->_useRarMax, _ngPost->_par2Pct,
                                     _ngPost->_doCompress, _ngPost->_doPar2,
                                     _ngPost->_rarName, _ngPost->_rarPass,
                                     _ngPost->_keepRar, _afterAct, !_fromAuto);
        _postingJob->setMovePostedTo(_afterMoveTo);

        _showJobStarted(_ngPost->startPostingJob(_postingJob));
    }
    else  if (_state == STATE::POSTING)
    {
        _state = STATE::STOPPING;
        emit _postingJob->stopPosting();
    }
}


bool PostingWidget::restoreFromQueue(const QJsonObject &entry, QString &error)
{
    PostingJob *job = PostingJob::fromQueueEntry(_ngPost, entry, this, error);
    if (!job)
        return false;

    // show what is posted
    _ui->filesList->clear();
    for (const QJsonValue &val : entry.value("files").toArray())
    {
        QFileInfo fi(val.toString());
        _ui->filesList->addPath(fi.absoluteFilePath(), fi.isDir());
    }
    _ui->nzbFileEdit->setText(entry.value("nzb").toString());
    _ui->compressCB->setChecked(entry.value("compress").toBool());
    _ui->par2CB->setChecked(entry.value("par2").toBool());
    _ui->keepRarCB->setChecked(entry.value("keepRar").toBool());
    if (entry.value("compress").toBool())
        _ui->compressNameEdit->setText(entry.value("rarName").toString());
    const QString pass = entry.value("rarPass").toString();
    _ui->nzbPassCB->setChecked(!pass.isEmpty());
    _ui->nzbPassEdit->setText(pass);

    _postingFinished = false;
    _state = STATE::POSTING;
    _repostInfo = RepostInfo();
    _postingJob = job;
    _showJobStarted(_ngPost->startPostingJob(job));
    return true;
}

void PostingWidget::restoreMissing(const RepostInfo &info)
{
    _repostInfo = info;
    _ui->filesList->clear();
    for (const RepostFile &file : info.files)
    {
        _ui->filesList->addPath(file.path, false);
        QListWidgetItem *item = _ui->filesList->item(_ui->filesList->count() - 1);
        if (item)
        {
            item->setText(QString("%1 [%2 err / %3]").arg(file.path).arg(file.parts.size()).arg(file.nbArticles));
            item->setForeground(QColor(file.parts.size() == static_cast<int>(file.nbArticles) ? _hmi->sDoneKOColor
                                                                                               : _hmi->sArticlesFailedColor));
        }
    }
    _ui->nzbFileEdit->setText(info.nzbPath);
    _postingFinished = true;
    _hmi->updateJobTab(this, _hmi->sDoneKOColor, QIcon(_hmi->sDoneKOIcon), QFileInfo(info.nzbPath).fileName());
    _updateRepostButton();
    _updateReleaseInfo();
}

void PostingWidget::forgetQueue()
{
    if (hasMissingArticles())
        _ngPost->queueForget(_repostInfo.queueId);
}

void PostingWidget::_refreshFileItems(const RepostInfo &postedAgain)
{
    // the list shows "[x err / n]" for the files that had failed Articles: update it after the repost
    for (const RepostFile &file : postedAgain.files)
    {
        int missing = 0;
        for (const RepostFile &left : _repostInfo.files)
            if (left.path == file.path)
                missing = left.parts.size();

        for (int i = 0 ; i < _ui->filesList->count() ; ++i)
        {
            QListWidgetItem *item = _ui->filesList->item(i);
            if (!item->text().startsWith(file.path + " ["))
                continue;
            if (missing == 0)
            {
                item->setText(QString("%1 [%2 ok]").arg(file.path).arg(file.nbArticles));
                item->setForeground(QColor(_hmi->sDoneOKColor));
            }
            else
            {
                item->setText(QString("%1 [%2 err / %3]").arg(file.path).arg(missing).arg(file.nbArticles));
                item->setForeground(QColor(missing == static_cast<int>(file.nbArticles) ? _hmi->sDoneKOColor
                                                                                         : _hmi->sArticlesFailedColor));
            }
            break;
        }
    }
}

void PostingWidget::_showJobStarted(bool hasStarted)
{
    QString buttonTxt;
    QColor  tabColor;
    QString tabIcon;
    if (hasStarted)
    {
        buttonTxt = tr("Stop Posting");
        tabColor  = _hmi->sPostingColor;
        tabIcon   = _hmi->sPostingIcon;
    }
    else
    {
        buttonTxt = tr("Cancel Posting");
        tabColor  = _hmi->sPendingColor;
        tabIcon   = _hmi->sPendingIcon;
    }
    _ui->postButton->setText(buttonTxt);
    _hmi->updateJobTab(this, tabColor, QIcon(tabIcon), _postingJob->nzbName());
    _updateRepostButton();
    _updateReleaseInfo();
}

void PostingWidget::onRepostMissing()
{
    if (_state != STATE::IDLE || _repostInfo.isEmpty())
        return;

    _postingFinished = false;
    _state = STATE::POSTING;
    _postingJob = PostingJob::createRepostJob(_ngPost, _repostInfo, this);
    _showJobStarted(_ngPost->startPostingJob(_postingJob));
}

void PostingWidget::_updateRepostButton()
{
    if (!_repostButton)
        return;
    bool show = _state == STATE::IDLE && !_repostInfo.isEmpty();
    _repostButton->setVisible(show);
    if (show)
        _repostButton->setText(tr("Repost missing Articles (%1)").arg(_repostInfo.nbArticles()));
}

void PostingWidget::onNzbPassToggled(bool checked)
{
    _ui->nzbPassEdit->setEnabled(checked);
    _ui->passLengthSB->setEnabled(checked);
    _ui->genPass->setEnabled(checked);
}

void PostingWidget::onGenNzbPassword()
{
    _ui->nzbPassEdit->setText(_ngPost->randomPass(static_cast<uint>(_ui->passLengthSB->value())));
}

void PostingWidget::onSelectFilesClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                this,
                tr("Select one or more files to Post"),
                _ngPost->_inputDir);

    int currentNbFiles = _ui->filesList->count();
    for (const QString &file : files)
        addPath(file, currentNbFiles);
}

void PostingWidget::onSelectFolderClicked()
{
    QString folder = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ngPost->_inputDir,
                QFileDialog::ShowDirsOnly);

    if (!folder.isEmpty())
        addPath(folder, _ui->filesList->count(), true);
}

void PostingWidget::onClearFilesClicked()
{
    _ui->filesList->clear2();
    _ui->nzbFileEdit->clear();
    _ui->compressNameEdit->clear();
    if (_hmi->hasAutoCompress())
    {
        onGenCompressName();
        onGenNzbPassword();
    }
    else
        _ui->compressNameEdit->clear();

    _hmi->clearJobTab(this);
}

void PostingWidget::onCompressCB(bool checked)
{
    _ui->compressNameEdit->setEnabled(checked);
    _ui->nameLengthSB->setEnabled(checked);
    _ui->genCompressName->setEnabled(checked);
    _ui->keepRarCB->setEnabled(checked);
}

void PostingWidget::onGenCompressName()
{
    _ui->compressNameEdit->setText(_ngPost->randomPass(static_cast<uint>(_ui->nameLengthSB->value())));
}

void PostingWidget::onCompressPathClicked()
{
    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("Select a Folder"),
                _ui->compressPathEdit->text(),
                QFileDialog::ShowDirsOnly);

    if (!path.isEmpty())
        _ui->compressPathEdit->setText(path);
}

void PostingWidget::onNzbFileClicked()
{
    QString path = QFileDialog::getSaveFileName(
                this,
                tr("Create nzb file"),
                _ngPost->_nzbPath,
                "*.nzb"
                );

    if (!path.isEmpty())
        _ui->nzbFileEdit->setText(path);
}

void PostingWidget::onRarPathClicked()
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

void PostingWidget::handleKeyEvent(QKeyEvent *keyEvent)
{
    qDebug() << "[PostingWidget::handleKeyEvent] key event: " << keyEvent->key();

    if(keyEvent->key() == Qt::Key_Delete || keyEvent->key() == Qt::Key_Backspace)
    {
        for (QListWidgetItem *item : _ui->filesList->selectedItems())
        {
            qDebug() << "[PostingWidget::handleKeyEvent] remove item: " << item->text();
            _ui->filesList->removeItemWidget2(item);
            delete item;
        }
    }
    else if (keyEvent->matches(QKeySequence::Paste))
    {
        const QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        if (mimeData->hasImage()) {
            qDebug() << "[PostingWidget::handleKeyEvent] try to paste image...";
        } else if (mimeData->hasHtml()) {
            qDebug() << "[PostingWidget::handleKeyEvent] try to paste html: ";
        } else if (mimeData->hasText()) {
            QString txt = mimeData->text();
            qDebug() << "[PostingWidget::handleKeyEvent] paste text: " << txt;
            int currentNbFiles = _ui->filesList->count();
            for (const QString &path : txt.split(QRegularExpression("\n|\r|\r\n")))
            {
                QFileInfo fileInfo(path);
                if (!fileInfo.exists())
                    qDebug() << "[PostingWidget::handleKeyEvent] NOT A FILE: " << path;
                else
                    addPath(path, currentNbFiles, fileInfo.isDir());
//                        else if (fileInfo.isDir())
//                        {
//                            QDir dir(fileInfo.absoluteFilePath());
//                            for (const QFileInfo &subFile : dir.entryInfoList(QDir::Files, QDir::Name))
//                            {
//                                if (subFile.isReadable())
//                                    _addFile(subFile.absoluteFilePath(), currentNbFiles);
//                            }
//                        }
            }

        } else if (mimeData->hasUrls()) {
            qDebug() << "[PostingWidget::handleKeyEvent] paste urls...";

        } else {
            qDebug() << "[PostingWidget::handleKeyEvent] unknown type...";
        }
    }

}


void PostingWidget::handleDropEvent(QDropEvent *e)
{
    int currentNbFiles = _ui->filesList->count();
    for (const QUrl &url : e->mimeData()->urls())
    {
        QString fileName = url.toLocalFile();
        addPath(fileName, currentNbFiles, QFileInfo(fileName).isDir());
    }
}

void PostingWidget::_buildFilesList(QFileInfoList &files, bool &hasFolder)
{
    for (int i = 0 ; i < _ui->filesList->count() ; ++i)
    {
        QFileInfo fileInfo(_ui->filesList->item(i)->text());
        if (fileInfo.exists())
        {
            files << fileInfo;
            if (fileInfo.isDir())
                hasFolder = true;
        }
    }
}

void PostingWidget::init()
{
    _ui->rarMaxCB->setChecked(_ngPost->_useRarMax);

    _ui->nzbPassCB->setChecked(false);
    onNzbPassToggled(false);

    _ui->compressPathEdit->setText(_ngPost->_tmpPath);
    _ui->rarEdit->setText(_ngPost->_rarPath);

    _ui->rarSizeEdit->setText(QString::number(_ngPost->_rarSize));
    _ui->rarSizeEdit->setValidator(new QIntValidator(1, 1000000, _ui->rarSizeEdit));

    _ui->keepRarCB->setChecked(_ngPost->_keepRar);

    _ui->redundancySB->setRange(0, 100);
    _ui->redundancySB->setValue(static_cast<int>(_ngPost->_par2Pct));
    _ui->redundancySB->setEnabled(_ngPost->_par2Args.isEmpty());

    if (!_ngPost->_rarPassFixed.isEmpty())
    {
        _ui->nzbPassCB->setChecked(true);
        _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
    }

    _ui->nameLengthSB->setRange(5, 50);
    _ui->nameLengthSB->setValue(static_cast<int>(_ngPost->_lengthName));
    _ui->passLengthSB->setRange(5, 50);
    _ui->passLengthSB->setValue(static_cast<int>(_ngPost->_lengthPass));

    _ui->filesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(_ui->selectFilesButton, &QAbstractButton::clicked, this, &PostingWidget::onSelectFilesClicked);
    connect(_ui->selectFolderButton,&QAbstractButton::clicked, this, &PostingWidget::onSelectFolderClicked);
    connect(_ui->clearFilesButton,  &QAbstractButton::clicked, this, &PostingWidget::onClearFilesClicked);
    connect(_ui->filesList,         &SignedListWidget::empty,  this, &PostingWidget::onClearFilesClicked, Qt::QueuedConnection);

    connect(_ui->compressCB,        &QAbstractButton::toggled, this, &PostingWidget::onCompressCB);
    connect(_ui->genCompressName,   &QAbstractButton::clicked, this, &PostingWidget::onGenCompressName);


    connect(_ui->compressPathButton,&QAbstractButton::clicked, this, &PostingWidget::onCompressPathClicked);
    connect(_ui->rarPathButton,     &QAbstractButton::clicked, this, &PostingWidget::onRarPathClicked);

    connect(_ui->nzbFileButton,     &QAbstractButton::clicked, this, &PostingWidget::onNzbFileClicked);

    connect(_ui->aboutButton,       &QAbstractButton::clicked, _ngPost, &NgPost::onAboutClicked);


    onCompressCB(_ngPost->_doCompress);
    if (_ngPost->_doCompress)
        _ui->compressCB->setChecked(true);
    if (_ngPost->_genName)
        onGenCompressName();
    if (_ngPost->_genPass)
    {
        _ui->nzbPassCB->setChecked(true);
        onGenNzbPassword();
    }
    if (_ngPost->_doPar2)
        _ui->par2CB->setChecked(true);
    if (_ngPost->_keepRar)
        _ui->keepRarCB->setChecked(true);

    QString fixedPass = _hmi->fixedArchivePassword();
    if (!fixedPass.isEmpty())
        _ui->nzbPassEdit->setText(fixedPass);

    _profileBar->applyRecommendedIfNew(); // first start of a new installation
}

void PostingWidget::genNameAndPassword(bool genName, bool genPass, bool doPar2, bool useRarMax)
{
    _ui->compressCB->setChecked(_ngPost->_doCompress);
    if (genName)
        onGenCompressName();
    if (genPass && _ngPost->_rarPassFixed.isEmpty())
    {
        _ui->nzbPassCB->setChecked(true);        
        onGenNzbPassword();
    }
    _ui->rarMaxCB->setChecked(useRarMax);

    if (doPar2)
        _ui->par2CB->setChecked(true);

    _ui->keepRarCB->setChecked(_ngPost->_keepRar);
}



void PostingWidget::udatePostingParams()
{
    if (!_ui->nzbFileEdit->text().isEmpty())
    {
        QFileInfo nzb(_ui->nzbFileEdit->text());
        if (!nzb.absolutePath().isEmpty())
            _ngPost->_nzbPath = nzb.absolutePath();
        _ngPost->setNzbName(nzb);
    }

    // fetch compression settings
    _ngPost->_tmpPath    = _ui->compressPathEdit->text();
    _ngPost->_doCompress = _ui->compressCB->isChecked();
    _ngPost->_rarPath    = _ui->rarEdit->text();
    _ngPost->_rarName    = _ui->compressNameEdit->text();
    if (_ui->nzbPassCB->isChecked())
        _ngPost->_rarPass = _ui->nzbPassEdit->text().toLocal8Bit();
    else
        _ngPost->_rarPass = QString();
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
    _ngPost->_useRarMax = _ui->rarMaxCB->isChecked();

    // fetch par2 settings
    _ngPost->_doPar2  = _ui->par2CB->isChecked();
    if (_ngPost->_par2Args.isEmpty())
        _ngPost->_par2Pct = static_cast<uint>(_ui->redundancySB->value());

    _ngPost->_keepRar = _ui->keepRarCB->isChecked();
}

void PostingWidget::retranslate()
{
    _ui->retranslateUi(this);
    _retranslateLayout();
    if (_profileBar)
        _profileBar->retranslate();
    if (_nfoCard)
        _nfoCard->retranslate();
    _ui->filesList->setEmptyState(tr("Drag files or folders here, or choose them with the buttons below."),
                                  tr("Select Files"), tr("Select Folder"));
    _ui->rarMaxCB->setToolTip(tr("limit the number of archive volume to %1 (cf config RAR_MAX)").arg(_ngPost->_rarMax));
    _ui->redundancySB->setToolTip(tr("Using PAR2_ARGS from config file: %1").arg(_ngPost->_par2Args));
    _ui->filesList->setToolTip(QString("%1<ul><li>%2</li><li>%3</li><li>%4</li></ul>%5").arg(
                                   tr("You can add files or folder by:")).arg(
                                   tr("Drag & Drop files/folders")).arg(
                                   tr("Right Click to add Files")).arg(
                                   tr("Click on Select Files/Folder buttons")).arg(
                                   tr("Bare in mind you can select items in the list and press DEL to remove them")));
}

void PostingWidget::setNzbPassword(const QString &pass)
{
    _ui->nzbPassCB->setChecked(true);
    _ui->nzbPassEdit->setText(pass);
}

void PostingWidget::setPackingAuto(bool enabled, const QStringList &keys)
{
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
    _ui->nzbPassCB->setChecked(genPass);
    _ui->compressCB->setChecked(compress);
    _ui->par2CB->setChecked(doPar2);


    if (compress)
    {
        if (_ui->nzbPassEdit->text().isEmpty())
        {
            if (_hmi->useFixedPassword())
                _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
            else if (genPass)
                onGenNzbPassword();
        }
        if (genName && _ui->compressNameEdit->text().isEmpty())
            onGenCompressName();
    }
}

void PostingWidget::addPath(const QString &path, int currentNbFiles, int isDir)
{
    if (_ui->filesList->addPathIfNotInList(path, currentNbFiles, isDir))
    {
        QFileInfo fileInfo(path);
        if (_ui->nzbFileEdit->text().isEmpty())
        {
            _ngPost->setNzbName(fileInfo);
            _ui->nzbFileEdit->setText(QString("%1.nzb").arg(_ngPost->nzbPath()));
        }
        if (_ui->compressNameEdit->text().isEmpty())
            _ui->compressNameEdit->setText(_ngPost->_nzbName);
    }
}

bool PostingWidget::_fileAlreadyInList(const QString &fileName, int currentNbFiles) const
{
    for (int i = 0 ; i < currentNbFiles ; ++i)
    {
        if (_ui->filesList->item(i)->text() == fileName)
            return true;
    }
    return false;
}

void PostingWidget::setIDLE()
{
    _ui->postButton->setText(tr("Post Files"));
    _state = STATE::IDLE;
    _updateRepostButton();
    _updateReleaseInfo();
}

void PostingWidget::setPosting()
{
    _hmi->updateJobTab(this, _hmi->sPostingColor, QIcon(_hmi->sPostingIcon), _postingJob->nzbName());
    _ui->postButton->setText(tr("Stop Posting"));
    _state = STATE::POSTING;
    _updateReleaseInfo();
}


//========================================================================
// ngPost Deluxe layout: file list, then the NZB and Archive cards,
// advanced options (folders, rar, volumes) hidden by default, Post button
//========================================================================
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QCheckBox>
#include <QLabel>

bool PostingWidget::sShowAdvanced = false;

static QFrame *newCard(QLabel *&title, QGridLayout *&grid)
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
    return card;
}

static QHBoxLayout *row(std::initializer_list<QWidget*> widgets, bool stretch = false)
{
    QHBoxLayout *h = new QHBoxLayout();
    h->setSpacing(8);
    for (QWidget *w : widgets)
        h->addWidget(w);
    if (stretch)
        h->addStretch();
    return h;
}

void PostingWidget::_buildLayout()
{
    QWidget *page = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(page);
    root->setContentsMargins(2, 12, 2, 2);
    root->setSpacing(12);

    // files: title + small tool bar, then the list (with its empty screen)
    QHBoxLayout *head = new QHBoxLayout();
    head->setSpacing(8);
    _filesTitle = new QLabel(page);
    _filesTitle->setObjectName("cardTitle");
    head->addWidget(_filesTitle);
    head->addStretch();
    head->addWidget(_ui->selectFilesButton);
    head->addWidget(_ui->selectFolderButton);
    head->addWidget(_ui->clearFilesButton);
    root->addLayout(head);
    root->addWidget(_ui->filesList, 1);

    // reminder: everything in the list is one release
    _releaseInfo = new QLabel(page);
    _releaseInfo->setObjectName("infoBanner");
    _releaseInfo->setWordWrap(true);
    _releaseInfo->setTextFormat(Qt::RichText);
    _releaseInfo->hide();
    root->addWidget(_releaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsInserted, this, &PostingWidget::_updateReleaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsRemoved,  this, &PostingWidget::_updateReleaseInfo);
    connect(_ui->filesList->model(), &QAbstractItemModel::modelReset,   this, &PostingWidget::_updateReleaseInfo);

    // profiles of options (shared with Auto Posting)
    _profileBar = new ProfileBar(_ngPost, page);
    _profileBar->setHandlers([this](){ return currentProfile(); }, [this](const PostProfile &p){ applyProfile(p); });
    root->addWidget(_profileBar);

    // NZB and Archive cards side by side
    QHBoxLayout *cards = new QHBoxLayout();
    cards->setSpacing(12);
    QGridLayout *g = nullptr;

    QFrame *nzb = newCard(_nzbTitle, g);
    g->addWidget(_ui->nzbFileLbl, 0, 0);
    g->addLayout(row({_ui->nzbFileEdit, _ui->nzbFileButton}), 0, 1);
    g->setColumnStretch(1, 1);
    _ui->nzbFileEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->nzbPassEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    static_cast<QVBoxLayout*>(nzb->layout())->addStretch(1); // the title and the file field stay at the top
    cards->addWidget(nzb, 1);

    QFrame *archive = newCard(_archiveTitle, g);
    g->addWidget(_ui->compressCB, 0, 0);
    g->addLayout(row({_ui->compressNameEdit, _ui->nameLengthSB, _ui->genCompressName}), 0, 1);
    g->addWidget(_ui->par2CB, 1, 0);
    g->addLayout(row({_ui->redundancyLbl, _ui->redundancySB, _ui->keepRarCB}, true), 1, 1);
    g->addWidget(_ui->nzbPassCB, 2, 0);
    g->addLayout(row({_ui->nzbPassEdit, _ui->passLengthSB, _ui->genPass}), 2, 1);
    g->setColumnStretch(1, 1);
    _ui->compressNameEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    cards->addWidget(archive, 1);
    root->addLayout(cards);

    // NFO file (shared with Auto Posting)
    _nfoCard = new NfoCard(_ngPost, page);
    root->addWidget(_nfoCard);

    // advanced options
    _advancedCard = newCard(_advancedTitle, g);
    g->addWidget(_ui->compressPathLbl, 0, 0);
    g->addLayout(row({_ui->compressPathEdit, _ui->compressPathButton}), 0, 1);
    g->addWidget(_ui->rarLbl, 1, 0);
    g->addLayout(row({_ui->rarEdit, _ui->rarPathButton}), 1, 1);
    g->addWidget(_ui->rarSizeLbl, 2, 0);
    g->addLayout(row({_ui->rarSizeEdit, _ui->rarMaxCB}, true), 2, 1);
    g->setColumnStretch(1, 1);
    _ui->compressPathEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarEdit->setMaximumWidth(QWIDGETSIZE_MAX);
    _ui->rarSizeEdit->setMaximumWidth(100);
    root->addWidget(_advancedCard);

    // summary of the post (size, estimated upload time)
    _summary = new QLabel(page);
    _summary->setObjectName("hint");
    _summary->hide();
    root->addWidget(_summary);
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsInserted, this, [this](){ _updateSummary(true); });
    connect(_ui->filesList->model(), &QAbstractItemModel::rowsRemoved,  this, [this](){ _updateSummary(true); });
    connect(_ui->filesList->model(), &QAbstractItemModel::modelReset,   this, [this](){ _updateSummary(true); });
    connect(_ui->compressCB, &QAbstractButton::toggled, this, [this](){ _updateSummary(false); });
    connect(_ui->par2CB,     &QAbstractButton::toggled, this, [this](){ _updateSummary(false); });
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](){ _updateSummary(false); });

    // bottom: advanced toggle on the left, Post button in the middle of the page
    QGridLayout *bottom = new QGridLayout();
    _advancedToggle = new QPushButton(page);
    _advancedToggle->setObjectName("linkButton");
    _advancedToggle->setCursor(Qt::PointingHandCursor);
    connect(_advancedToggle, &QAbstractButton::clicked, this, [this](){ _showAdvanced(!sShowAdvanced); });
    bottom->addWidget(_advancedToggle, 0, 0, Qt::AlignLeft | Qt::AlignVCenter);
    _ui->postButton->setObjectName("accentButton");
    _ui->postButton->setIcon(QIcon());
    _ui->postButton->setMinimumSize(170, 38);
    bottom->addWidget(_ui->postButton, 0, 1, Qt::AlignCenter);
    _repostButton = new QPushButton(page);
    _repostButton->setMinimumHeight(38);
    _repostButton->setCursor(Qt::PointingHandCursor);
    _repostButton->setToolTip(tr("Some Articles couldn't be posted. This posts only those ones again and fixes the nzb file."));
    _repostButton->hide();
    connect(_repostButton, &QAbstractButton::clicked, this, &PostingWidget::onRepostMissing);
    bottom->addWidget(_repostButton, 0, 2, Qt::AlignRight | Qt::AlignVCenter);
    bottom->setHorizontalSpacing(0); // nothing in the right column: no spacing, so the button is really in the middle
    bottom->setColumnStretch(0, 1);
    bottom->setColumnStretch(1, 0);
    bottom->setColumnStretch(2, 1);
    root->addLayout(bottom);

    _ui->aboutButton->hide(); // About is in the side bar

    // the .ui limited the width of some labels and check boxes (they were on one long line)
    for (QWidget *w : page->findChildren<QWidget*>())
        if (qobject_cast<QLabel*>(w) || qobject_cast<QCheckBox*>(w))
            w->setMaximumWidth(QWIDGETSIZE_MAX);

    // the .ui limited the width of some labels and check boxes (they were on one long line)
    for (QWidget *w : page->findChildren<QWidget*>())
        if (qobject_cast<QLabel*>(w) || qobject_cast<QCheckBox*>(w))
            w->setMaximumWidth(QWIDGETSIZE_MAX);

    delete layout(); // the old layout of the .ui (the widgets have been moved in the new one)
    QVBoxLayout *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addWidget(page);

    // a change made by hand: the profile is now "Custom"
    for (QAbstractButton *b : {static_cast<QAbstractButton*>(_ui->compressCB), static_cast<QAbstractButton*>(_ui->par2CB),
                               static_cast<QAbstractButton*>(_ui->keepRarCB), static_cast<QAbstractButton*>(_ui->nzbPassCB)})
        connect(b, &QAbstractButton::clicked, _profileBar, &ProfileBar::markCustom);
    connect(_ui->redundancySB, QOverload<int>::of(&QSpinBox::valueChanged), _profileBar, &ProfileBar::markCustom);

    _showAdvanced(sShowAdvanced);
    _retranslateLayout();
}

PostProfile PostingWidget::currentProfile() const
{
    PostProfile p;
    p.compress = _ui->compressCB->isChecked();
    p.genName  = p.compress && !_ui->compressNameEdit->text().isEmpty();
    p.genPass  = _ui->nzbPassCB->isChecked();
    p.par2     = _ui->par2CB->isChecked();
    p.par2Pct  = static_cast<uint>(_ui->redundancySB->value());
    p.keepRar  = _ui->keepRarCB->isChecked();
    return p;
}

void PostingWidget::applyProfile(const PostProfile &p)
{
    _ui->compressCB->setChecked(p.compress);
    _ui->par2CB->setChecked(p.par2);
    if (_ngPost->_par2Args.isEmpty() && p.par2Pct > 0)
        _ui->redundancySB->setValue(static_cast<int>(p.par2Pct));
    _ui->keepRarCB->setChecked(p.keepRar);
    _ui->nzbPassCB->setChecked(p.genPass);
    if (p.compress)
    {
        if (p.genPass && _ui->nzbPassEdit->text().isEmpty())
        {
            if (_hmi->useFixedPassword())
                _ui->nzbPassEdit->setText(_ngPost->_rarPassFixed);
            else
                onGenNzbPassword();
        }
        if (p.genName && _ui->compressNameEdit->text().isEmpty())
            onGenCompressName();
    }
}

void PostingWidget::_showAdvanced(bool show)
{
    sShowAdvanced = show;
    _advancedCard->setVisible(show);
    _retranslateLayout();
}

void PostingWidget::_updateReleaseInfo()
{
    if (!_releaseInfo)
        return;
    // only while the post is prepared: during and after the upload the list shows the volumes of the archive
    _releaseInfo->setVisible(_ui->filesList->count() > 1 && _state == STATE::IDLE && !_postingFinished);
}

void PostingWidget::_retranslateLayout()
{
    if (!_filesTitle)
        return;
    _releaseInfo->setText(tr("<b>One release:</b> all the files and folders of this list are posted together, "
                             "with a single NZB file and, if Compress is on, a single archive. "
                             "To post separate releases, use one tab per release (<i>%1</i>) "
                             "or the <i>%2</i> tab, which makes one post for each file or folder.").arg(
                              QCoreApplication::translate("MainWindow", "New"), _ngPost->folderMonitoringName()));
    _updateReleaseInfo();
    _filesTitle->setText(tr("Files to post"));
    _nzbTitle->setText(tr("NZB"));
    _archiveTitle->setText(tr("Archive"));
    _advancedTitle->setText(tr("Advanced options"));
    _advancedToggle->setText(sShowAdvanced ? tr("▾ Hide advanced options") : tr("▸ Advanced options (temporary folder, rar / 7z, volumes)"));

    _ui->nzbFileLbl->setText(tr("File"));
    _ui->nzbPassCB->setText(tr("Password"));
    _ui->compressCB->setText(tr("Compress"));
    _ui->par2CB->setText(tr("par2"));
    _ui->redundancyLbl->setText(tr("redundancy (%)"));
    _ui->keepRarCB->setText(tr("keep the archives"));
    _ui->compressPathLbl->setText(tr("Temporary folder"));
    _ui->rarLbl->setText(tr("rar / 7z program"));
    _ui->rarSizeLbl->setText(tr("Volume size (MB)"));
    _ui->rarMaxCB->setText(tr("limit the number of volumes"));
    _ui->clearFilesButton->setText(tr("Remove All"));
    _ui->postButton->setText(isPosting() ? _ui->postButton->text() : tr("Post Files"));
    _repostButton->setToolTip(tr("Some Articles couldn't be posted. This posts only those ones again and fixes the nzb file."));
    _updateRepostButton();
}
