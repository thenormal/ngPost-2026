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

#include "PostingJob.h"
#include <QSaveFile>
#include <QJsonArray>
#include <QDateTime>
#include "PostQueue.h"
#include "NgPost.h"
#include "NntpConnection.h"
#include "nntp/NntpServerParams.h"
#include "utils/Diagnostics.h"
#include "utils/LatencyStats.h"
#include "utils/NfoWriter.h"
#include "utils/MediaInfoReader.h"
#include <QDirIterator>
#include "nntp/NntpFile.h"
#include "nntp/NntpArticle.h"
#ifdef __USE_HMI__
  #include "hmi/PostingWidget.h"
#endif
#include <cmath>
#include <QDebug>
#include <QProcess>
#include <QThread>
#include <QMutex>
#include <QCoreApplication>
#include <QDir>

//! how long we wait for the queued 'file posted' signals once all the connections are closed
static const int sLastFilesTimeoutMs = 10000;

//! Articles that failed on every connection / server are kept (in memory) for a second round, up to this number
static const int sMaxHeldArticles = 200;

//! pause before the second round (30 s; the tests shorten it with NGPOST_SECOND_ROUND_DELAY_MS)
static int secondRoundDelayMs()
{
    bool ok = false;
    int delay = qEnvironmentVariableIntValue("NGPOST_SECOND_ROUND_DELAY_MS", &ok);
    return (ok && delay >= 0) ? delay : 30000;
}

PostingJob::PostingJob(NgPost *ngPost,
                       const QString &nzbFilePath,
                       const QFileInfoList &files,
                       PostingWidget *postWidget,
                       const QList<QString> &grpList,
                       const std::string    &from,
                       bool obfuscateArticles,
                       bool obfuscateFileName,
                       const QString &tmpPath,
                       const QString &rarPath,
                       const QString &rarArgs,
                       uint rarSize,
                       bool useRarMax,
                       uint par2Pct,
                       bool doCompress,
                       bool doPar2,
                       const QString &rarName,
                       const QString &rarPass,
                       bool keepRar,
                       bool delFilesAfterPost,
                       bool overwriteNzb,
                       QObject *parent) :
    QObject (parent),
    _ngPost(ngPost), _files(files), _postWidget(postWidget),

    _extProc(nullptr), _compressDir(nullptr), _limitProcDisplay(false), _nbProcDisp(42),

    _tmpPath(tmpPath), _rarPath(rarPath), _rarArgs(rarArgs), _rarSize(rarSize), _useRarMax(useRarMax), _par2Pct(par2Pct),
    _doCompress(doCompress), _doPar2(doPar2), _rarName(rarName), _rarPass(rarPass), _keepRar(keepRar),
    _splitArchive(false),

    _nntpConnections(), _closedConnections(),

    _nzbName(QFileInfo(nzbFilePath).fileName()),
    _filesToUpload(), _filesInProgress(), _filesFailed(),
    _nbFiles(0), _nbPosted(0),


    _nzbFilePath(nzbFilePath), _nzb(nullptr), _nzbStream(),
    _nntpFile(nullptr), _file(nullptr), _part(0),
    _timeStart(), _totalSize(0), _pauseTimer(), _pauseDuration(0),

    _nbConnections(0), _nbThreads(_ngPost->_nbThreads),
    _nbArticlesUploaded(0), _nbArticlesFailed(0),
    _uploadedSize(0), _nbArticlesTotal(0),
    _stopPosting(0x0), _noMoreFiles(0x0),
    _postStarted(false), _packed(false), _postFinished(false),
    _obfuscateArticles(obfuscateArticles), _obfuscateFileName(obfuscateFileName),
    _delFilesAfterPost(delFilesAfterPost ? 0x1 : 0x0),
    _originalFiles(!postWidget || delFilesAfterPost  || obfuscateFileName ? files : QFileInfoList()),
    _nbInputFiles(files.size()),
    _secureDiskAccess(), _posters(),
    _overwriteNzb(overwriteNzb),
    _grpList(grpList), _from(from),
    _use7z(false), _isPaused(false), _resumeTimer(), _isActiveJob(false),
    _inputFiles(files)
  #ifdef __COMPUTE_IMMEDIATE_SPEED__
    ,_immediateSize(0), _immediateSpeedTimer(), _immediateSpeed("0 B/s"), _useHMI(_ngPost->useHMI())
  #endif
{
#ifdef __DEBUG__
    qDebug() << "[PostingJob] >>>> Construct " << this;
#endif
    connect(this, &PostingJob::startPosting,     this,    &PostingJob::onStartPosting,   Qt::QueuedConnection);
    connect(this, &PostingJob::stopPosting,      this,    &PostingJob::onStopPosting,    Qt::QueuedConnection);
    connect(this, &PostingJob::postingStarted,   _ngPost, &NgPost::onPostingJobStarted,  Qt::QueuedConnection);
    connect(this, &PostingJob::packingDone,      _ngPost, &NgPost::onPackingDone,        Qt::QueuedConnection);
    connect(this, &PostingJob::postingFinished,  _ngPost, &NgPost::onPostingJobFinished, Qt::QueuedConnection);
    connect(this, &PostingJob::noMoreConnection, _ngPost, &NgPost::onPostingJobFinished, Qt::QueuedConnection);

//    connect(this, &PostingJob::scheduleNextArticle, this, &PostingJob::onPrepareNextArticle, Qt::QueuedConnection);

#ifdef __USE_HMI__
    if (_postWidget)
    {
        connect(this, &PostingJob::filePosted,       _postWidget, &PostingWidget::onFilePosted,        Qt::QueuedConnection);
        connect(this, &PostingJob::archiveFileNames, _postWidget, &PostingWidget::onArchiveFileNames,  Qt::QueuedConnection);
        connect(this, &PostingJob::articlesNumber,   _postWidget, &PostingWidget::onArticlesNumber,    Qt::QueuedConnection);
        connect(this, &PostingJob::postingFinished,  _postWidget, &PostingWidget::onPostingJobDone,    Qt::QueuedConnection);
        connect(this, &PostingJob::noMoreConnection, _postWidget, &PostingWidget::onPostingJobDone,    Qt::QueuedConnection);
    }
#endif

    connect(&_resumeTimer, &QTimer::timeout, this, &PostingJob::onResumeTriggered);
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    if (_useHMI)
        connect(&_immediateSpeedTimer, &QTimer::timeout, this, &PostingJob::onImmediateSpeedComputation, Qt::QueuedConnection);
#endif

    if (ngPost->debugMode())
        _log(NntpConnection::sslSupportInfo());
}

PostingJob::~PostingJob()
{
    delete _nfoDir;

#ifdef __DEBUG__
    _logAs(NgPost::LOG_DEBUG, "Destructing PostingJob");
    qDebug() << "[PostingJob] <<<< Destruction " << this;
#endif

    if (_ngPost->debugMode())
        _logAs(NgPost::LOG_DEBUG, "Deleting PostingJob");

    if (_compressDir)
    {
        if (hasPostFinishedSuccessfully())
            _cleanCompressDir();
        delete _compressDir;
        _compressDir = nullptr;
    }

    if (_extProc)
        _cleanExtProc();

    qDeleteAll(_filesFailed);
    qDeleteAll(_filesInProgress);
    qDeleteAll(_filesToUpload);
    qDeleteAll(_nntpConnections);
    qDeleteAll(_closedConnections);
    qDeleteAll(_posters);

    if (_nzb)
        delete _nzb;
    if (_file)
        delete _file;
}

void PostingJob::pause()
{
    _log("Pause posting...");
    for (NntpConnection *con : _nntpConnections)
        emit con->killConnection();

    _isPaused = true;
    _pauseTimer.start();
}

void PostingJob::resume()
{
    _log("Resume posting...");
    for (NntpConnection *con : _nntpConnections)
        emit con->startConnection();

    _isPaused = false;
    _pauseDuration += _pauseTimer.elapsed();
}

QString PostingJob::sslSupportInfo()
{
    return NntpConnection::sslSupportInfo();
}

bool PostingJob::supportsSsl()
{
    return NntpConnection::supportsSsl();
}

void PostingJob::onResumeTriggered()
{
    if (_isPaused)
    {
        _log(tr("Try to resume posting"));
        _nntpConnections.swap(_closedConnections);
        _ngPost->resume();
    }
}

#ifdef __COMPUTE_IMMEDIATE_SPEED__
void PostingJob::onImmediateSpeedComputation()
{
    QString power = " ";
    int immediateSpeedDurationMs = NgPost::immediateSpeedDurationMs();
    double bandwidth = 1000. * _immediateSize / immediateSpeedDurationMs;
    if (bandwidth > 1024)
    {
        bandwidth /= 1024;
        power = "k";
    }
    if (bandwidth > 1024)
    {
        bandwidth /= 1024;
        power = "M";
    }

    _immediateSpeed = QString("%1 %2B/s").arg(bandwidth, 6, 'f', 2).arg(power);
    _immediateSize  = 0;
    _immediateSpeedTimer.start(immediateSpeedDurationMs);
}
#endif

void PostingJob::onStartPosting(bool isActiveJob)
{
    _isActiveJob = isActiveJob;
    if (!_jobTimer.isValid())
        _jobTimer.start();
#ifdef __DEBUG__
qDebug() << "[MB_TRACE][Issue#82][PostingJob::onStartPosting] job: " << this
         << ", file: " << nzbName() << " (isActive: " << isActiveJob << ")";
#endif
#ifdef __USE_HMI__
    if (_postWidget)
    {
        if (_isRepost)
            _log(tr("<h3>Post again the missing Articles #%1: %2</h3>").arg(_postWidget->jobNumber()).arg(_nzbName));
        else
            _log(tr("<h3>Start Post #%1: %2</h3>").arg(_postWidget->jobNumber()).arg(_nzbName));
    }
    else
#endif
        _logAs(NgPost::LOG_PHASE, QString("\n\n[%1] %2: %3").arg(timestamp()).arg(_isRepost ? tr("Post again the missing Articles") : tr("Start posting")).arg(_nzbName));

    _readMediaInfo();

    if (_doCompress)
    {
#ifdef __USE_TMP_RAM__
        if (_ngPost->useTmpRam())
        {
            qint64 sourceSize = 0;
            for (const QFileInfo &fi : _files)
                sourceSize += NgPost::recursiveSize(fi);

            double sourceSizeWithRatio = _ngPost->ramRatio() * sourceSize,
                    availableSize = static_cast<double>(_ngPost->ramAvailable());
            if (sourceSizeWithRatio  <  availableSize)
            {
                _tmpPath = _ngPost->_ramPath;
                _log(tr("Using TMP_RAM path as temporary folder. Post size: %1").arg(
                         humanSize(static_cast<double>(sourceSize))));
            }
            else
            {
                _error(tr("Couldn't use TMP_RAM as there is not enough space: %1 available for a Post with ratio of %2").arg(
                           humanSize(availableSize)).arg(humanSize(sourceSizeWithRatio)));
            }
        }
#endif


#ifdef __DEBUG__
        _logAs(NgPost::LOG_DEBUG, "[PostingJob::onStartPosting] Starting compression...");
#endif
        if (!startCompressFiles(_rarPath, _tmpPath, _rarName, _rarPass, _rarSize))
            emit postingFinished();
    }
    else if (_doPar2)
    {
#ifdef __USE_TMP_RAM__
        if (_ngPost->useTmpRam())
        {
            qint64 sourceSize = 0;
            for (const QFileInfo &fi : _files)
                sourceSize += NgPost::recursiveSize(fi);

            double par2Size = (_ngPost->ramRatio() - 1) * sourceSize,
                    availableSize = static_cast<double>(_ngPost->ramAvailable());
            if (par2Size  <  availableSize)
            {
                _tmpPath = _ngPost->_ramPath;
                _log(tr("Using TMP_RAM path as temporary folder for par2. Post size: %1").arg(
                         humanSize(static_cast<double>(sourceSize))));
            }
            else
            {
                _error(tr("Couldn't use TMP_RAM as there is not enough space: %1 available for a par2 volume using TMP_RAM_RATIO of %2").arg(
                           humanSize(availableSize)).arg(humanSize(par2Size)));
            }
        }
#endif
        if (!startGenPar2(_tmpPath, _rarName, _par2Pct))
            emit postingFinished();
    }
    else
    {
        _packed = true;
        _postFiles();
    }

}


#include "Poster.h"
void PostingJob::_postFiles()
{    
    _postStarted = true;
    _phase = Phase::UPLOADING;
    _diagRegisterPost();
    Diagnostics::instance().event("UPLOAD", QString("start: %1 file(s) (nzb: %2)").arg(_files.size()).arg(_nzbFilePath));

#ifdef __USE_HMI__
    if (_postWidget) // in case we were in Pending mode
        _postWidget->setPosting();
#endif

    if (_doCompress)
    {
        QStringList archiveNames;
        _files.clear();
        for (const QFileInfo & file : _compressDir->entryInfoList(QDir::Files, QDir::Name))
        {
            _files << file;
            archiveNames << file.absoluteFilePath();
            if (_ngPost->debugMode())
                _ngPost->_log(QString("  - %1").arg(file.fileName()));
        }
        emit archiveFileNames(archiveNames);
    }
    else if (_doPar2)
    {
        QStringList archiveNames;
        for (const QFileInfo & file : _files)
            archiveNames << file.absoluteFilePath();

        for (const QFileInfo & file : _compressDir->entryInfoList(QDir::Files, QDir::Name))
        {
            _files << file;
            archiveNames << file.absoluteFilePath();
            if (_ngPost->debugMode())
                _ngPost->_log(QString("  - %1").arg(file.fileName()));
        }
        emit archiveFileNames(archiveNames);
    }
    if (!_isRepost && _ngPost->_genNfo)
        _prepareNfo();
    _initPosting();
    if (!_isRepost)
    {
        _saveNfoNextToNzb();
        _nzbPathFinal = true;          // _initPosting may have changed it (no overwrite: name_1.nzb)
        _ngPost->queueSaveJob(this);   // so a restart overwrites this nzb and not another one
    }

    if (_nbThreads > QThread::idealThreadCount())
        _nbThreads = QThread::idealThreadCount();

    int  nbPosters = _nbThreads/2, nbCon = _createNntpConnections();
    if (nbPosters < 1)
        nbPosters = 1;
    if (!nbCon)
    {
        _error(tr("Error: there are no NntpConnection..."));
        emit postingFinished();
        return;
    }

    if (_isRepost)
    {
        // nothing to write: the Message-IDs are fixed in the existing nzb when everything is posted
    }
    else if (!_nzb->open(QIODevice::WriteOnly))
    {
        _error(tr("Error: Can't create nzb output file: %1").arg(_nzbFilePath));
        emit postingFinished();
        return ;
    }
    else
    {
        QString tab = _ngPost->space();
        _nzbStream.setDevice(_nzb);
        _nzbStream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   << "<!DOCTYPE nzb PUBLIC \"-//newzBin//DTD NZB 1.1//EN\" \"http://www.newzbin.com/DTD/nzb/nzb-1.1.dtd\">\n"
                   << "<nzb xmlns=\"http://www.newzbin.com/DTD/2003/nzb\">\n";

        if (!_rarPass.isEmpty() || _ngPost->_meta.size())
        {
            _nzbStream << tab << "<head>\n";
            for (auto itMeta = _ngPost->_meta.cbegin(); itMeta != _ngPost->_meta.cend() ; ++itMeta)
                _nzbStream << tab << tab << "<meta type=\"" << itMeta.key() << "\">" << itMeta.value() << "</meta>\n";
            if (!_rarPass.isEmpty())
                _nzbStream << tab << tab << "<meta type=\"password\">" << _rarPass << "</meta>\n";
            _nzbStream << tab << "</head>\n\n";
        }
        _nzbStream << MB_FLUSH;
    }



    _timeStart.start();

//    QMutexLocker lock(&_secureArticles); // start the connections but they must wait _prepareArticles

    if (nbPosters > nbCon)
        nbPosters = nbCon; // we can't have more thread than available connections


    _posters.reserve(nbPosters);
    int nbConPerPoster = static_cast<int>(std::floor(nbCon / nbPosters));
    int nbExtraCon     = nbCon - nbConPerPoster * nbPosters;
#ifdef __DEBUG__
    qDebug() << "[PostingJob::_postFiles] nbFiles: " << _filesToUpload.size()
             << ", nbPosters: " << nbPosters
             << ", nbCons: " << nbCon
             << " => nbCon per Poster: " << nbConPerPoster
             << " (nbExtraCon: " << nbExtraCon << ")";
#endif


    int conIdx = 0;
    for (ushort posterIdx = 0; posterIdx < nbPosters ; ++posterIdx)
    {
        Poster *poster = new Poster(this, posterIdx);
        _posters.append(poster);
        poster->lockQueue(); // lock queue so the connection will wait before starting building Articles

        for (int i = 0 ; i < nbConPerPoster ; ++i)
            poster->addConnection(_nntpConnections.at(conIdx++));

        if (nbExtraCon-- > 0)
            poster->addConnection(_nntpConnections.at(conIdx++));

        poster->startThreads();
    }

    // Prepare 2 Articles for each connections
    _preparePostersArticles();


#ifdef __COMPUTE_IMMEDIATE_SPEED__
    _immediateSpeedTimer.start(NgPost::immediateSpeedDurationMs());
#endif

    for (Poster *poster : _posters)
        poster->unlockQueue();

    emit postingStarted();
}



void PostingJob::onStopPosting()
{
    if (_extProc)
    {
        _log(tr("killing external process..."));
        _extProc->terminate();
        _extProc->waitForFinished();
    }
    else
    {
        _finishPosting();
        emit postingFinished();
    }
}


void PostingJob::onDisconnectedConnection(NntpConnection *con)
{
    if (MB_LoadAtomic(_stopPosting))
        return; // we're destructing all the connections

    if (!con->hasNoMoreFiles())
        _error(tr("Error: disconnected connection: #%1\n").arg(con->getId()));

    if (_nntpConnections.removeOne(con))
    {
        con->resetErrorCount(); // In case we will resume if we loose all
        _closedConnections.append(con);

        // a Poster that lost all its connections gives its prepared Articles to the other ones
        Poster *poster = con->poster();
        if (poster && !con->hasNoMoreFiles())
        {
            bool posterHasConnection = false;
            for (NntpConnection *other : _nntpConnections)
            {
                if (other->poster() == poster)
                {
                    posterHasConnection = true;
                    break;
                }
            }
            if (!posterHasConnection)
                poster->moveArticlesToRecovery();
        }

        if (_nntpConnections.isEmpty())
        {
            if (con->hasNoMoreFiles())
            {
                // some Articles failed: post them again once (the connections are restarted after a pause)
                if (!_postFinished && _startSecondRoundIfNeeded())
                    return;

                if (!_postFinished && _filesToUpload.isEmpty() && _filesFailed.isEmpty() && _nbPosted < _nbFiles)
                {
                    // Every Article has been sent but the 'all articles posted' signals of the last files
                    // are still queued: they go through their NntpFile first, so they can arrive after this
                    // disconnection. Finishing now would close (and delete) the nzb before those files are
                    // written in it: they would be missing from the nzb, or ngPost would crash when they arrive.
                    // onNntpFilePosted finishes the job when the last one arrives.
                    // Safety net: if they never come, we close the job as before.
                    _waitingLastFiles = true;
                    QTimer::singleShot(sLastFilesTimeoutMs, this, [this]() {
                        if (!_waitingLastFiles || MB_LoadAtomic(_stopPosting))
                            return;
                        _waitingLastFiles = false;
                        _error(tr("Error: the last files were never confirmed as posted, closing the Post"));
                        _keepInQueue = true;
                        _finishPosting();
                        if (!_postFinished)
                            emit noMoreConnection();
                    });
                }
                else
                {
                    _finishPosting();
                    if (!_postFinished)
                        emit noMoreConnection();
                }
            }
            else
            {
                // every connection was refused by the server because of the user / password:
                // waiting and retrying would loop for ever, so we stop here
                bool allRefused = !_closedConnections.isEmpty();
                for (NntpConnection *closed : _closedConnections)
                    allRefused = allRefused && closed->hasAuthFailed();
                if (allRefused)
                {
                    _error(tr("Authentication failed on all the connections: the server refused the user name or password. Post stopped."));
                    _finishPosting();
                    if (!_postFinished)
                        emit noMoreConnection();
                    return;
                }

                _error(tr("we lost all the connections..."));
                if (_ngPost->_tryResumePostWhenConnectionLost)
                {
                    int sleepDurationInSec = _ngPost->waitDurationBeforeAutoResume();
                    _logAs(NgPost::LOG_WARN, tr("Sleep for %1 sec before trying to reconnect").arg(sleepDurationInSec));
                    _ngPost->pause();
                    _resumeTimer.start(sleepDurationInSec*1000);
                }
                else
                {
                    _keepInQueue = true; // the network may be back at the next start
                    _finishPosting();
                    if (!_postFinished)
                        emit noMoreConnection();
                }
            }
        }
    }
}

void PostingJob::recoverArticle(NntpArticle *article, const NntpServerParams *server, const QString &conPrefix)
{
    QMutexLocker lock(&_secureRecovery); // called from the connection threads
    article->addRelease(server);
    article->resetNbTrySending();
    article->genNewId();

    // with 1 server: 3 connections can try it, with N servers: at least one connection of each server
    const int maxReleases = qMax(2, _nbEnabledServers);
    if (article->nbReleases() <= maxReleases)
    {
        _recoveryArticles.enqueue(article);
        if (_ngPost->debugMode())
            _logAs(NgPost::LOG_DEBUG, tr("[%1] Article given to another connection: %2").arg(conPrefix).arg(article->str()));
        return;
    }

    // it failed everywhere: keep it for the second round (if not too many)
    if (!_secondRoundDone && !_heldOverflow)
    {
        if (_heldArticles.size() < sMaxHeldArticles)
        {
            _heldArticles.append(article);
            if (_ngPost->debugMode())
                _logAs(NgPost::LOG_DEBUG, tr("[%1] Article kept for a second attempt: %2").arg(conPrefix).arg(article->str()));
            return;
        }
        _heldOverflow = true;
        _error(tr("More than %1 Articles failed: there won't be a second attempt, they are counted as failed").arg(
                   sMaxHeldArticles));
        for (NntpArticle *held : _heldArticles)
            emit held->failed(held->size());
        _heldArticles.clear();
    }
    _error(tr("give up on Article: %1").arg(article->str()));
    emit article->failed(article->size());
}

NntpArticle *PostingJob::takeRecoveryArticle(const NntpServerParams *server, bool onlyIfNotFailedThere)
{
    QMutexLocker lock(&_secureRecovery); // called from the connection threads
    for (int i = 0 ; i < _recoveryArticles.size() ; ++i)
    {
        NntpArticle *article = _recoveryArticles.at(i);
        // never on a server where it failed while another server could still take it
        bool canTake = !article->hasFailedOn(server)
                || (!onlyIfNotFailedThere && article->nbFailedServers() >= _nbEnabledServers);
        if (canTake)
        {
            _recoveryArticles.removeAt(i);
            return article;
        }
    }
    return nullptr;
}

void PostingJob::addToRecovery(const QQueue<NntpArticle*> &articles)
{
    QMutexLocker lock(&_secureRecovery);
    for (NntpArticle *article : articles)
        _recoveryArticles.enqueue(article);
}

bool PostingJob::_startSecondRoundIfNeeded()
{
    int nbArticles = 0;
    {
        QMutexLocker lock(&_secureRecovery);
        if (_recoveryArticles.isEmpty() && _heldArticles.isEmpty())
            return false;

        if (_secondRoundDone || _heldOverflow)
        {
            lock.unlock();
            _failRecoveryArticles();
            return false;
        }

        _secondRoundDone = true;
        for (NntpArticle *held : _heldArticles)
            _recoveryArticles.enqueue(held);
        _heldArticles.clear();
        for (NntpArticle *article : _recoveryArticles)
        {
            article->resetReleases(); // the servers where it failed are remembered: the others go first
            article->resetNbTrySending();
            article->genNewId();
        }
        nbArticles = _recoveryArticles.size();
    }

    int delayMs = secondRoundDelayMs();
    _logAs(NgPost::LOG_WARN, tr("%1 Article(s) could not be posted: new attempt in %2 seconds").arg(nbArticles).arg(delayMs / 1000));
    QTimer::singleShot(delayMs, this, [this]() { _restartConnectionsForSecondRound(); });
    return true;
}

void PostingJob::_restartConnectionsForSecondRound()
{
    if (MB_LoadAtomic(_stopPosting) || _postFinished)
        return;

    _logAs(NgPost::LOG_WARN, tr("Second attempt for the Articles that could not be posted"));
    _nntpConnections.append(_closedConnections);
    _closedConnections.clear();
    if (_isPaused)
        return; // they will be started by the resume

    for (NntpConnection *con : _nntpConnections)
    {
        con->resetErrorCount();
        emit con->startConnection();
    }
}

void PostingJob::_failRecoveryArticles()
{
    QMutexLocker lock(&_secureRecovery);
    for (NntpArticle *article : _recoveryArticles)
    {
        _error(tr("give up on Article: %1").arg(article->str()));
        emit article->failed(article->size());
    }
    _recoveryArticles.clear();
    for (NntpArticle *article : _heldArticles)
    {
        _error(tr("give up on Article: %1").arg(article->str()));
        emit article->failed(article->size());
    }
    _heldArticles.clear();
}

void PostingJob::onNntpFileStartPosting()
{
    NntpFile *nntpFile = static_cast<NntpFile*>(sender());
    if (!_ngPost->useHMI())
        _log(QString("[%1][%2: %3] >>>>> %4").arg(timestamp()).arg(tr("avg. speed")).arg(avgSpeed()).arg(nntpFile->name()));
}

void PostingJob::onNntpFilePosted()
{
    NntpFile *nntpFile = static_cast<NntpFile*>(sender());
    if (MB_LoadAtomic(_stopPosting))
    {
        // the Post is already closed (stopped, or all the connections were lost): the nzb is closed,
        // _finishPosting has listed this file as not posted. Nothing to write, just free it.
        _filesInProgress.remove(nntpFile);
        emit nntpFile->scheduleDeletion();
        return;
    }
    if (nntpFile->isRepost())
    {
        for (const RepostPart &repostPart : nntpFile->repostParts())
            _totalSize += static_cast<quint64>(repostPart.bytes);
    }
    else
        _totalSize += static_cast<quint64>(nntpFile->fileSize());
    ++_nbPosted;
    if (_postWidget && !_isRepost)
        emit filePosted(nntpFile->path(), nntpFile->nbArticles(), nntpFile->nbFailedArticles());

    if (_ngPost->_dispFilesPosting && !_ngPost->useHMI())
        _log(QString("[%1][%2: %3] <<<<< %4").arg(timestamp()).arg(tr("avg. speed")).arg(avgSpeed()).arg(nntpFile->name()));

    if (_isRepost)
        _collectRepostIds(nntpFile);
    else
    {
        if (_nzb && _nzb->isOpen())
            nntpFile->writeToNZB(_nzbStream, QString::fromStdString(_from));
        else
            _error(tr("Error: the nzb is already closed, %1 is not in it").arg(nntpFile->name()));

        // keep what we need to post again its failed Articles (the Articles are deleted with the file)
        RepostFile repostFile;
        if (nntpFile->collectRepost(repostFile))
            _repostInfo.files.append(repostFile);
    }
    _filesInProgress.remove(nntpFile);
    emit nntpFile->scheduleDeletion();
    if (_nbPosted == _nbFiles)
    {
        _waitingLastFiles = false;
#ifdef __DEBUG__
        _log(QString("All files have been posted => closing Job (nb article uploaded: %1, failed: %2)").arg(
                 _nbArticlesUploaded).arg(_nbArticlesFailed));
#endif

        _postFinished = true;
        _finishPosting();

        emit postingFinished();
    }
}

void PostingJob::onNntpErrorReading()
{
    NntpFile *nntpFile = static_cast<NntpFile*>(sender());
    ++_nbPosted;
    if (_postWidget)
        emit filePosted(nntpFile->path(), nntpFile->nbArticles(), nntpFile->nbArticles());

    if (_ngPost->_dispFilesPosting && !_ngPost->useHMI())
        _log(tr("[avg. speed: %1] <<<<< %2").arg(avgSpeed()).arg(nntpFile->name()));

    _filesInProgress.remove(nntpFile);
    _filesFailed.insert(nntpFile);
    if (_nbPosted == _nbFiles)
    {
#ifdef __DEBUG__
        _log(QString("All files have been posted => closing Job (nb article uploaded: %1, failed: %2)").arg(
                 _nbArticlesUploaded).arg(_nbArticlesFailed));
#endif

        _postFinished = true;
        _finishPosting();

        emit postingFinished();
    }
}

void PostingJob::_log(const QString &aMsg, bool newline) const
{
    emit _ngPost->log(aMsg, newline);
}

void PostingJob::_logAs(int level, const QString &aMsg) const
{
    emit _ngPost->logLevel(aMsg, level);
}

void PostingJob::_error(const QString &error) const
{
    emit _ngPost->error(error);
}

int PostingJob::_createNntpConnections()
{
    _nbConnections    = 0;
    _nbEnabledServers = 0;
    for (NntpServerParams *srvParams : _ngPost->_nntpServers)
    {
        if (srvParams->enabled)
        {
            _nbConnections += srvParams->nbCons;
            ++_nbEnabledServers;
        }
    }
    if (_nbEnabledServers == 0)
        _nbEnabledServers = 1;

    _nntpConnections.reserve(_nbConnections);
    int conIdx = 0;
    for (NntpServerParams *srvParams : _ngPost->_nntpServers)
    {
        if (srvParams->enabled)
        {
            for (int k = 0 ; k < srvParams->nbCons ; ++k)
            {
                NntpConnection *nntpCon = new NntpConnection(_ngPost, ++conIdx, *srvParams);
                connect(nntpCon, &NntpConnection::log,   _ngPost, &NgPost::onLog, Qt::QueuedConnection);
                connect(nntpCon, &NntpConnection::error, _ngPost, &NgPost::onError, Qt::QueuedConnection);
                connect(nntpCon, &NntpConnection::errorConnecting, _ngPost, &NgPost::onErrorConnecting, Qt::QueuedConnection);
                connect(nntpCon, &NntpConnection::disconnected, this, &PostingJob::onDisconnectedConnection, Qt::QueuedConnection);
                _nntpConnections.append(nntpCon);
            }
        }
    }

    if (_ngPost->useHMI())
        _log(tr("Number of available Nntp Connections: %1").arg(_nbConnections));
    else
        _log(QString("[%1] %2: %3").arg(timestamp()).arg(tr("Number of available Nntp Connections")).arg(_nbConnections));

    return _nbConnections;
}

void PostingJob::_preparePostersArticles()
{
    if (_ngPost->debugFull())
        _logAs(NgPost::LOG_DEBUG, "PostingJob::_prepareArticles");

    for (int i = 0 ; i < _ngPost->sNbPreparedArticlePerConnection ; ++i)
    {
        if (MB_LoadAtomic(_noMoreFiles))
            break;
        for (Poster *poster : _posters)
        {
            if (!poster->prepareArticlesInAdvance())
                break;
        }
    }
}

void PostingJob::_delOriginalFiles()
{
    for (const QFileInfo &fi : _originalFiles)
    {
        QString path = fi.absoluteFilePath();
        _log(tr("Deleting posted %1: %2").arg(fi.isDir()?tr("folder"):tr("file")).arg(path));
        if (fi.isDir())
        {
            QDir dir(path);
            dir.removeRecursively();
        }
        else
            QFile::remove(path);
    }
}



NntpArticle *PostingJob::_readNextArticleIntoBufferPtr(const QString &threadName, char **bufferPtr)
{
//qDebug() << "[PostingJob::readNextArticleIntoBufferPtr] " << threadName;
    if (!_nntpFile)
    {
        _nntpFile = _getNextFile();
        if (!_nntpFile)
        {
            _noMoreFiles = 0x1;
#ifdef __DEBUG__
            _logAs(NgPost::LOG_DEBUG, tr("[%1] No more file to post...").arg(threadName));
#endif
            return nullptr;
        }
    }

    if (!_file)
    {
        _file = new QFile(_nntpFile->path());
        if (_file->open(QIODevice::ReadOnly))
        {
            if (_ngPost->debugFull())
                _logAs(NgPost::LOG_DEBUG, tr("[%1] starting processing file %2").arg(threadName).arg(_nntpFile->path()));
            _part = 0;
        }
        else
        {
            if (_ngPost->debugMode())
                _error(tr("[%1] Error: couldn't open file %2").arg(threadName).arg(_nntpFile->path()));
            else
                _error(tr("Error: couldn't open file %1").arg(_nntpFile->path()));
            emit _nntpFile->errorReadingFile();
            delete _file;
            _file = nullptr;
            _nntpFile = nullptr;
            return _readNextArticleIntoBufferPtr(threadName, bufferPtr); // Check if we have more files
        }
    }


    if (_file && _nntpFile->isRepost())
    {
        // posting again only the Articles that failed: we read exactly their part of the file
        const QVector<RepostPart> &parts = _nntpFile->repostParts();
        if (static_cast<int>(_part) < parts.size())
        {
            const RepostPart &repostPart = parts.at(static_cast<int>(_part++));
            qint64 bytesRead = 0;
            if (repostPart.bytes > 0 && repostPart.bytes <= static_cast<qint64>(_ngPost->articleSize())
                    && _file->seek(repostPart.pos))
                bytesRead = _file->read(*bufferPtr, repostPart.bytes);
            if (bytesRead != repostPart.bytes)
            {
                _error(tr("Error: couldn't read the Article %1 of %2 (has the file been modified or the Article size changed?)").arg(
                           repostPart.part).arg(_nntpFile->path()));
                emit _nntpFile->errorReadingFile();
                _file->close();
                delete _file;
                _file = nullptr;
                _nntpFile = nullptr;
                return _readNextArticleIntoBufferPtr(threadName, bufferPtr);
            }
            (*bufferPtr)[bytesRead] = '\0';
            return new NntpArticle(_nntpFile, repostPart.part, repostPart.pos, bytesRead,
                                   _obfuscateArticles ? nullptr : &_from, _obfuscateArticles);
        }
        _file->close();
        delete _file;
        _file = nullptr;
        _nntpFile = nullptr;
        return _readNextArticleIntoBufferPtr(threadName, bufferPtr); // all the Articles of this file are read
    }

    if (_file)
    {
        qint64 pos       = _file->pos();
        qint64 bytesRead = _file->read(*bufferPtr, _ngPost->articleSize());
        if (bytesRead > 0)
        {
            (*bufferPtr)[bytesRead] = '\0';
            if (_ngPost->debugFull())
                _logAs(NgPost::LOG_DEBUG, tr("[%1] we've read %2 bytes from %3 (=> new pos: %4)").arg(threadName).arg(bytesRead).arg(pos).arg(_file->pos()));
            ++_part;
            NntpArticle *article = new NntpArticle(_nntpFile, _part, pos, bytesRead,
                                                   _obfuscateArticles ? nullptr : &_from,
                                                   _obfuscateArticles);
            return article;
        }
        else
        {
            if (_ngPost->debugFull())
                _logAs(NgPost::LOG_DEBUG, tr("[%1] finished processing file %2").arg(threadName).arg(_nntpFile->path()));

            _file->close();
            delete _file;
            _file = nullptr;
            _nntpFile = nullptr;
        }
    }

    return _readNextArticleIntoBufferPtr(threadName, bufferPtr); // if we didn't have an Article, check next file
}


void PostingJob::_initPosting()
{
    // initialize buffer and nzb file
    if (!_overwriteNzb && !_isRepost)
    {
        QFileInfo fi(_nzbFilePath);
        if (fi.exists())
        {
            QString baseName     = fi.completeBaseName();
            ushort  nbDuplicates = 1;
            do {
                _nzbFilePath = QString("%1/%2_%3.nzb").arg(fi.absolutePath()).arg(baseName).arg(nbDuplicates++);
                fi = QFileInfo(_nzbFilePath);
            } while(fi.exists());

            _nzbFilePath = fi.absoluteFilePath();
        }
    }
    _nzb     = _isRepost ? nullptr : new QFile(_nzbFilePath);
    _nbFiles = static_cast<uint>(_files.size());

    int numPadding = 1;
    double padding = static_cast<double>(_nbFiles) / 10.;
    while (padding > 1.)
    {
        ++numPadding;
        padding /= 10.;
    }

    if (_isRepost)
    {
        // only the Articles that failed: same numbers, groups and total of Articles as the first time
        _filesToUpload.reserve(static_cast<int>(_nbFiles));
        for (const RepostFile &repostFile : _repostSource.files)
        {
            NntpFile *nntpFile = new NntpFile(this, QFileInfo(repostFile.path),
                                              repostFile.num, repostFile.nbFiles, repostFile.padding,
                                              repostFile.groups,
                                              repostFile.nbArticles, static_cast<uint>(repostFile.parts.size()));
            nntpFile->setRepostParts(repostFile.parts);
            connect(nntpFile, &NntpFile::allArticlesArePosted, this, &PostingJob::onNntpFilePosted, Qt::QueuedConnection);
            connect(nntpFile, &NntpFile::errorReadingFile,     this, &PostingJob::onNntpErrorReading, Qt::QueuedConnection);
            _filesToUpload.enqueue(nntpFile);
            _nbArticlesTotal += nntpFile->nbExpectedArticles();
            for (const RepostPart &part : repostFile.parts)
                _plannedSize += static_cast<quint64>(part.bytes);
        }
        emit articlesNumber(_nbArticlesTotal);
        return;
    }

    // initialize the NntpFiles (active objects)
    _filesToUpload.reserve(static_cast<int>(_nbFiles));
    uint fileNum = 0;
    int nbGroups = _grpList.size();
    for (const QFileInfo &file : _files)
    {
        NntpFile *nntpFile = new NntpFile(this,
                                          file,
                                          ++fileNum,
                                          _nbFiles,
                                          numPadding,
                                          _obfuscateArticles && _ngPost->groupPolicyPerFile() && nbGroups > 1 ?
                                              QStringList(_grpList.at(std::rand() % nbGroups)) : _grpList);
        connect(nntpFile, &NntpFile::allArticlesArePosted, this, &PostingJob::onNntpFilePosted, Qt::QueuedConnection);
        connect(nntpFile, &NntpFile::errorReadingFile,     this, &PostingJob::onNntpErrorReading, Qt::QueuedConnection);
        if (_ngPost->_dispFilesPosting && _ngPost->debugMode())
            connect(nntpFile, &NntpFile::startPosting, this, &PostingJob::onNntpFileStartPosting, Qt::QueuedConnection);

        _filesToUpload.enqueue(nntpFile);
        _nbArticlesTotal += nntpFile->nbExpectedArticles();
        _plannedSize += static_cast<quint64>(nntpFile->fileSize());
    }
    emit articlesNumber(_nbArticlesTotal);
}

void PostingJob::_readMediaInfo()
{
    if (!_ngPost->_genNfo || !_ngPost->_nfoMedia || _nfoHidesNames() || _isRepost)
        return;
    QString program = MediaInfoReader::findProgram(_ngPost->_mediaInfoPath);
    if (program.isEmpty())
    {
        _log(tr("MediaInfo was not found: the NFO file has no media information"));
        return;
    }
    QList<QPair<QString, QString>> files;
    for (const QFileInfo &fi : _inputFiles)
    {
        if (fi.isDir())
        {
            QDir base(fi.absoluteFilePath());
            QDirIterator it(base.absolutePath(), QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            QList<QPair<QString, QString>> found;
            while (it.hasNext())
            {
                it.next();
                found.append({fi.fileName() + "/" + base.relativeFilePath(it.filePath()), it.filePath()});
            }
            std::sort(found.begin(), found.end());
            files += found;
        }
        else
            files.append({fi.fileName(), fi.absoluteFilePath()});
    }
    MediaInfoReader::Result res = MediaInfoReader::read(program, files);
    _nfoMediaText      = res.text;
    _nfoMediaFiles     = res.nbMediaFiles;
    _nfoMediaDescribed = res.nbDescribed;
    _nfoMediaTimedOut  = res.timedOut;
}

void PostingJob::_prepareNfo()
{
    NfoWriter::NfoInfo info;
    info.postName   = QFileInfo(_nzbFilePath).completeBaseName();
    info.appName    = _ngPost->appName();
    info.appVersion = NgPost::versionLabel();
    info.date       = QDateTime::currentDateTime();
    for (const QFileInfo &fi : _inputFiles)
    {
        if (fi.isDir())
        {
            QDir base(fi.absoluteFilePath());
            QDirIterator it(base.absolutePath(), QDir::Files|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            QList<NfoWriter::File> found;
            while (it.hasNext())
            {
                it.next();
                found.append({fi.fileName() + "/" + base.relativeFilePath(it.filePath()), it.fileInfo().size()});
            }
            std::sort(found.begin(), found.end(), [](const NfoWriter::File &a, const NfoWriter::File &b){ return a.name < b.name; });
            info.files += found;
        }
        else
            info.files.append({fi.fileName(), fi.size()});
    }
    // the names of the files must not appear in a post that hides them
    info.hideNames  = _nfoHidesNames();
    info.compressed = _doCompress;
    info.archiveFormat = _use7z ? "7z" : "RAR";
    info.par2       = _doPar2;
    info.par2Pct    = _par2Pct;
    for (const QFileInfo &fi : _files)
    {
        info.uploadedSize += fi.size();
        if (_doCompress && !fi.fileName().endsWith(".par2", Qt::CaseInsensitive))
            ++info.nbVolumes;
    }
    if (_ngPost->_nfoPassword)
        info.password = _rarPass;
    info.freeText = _ngPost->_nfoText;
    info.mediaEnabled   = _ngPost->_nfoMedia;
    info.media          = _nfoMediaText;
    info.mediaFiles     = _nfoMediaFiles;
    info.mediaDescribed = _nfoMediaDescribed;
    info.mediaTimedOut  = _nfoMediaTimedOut;
    _nfoContent   = NfoWriter::build(info);

    if (!_ngPost->_nfoInPost)
        return;
    _nfoDir = new QTemporaryDir();
    if (!_nfoDir->isValid())
    {
        _error(tr("Couldn't create a temporary folder for the NFO file: it is not posted"));
        return;
    }
    // with obfuscated file names the NFO gets a random name too
    QString name = _obfuscateFileName ? _ngPost->randomPass(_ngPost->_lengthName) : info.postName;
    QFile nfo(_nfoDir->filePath(name + ".nfo"));
    if (nfo.open(QIODevice::WriteOnly))
    {
        nfo.write(_nfoContent.toUtf8());
        nfo.close();
        _files << QFileInfo(nfo.fileName());
    }
    else
        _error(tr("Couldn't write the NFO file: it is not posted"));
}

bool PostingJob::_nfoHidesNames() const
{
    return _ngPost->_nfoInPost && (_obfuscateFileName || _obfuscateArticles);
}

void PostingJob::_saveNfoNextToNzb()
{
    if (_nfoContent.isEmpty())
        return;
    QFileInfo nzb(_nzbFilePath);
    QSaveFile file(QString("%1/%2.nfo").arg(nzb.absolutePath(), nzb.completeBaseName()));
    if (file.open(QIODevice::WriteOnly) && file.write(_nfoContent.toUtf8()) >= 0 && file.commit())
        _log(tr("NFO file written: %1").arg(QDir::toNativeSeparators(QFileInfo(file.fileName()).absoluteFilePath())));
    else
        _error(tr("Couldn't write the NFO file next to the nzb: %1").arg(QDir::toNativeSeparators(file.fileName())));
}

void PostingJob::_finishPosting()
{
#ifdef __DEBUG__
qDebug() << "[MB_TRACE][PostingJob::_finishPosting]";
#endif
    _stopPosting = 0x1;

    if (_timeStart.isValid() && _postFinished)
    {
        _nbArticlesUploaded = _nbArticlesTotal; // we might not have processed the last onArticlePosted
        _uploadedSize       = _totalSize;
    }

    if (_ngPost->debugMode())
        _logAs(NgPost::LOG_DEBUG, "Finishing posting...");

    _phase = Phase::DONE;
    _ngPost->_finishPosting(); // to update progress bar

    // 1.: print stats
    if (_timeStart.isValid())
        _printStats();


    for (NntpConnection *con : _nntpConnections)
        emit con->killConnection();

    qApp->processEvents();


    // 4.: stop and wait for all threads
    // 2.: close nzb file
    _closeNzb();
    _fillRepostMeta();


    // 3.: close all the connections (they're living in the _threadPool)
    for (Poster *poster : _posters)
        poster->stopThreads();

    if (_ngPost->debugMode())
        _logAs(NgPost::LOG_DEBUG, "All posters stopped...");


#ifdef __DEBUG__
    _logAs(NgPost::LOG_DEBUG, "All connections are closed...");
#endif

    // 5.: print out the list of files that havn't been posted
    // (in case of disconnection)
    int nbPendingFiles = _filesToUpload.size() + _filesInProgress.size() + _filesFailed.size();
    if (nbPendingFiles)
    {
        _error(tr("ERROR: there were %1 on %2 that havn't been posted:").arg(
                   nbPendingFiles).arg(_nbFiles));
        bool isDebugMode = _ngPost->debugMode();
        for (NntpFile *file : _filesInProgress)
        {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fInProgress%1)").arg(file->missingArticles());
            _error(msg);
        }
        for (NntpFile *file : _filesToUpload)
        {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fToUpload%1)").arg(file->missingArticles());
            _error(msg);
        }
        for (NntpFile *file : _filesFailed)
        {
            QString msg = QString("  - %1").arg(file->stats());
            if (isDebugMode)
                msg += QString(" (fFailed%1)").arg(file->missingArticles());
            _error(msg);
        }
        _error(tr("you can try to repost only those and concatenate the nzb with the current one ;)"));
    }
    else if (_postFinished && MB_LoadAtomic(_delFilesAfterPost) && !_isRepost)
    {
        if (_nbArticlesFailed == 0 && _repostInfo.isEmpty())
            _delOriginalFiles();
        else
            _logAs(NgPost::LOG_WARN, tr("The original files have NOT been deleted because some Articles couldn't be posted."));
    }

    if (_isRepost)
        _applyRepost();

    _ngPost->queueJobEnded(this);
}

PostingJob *PostingJob::createRepostJob(NgPost *ngPost, const RepostInfo &info, PostingWidget *postWidget)
{
    QFileInfoList files;
    for (const RepostFile &file : info.files)
        files << QFileInfo(file.path);

    PostingJob *job = new PostingJob(ngPost, info.nzbPath, files, postWidget,
                                     info.grpList, info.from,
                                     info.obfuscateArticles, false,
                                     QString(), QString(), QString(), 0, false, 0,
                                     false, false, QString(), QString(),
                                     info.keepRar, false, true);
    job->_isRepost       = true;
    job->_queueId        = info.queueId;
    job->_repostSource   = info;
    job->_originalFiles  = info.originals;
    job->_delFilesAfterPost = info.delOriginals ? 0x1 : 0x0;
    job->_packed         = true; // the archives already exist
    return job;
}

static QJsonArray sToJsonArray(const QList<QString> &list)
{
    QJsonArray arr;
    for (const QString &str : list)
        arr << str;
    return arr;
}

QJsonObject PostingJob::toQueueEntry() const
{
    QJsonObject entry;
    entry.insert("id", _queueId);
    entry.insert("state", PostQueue::sStateWaiting);
    entry.insert("added", QDateTime::currentDateTime().toString(Qt::ISODate));
    QList<QString> files;
    for (const QFileInfo &fi : _inputFiles)
        files << fi.absoluteFilePath();
    entry.insert("files", sToJsonArray(files));
    entry.insert("nzb", _nzbFilePath);
    entry.insert("nzbFinal", _nzbPathFinal);
    entry.insert("overwriteNzb", _overwriteNzb);
    entry.insert("groups", sToJsonArray(_grpList));
    entry.insert("from", QString::fromStdString(_from));
    entry.insert("obfuscateArticles", _obfuscateArticles);
    entry.insert("obfuscateFileName", _obfuscateFileName);
    entry.insert("tmpPath", _tmpPath);
    entry.insert("rarPath", _rarPath);
    entry.insert("rarArgs", _rarArgs);
    entry.insert("rarSize", static_cast<int>(_rarSize));
    entry.insert("useRarMax", _useRarMax);
    entry.insert("par2Pct", static_cast<int>(_par2Pct));
    entry.insert("compress", _doCompress);
    entry.insert("par2", _doPar2);
    entry.insert("rarName", _rarName);
    entry.insert("rarPass", _rarPass);
    entry.insert("keepRar", _keepRar);
    entry.insert("delFiles", MB_LoadAtomic(_delFilesAfterPost) ? true : false);
    return entry;
}

PostingJob *PostingJob::fromQueueEntry(NgPost *ngPost, const QJsonObject &entry, PostingWidget *postWidget, QString &error)
{
    QFileInfoList files;
    QStringList   missingFiles;
    for (const QJsonValue &val : entry.value("files").toArray())
    {
        QFileInfo fi(val.toString());
        if (fi.exists())
            files << fi;
        else
            missingFiles << val.toString();
    }
    if (!missingFiles.isEmpty() || files.isEmpty())
    {
        error = tr("the files to post are not there anymore: %1").arg(missingFiles.join(", "));
        return nullptr;
    }

    QList<QString> groups;
    for (const QJsonValue &val : entry.value("groups").toArray())
        groups << val.toString();

    bool nzbFinal = entry.value("nzbFinal").toBool();
    PostingJob *job = new PostingJob(ngPost, entry.value("nzb").toString(), files, postWidget,
                                     groups, entry.value("from").toString().toStdString(),
                                     entry.value("obfuscateArticles").toBool(), entry.value("obfuscateFileName").toBool(),
                                     entry.value("tmpPath").toString(), entry.value("rarPath").toString(),
                                     entry.value("rarArgs").toString(),
                                     static_cast<uint>(entry.value("rarSize").toInt()), entry.value("useRarMax").toBool(),
                                     static_cast<uint>(entry.value("par2Pct").toInt()),
                                     entry.value("compress").toBool(), entry.value("par2").toBool(),
                                     entry.value("rarName").toString(), entry.value("rarPass").toString(),
                                     entry.value("keepRar").toBool(), entry.value("delFiles").toBool(),
                                     // the nzb of the interrupted post is ours: overwrite it
                                     nzbFinal ? true : entry.value("overwriteNzb").toBool(true));
    job->_queueId = entry.value("id").toString();
    job->_resumed = true;
    return job;
}

void PostingJob::_fillRepostMeta()
{
    if (_isRepost || _repostInfo.isEmpty())
        return;
    _repostInfo.queueId           = _queueId;
    _repostInfo.nzbPath           = _nzbFilePath;
    _repostInfo.grpList           = _grpList;
    _repostInfo.from              = _from;
    _repostInfo.obfuscateArticles = _obfuscateArticles;
    _repostInfo.compressDir       = _compressDir ? _compressDir->absolutePath() : QString();
    _repostInfo.keepRar           = _keepRar;
    _repostInfo.delOriginals      = MB_LoadAtomic(_delFilesAfterPost);
    _repostInfo.originals         = _originalFiles;
}

void PostingJob::_collectRepostIds(NntpFile *nntpFile)
{
    QHash<uint, QString> postedIds;
    nntpFile->postedMsgIds(postedIds);
    for (const RepostFile &repostFile : _repostSource.files)
    {
        if (repostFile.path != nntpFile->path())
            continue;
        for (const RepostPart &repostPart : repostFile.parts)
        {
            auto it = postedIds.constFind(repostPart.part);
            if (it != postedIds.constEnd() && !it.value().isEmpty())
                _repostIds.append(qMakePair(repostPart.oldMsgId, it.value()));
        }
        break;
    }
}

bool PostingJob::_patchNzb(const QVector<QPair<QString, QString>> &ids, QSet<QString> &fixed)
{
    const QString &nzbPath = _repostSource.nzbPath;
    QFile nzb(nzbPath);
    if (!nzb.open(QIODevice::ReadOnly))
    {
        _error(tr("Error: can't read the nzb file to fix it: %1").arg(nzbPath));
        return false;
    }
    QString content = QString::fromUtf8(nzb.readAll());
    nzb.close();

    for (const QPair<QString, QString> &id : ids)
    {
        const QString oldSegment = QString(">%1</segment>").arg(id.first);
        if (id.first.isEmpty() || !content.contains(oldSegment))
        {
            _error(tr("Error: the Message-ID of an Article is not in the nzb file: %1").arg(id.first));
            continue;
        }
        content.replace(oldSegment, QString(">%1</segment>").arg(id.second));
        fixed.insert(id.first);
    }

    if (fixed.isEmpty())
        return true; // nothing to write

    QSaveFile out(nzbPath);
    if (!out.open(QIODevice::WriteOnly) || out.write(content.toUtf8()) < 0 || !out.commit())
    {
        _error(tr("Error: can't write the fixed nzb file: %1").arg(nzbPath));
        fixed.clear();
        return false;
    }
    return true;
}

void PostingJob::_applyRepost()
{
    _repostApplied = true;
    QSet<QString> fixed;
    if (!_repostIds.isEmpty())
        _patchNzb(_repostIds, fixed);

    // what is still missing: the Articles that are not posted, or posted but not fixed in the nzb
    RepostInfo remaining = _repostSource;
    remaining.files.clear();
    int nbFixed = 0;
    for (const RepostFile &repostFile : _repostSource.files)
    {
        RepostFile left = repostFile;
        left.parts.clear();
        for (const RepostPart &repostPart : repostFile.parts)
        {
            if (fixed.contains(repostPart.oldMsgId))
                ++nbFixed;
            else
                left.parts.append(repostPart);
        }
        if (!left.parts.isEmpty())
            remaining.files.append(left);
    }

    if (nbFixed)
        _logAs(NgPost::LOG_OK, tr("%1 missing Articles posted, the nzb file has been updated: %2").arg(nbFixed).arg(_repostSource.nzbPath));

    if (!remaining.isEmpty())
    {
        _repostInfo = remaining;
        _error(tr("%1 Articles are still missing: they can be posted again.").arg(remaining.nbArticles()));
        return;
    }

    _logAs(NgPost::LOG_OK, tr("All the Articles are now posted."));
    if (!_repostSource.compressDir.isEmpty() && !_repostSource.keepRar)
    {
        QDir(_repostSource.compressDir).removeRecursively();
        _log(tr("Temporary files deleted."));
    }
    if (MB_LoadAtomic(_delFilesAfterPost))
        _delOriginalFiles();
}

void PostingJob::_closeNzb()
{
    if (_nzb)
    {
        if (_nzb->isOpen())
        {
            _nzbStream << "</nzb>\n";
            _nzbStream.flush();
            _nzb->close();
            _ngPost->doNzbPostCMD(this);
        }
        _nzbStream.setDevice(nullptr); // the stream must never write into the deleted file
        delete _nzb;
        _nzb = nullptr;
    }

}

void PostingJob::_printStats() const
{
    QString size = postSize();

    int duration = static_cast<int>(_timeStart.elapsed() - _pauseDuration);
    double sec = duration/1000;

    QString msgEnd("\n"), ts = QString("[%1] ").arg(timestamp());
    if (!_ngPost->useHMI())
        msgEnd += ts;
    msgEnd += tr("Upload size: %1 in %2 (%3 sec) \
                 => average speed: %4 (%5 connections on %6 threads)\n").arg(size).arg(
            QTime::fromMSecsSinceStartOfDay(duration).toString("hh:mm:ss.zzz")).arg(sec).arg(
                avgSpeed()).arg(_nntpConnections.size()+_closedConnections.size()).arg(_posters.size()*2);

    Diagnostics::instance().event("UPLOAD", QString("end: %1 in %2 s => %3 | %4 connections | articles: %5 ok, %6 failed / %7").arg(
                                      size).arg(sec).arg(avgSpeed()).arg(_nntpConnections.size()+_closedConnections.size()).arg(
                                      _nbArticlesUploaded - _nbArticlesFailed).arg(_nbArticlesFailed).arg(_nbArticlesTotal));
    QString latency = LatencyStats::instance().oneLine();
    if (!latency.isEmpty())
    {
        Diagnostics::instance().event("UPLOAD", latency);
        msgEnd += latency + "\n";
    }
    if (_nbArticlesFailed > 0)
        Diagnostics::instance().anomaly(QString("%1 / %2 articles failed to be uploaded").arg(_nbArticlesFailed).arg(_nbArticlesTotal));

    if (_nbArticlesFailed > 0)
        msgEnd += tr("%1 / %2 articles FAILED to be uploaded (even with %3 retries)...\n").arg(
                      _nbArticlesFailed).arg(_nbArticlesTotal).arg(NntpArticle::nbMaxTrySending());

    if (_compressMs >= 0 || _par2Ms >= 0)
    {
        // where the time went (only when there is more than the upload)
        QStringList phases;
        if (_compressMs >= 0)
            phases << tr("Compression %1").arg(humanDuration(_compressMs));
        if (_par2Ms >= 0)
            phases << tr("par2 %1").arg(humanDuration(_par2Ms));
        phases << tr("Upload %1").arg(humanDuration(duration));
        phases << tr("Total %1").arg(humanDuration(jobDurationMs()));
        if (!_ngPost->useHMI())
            msgEnd += ts;
        msgEnd += phases.join(QString::fromUtf8(" \u00b7 ")) + "\n";
    }

    if (_nzb)
    {
        if (!_ngPost->useHMI())
            msgEnd += ts;
        msgEnd += tr("nzb file: %1\n").arg(_nzbFilePath);
        if (_doCompress)
        {
            if (!_ngPost->useHMI())
                msgEnd += ts;
            msgEnd += tr("file: %1, rar name: %2").arg(_nzbName).arg(_rarName);
            if (!_rarPass.isEmpty())
                msgEnd += tr(", rar pass: %1").arg(_rarPass);
        }
        if (!_ngPost->_urlNzbUpload)
            msgEnd += "\n";
    }

    _logAs(_nbArticlesFailed > 0 ? NgPost::LOG_WARN : NgPost::LOG_OK, msgEnd);
}

qint64 PostingJob::etaSeconds()
{
    if (_phase != Phase::UPLOADING || _isPaused || !_timeStart.isValid() || _plannedSize == 0)
        return -1;
    // clock of the upload without the pauses, speed of the last seconds (smoothed)
    qint64 now = _timeStart.elapsed() - _pauseDuration;
    if (_etaLastMs < 0)
    {
        _etaLastMs    = now;
        _etaLastBytes = _uploadedSize;
    }
    else if (now - _etaLastMs >= 1000)
    {
        double inst = static_cast<double>(_uploadedSize - _etaLastBytes) * 1000. / static_cast<double>(now - _etaLastMs);
        _etaSpeed     = _etaSpeed > 0. ? 0.7 * _etaSpeed + 0.3 * inst : inst;
        _etaLastMs    = now;
        _etaLastBytes = _uploadedSize;
    }
    if (_etaSpeed <= 0. || _uploadedSize >= _plannedSize)
        return -1;
    return static_cast<qint64>(std::ceil(static_cast<double>(_plannedSize - _uploadedSize) / _etaSpeed));
}

QString PostingJob::humanDuration(qint64 ms)
{
    if (ms < 0)
        ms = 0;
    if (ms < 10000)
        return tr("%1 s").arg(ms / 1000., 0, 'f', 1);
    qint64 sec = (ms + 500) / 1000;
    if (sec < 60)
        return tr("%1 s").arg(sec);
    qint64 min = sec / 60;
    if (min < 60)
        return tr("%1 min %2 s").arg(min).arg(sec % 60);
    return tr("%1 h %2 min").arg(min / 60).arg(min % 60);
}



bool PostingJob::startCompressFiles(const QString &cmdRar,
                                   const QString &tmpFolder,
                                   const QString &archiveName,
                                   const QString &pass,
                                   uint volSize)
{
    if (!_canCompress())
        return false;

    // 1.: create archive temporary folder
    QString archiveTmpFolder = _createArchiveFolder(tmpFolder, archiveName);
    if (archiveTmpFolder.isEmpty())
        return false;

    _extProc = new QProcess(this);
    connect(_extProc, &QProcess::readyReadStandardOutput, this, &PostingJob::onExtProcReadyReadStandardOutput, Qt::DirectConnection);
    connect(_extProc, &QProcess::readyReadStandardError,  this, &PostingJob::onExtProcReadyReadStandardError,  Qt::DirectConnection);
    connect(_extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &PostingJob::onCompressionFinished, Qt::QueuedConnection);

    _use7z = false;
    // check the program name (7z, 7za, 7zz, 7zr, 7z.exe...) and not the whole path:
    // a folder like "/opt/7zip/" must not turn rar into 7z
    if (QFileInfo(_rarPath).completeBaseName().toLower().startsWith("7z"))
    {
        _use7z = true;
        if (_rarArgs.isEmpty())
            _rarArgs = _ngPost->sDefault7zOptions;
    }
    else
    {
        if (_rarArgs.isEmpty())
            _rarArgs = _ngPost->sDefaultRarExtraOptions;
    }

    // 2.: create rar args (rar a -v50m -ed -ep1 -m0 -hp"$PASS" "$TMP_FOLDER/$RAR_NAME.rar" "${FILES[@]}")
//    QStringList args = {"a", "-idp", "-ep1", compressLevel, QString("%1/%2.rar").arg(archiveTmpFolder).arg(archiveName)};
    QStringList args = NgPost::splitArgs(_rarArgs); // respects the double quotes (and ignores multiple spaces)
    if (!args.contains("a"))
        args.prepend("a");
    if (!_use7z && !args.contains("-idp"))
        args << "-idp";
    // The archive password is NOT put on the command line anymore (it would be visible in the
    // process list / Task Manager): 7z reads it on its standard input, rar from the RAR environment variable.
    // Fallback on the command line only for passwords that those channels can't carry safely
    // (non ASCII characters for both, double quotes for rar).
    QByteArray passForStdin;
    if (!pass.isEmpty())
    {
        bool asciiOnly = true;
        for (const QChar &c : pass)
        {
            if (c.unicode() < 0x20 || c.unicode() > 0x7E)
            {
                asciiOnly = false;
                break;
            }
        }
        if (_use7z)
        {
            if (!args.contains("-mhe=on"))
                args << "-mhe=on";
            if (asciiOnly)
            {
                args << "-p"; // empty => 7z asks it on stdin (twice on some versions to verify)
                passForStdin = QString("%1\n%1\n").arg(pass).toLatin1();
            }
            else
                args << QString("-p%1").arg(pass);
        }
        else
        {
            if (asciiOnly && !pass.contains('"'))
            {
                QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
                QString rarEnv = env.value("RAR"); // keep the user's default switches if any
                env.insert("RAR", QString("%1%2-hp\"%3\"").arg(rarEnv, rarEnv.isEmpty() ? "" : " ", pass));
                _extProc->setProcessEnvironment(env);
            }
            else
                args << QString("-hp%1").arg(pass);
        }
    }
    if (volSize > 0 || _useRarMax)
    {
        if (_useRarMax)
        {
            qint64 postSize = 0;
            for (const QFileInfo &fileInfo : _files)
            {
                if (fileInfo.isDir())
                    postSize += _dirSize(fileInfo.absoluteFilePath());
                else
                    postSize += fileInfo.size();
            }
            postSize /= 1024*1024; // to get it in MB
            if (volSize > 0)
            {
                if (postSize / volSize > _ngPost->_rarMax)
                    volSize = static_cast<uint>(postSize / _ngPost->_rarMax) + 1;
            }
            else
                volSize = static_cast<uint>(postSize / _ngPost->_rarMax) + 1;

#ifdef __DEBUG__
            qDebug() << tr("postSize: %1 MB => volSize: %2").arg(postSize).arg(volSize);
#endif
            if (_ngPost->debugMode())
                _log(tr("postSize: %1 MB => volSize: %2").arg(postSize).arg(volSize));
        }
        args << QString("-v%1m").arg(volSize);
        _splitArchive = true;
    }

#if defined( Q_OS_WIN )
    if (archiveTmpFolder.startsWith("//"))
        archiveTmpFolder.replace(QRegExp("^//"), "\\\\");
#endif
    args << QString("%1/%2.%3").arg(archiveTmpFolder, archiveName, _use7z ? "7z" : "rar");


    if (_obfuscateFileName)
    {
        _files.clear();
        for (const QFileInfo &fileInfo : _originalFiles)
        {
            QString obfuscatedName = QString("%1/%2").arg(fileInfo.absolutePath(), _ngPost->randomPass(_ngPost->_lengthName)),
                    fileName = fileInfo.absoluteFilePath();

            if (QFile::rename(fileName, obfuscatedName))
            {
                _obfuscatedFileNames.insert(obfuscatedName, fileName);
                _files << QFileInfo(obfuscatedName);
            }
            else
            {
                _files << fileInfo;
                _error(tr("Couldn't rename file %1").arg(fileName));
            }
        }
    }


    bool hasDir = false;
    if (_files.size() == 1)
    { // Only remove rar root folder if there is only ONE folder AND RAR_NO_ROOT_FOLDER is set
        const QFileInfo &fileInfo = _files.first();
        QString path = fileInfo.absoluteFilePath();
#if defined( Q_OS_WIN )
        if (path.startsWith("//"))
            path.replace(QRegExp("^//"), "\\\\");
#endif
        if (fileInfo.isDir())
        {
            hasDir = true;
            if (_ngPost->removeRarRootFolder())
                path += "/";
        }
        args << path;
    }
    else
    {
        for (const QFileInfo &fileInfo : _files)
        {
            QString path = fileInfo.absoluteFilePath();
#if defined( Q_OS_WIN )
            if (path.startsWith("//"))
                path.replace(QRegExp("^//"), "\\\\");
#endif
            if (fileInfo.isDir())
                hasDir = true;
            args << path;
        }
    }

    if (hasDir && !args.contains("-r"))
        args << "-r";

    // 3.: launch rar
    if (_ngPost->debugMode() || !_postWidget)
        _logAs(NgPost::LOG_PHASE, QString("[%1] %2: %3 %4\n").arg(timestamp()).arg(tr("Compressing files")).arg(cmdRar).arg(args.join(" ")));
    else
        _logAs(NgPost::LOG_PHASE, QString("%1...\n").arg(tr("Compressing files")));
    _limitProcDisplay = false;
    _diagRegisterPost();
    Diagnostics::instance().event("COMPRESS", QString("start: %1 %2 | input: %3 in %4 file(s)/folder(s) | tmp: %5").arg(
                                      cmdRar, args.join(" "), Diagnostics::humanSize(_diagInputSize)).arg(
                                      (_originalFiles.isEmpty() ? _files : _originalFiles).size()).arg(archiveTmpFolder));
    _diagCompressing = true;
    _diagFirstOut = _diagLastOut = _diagProcDone = -1;
    _diagNbOut = 0;
    _diagOutTail.clear();
    // exact moment when the compressor exits (the "finished" slot is queued: it could run later)
    connect(_extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this,
            [this](){ if (_diagCompressing && _diagProcDone < 0) _diagProcDone = _diagTimer.elapsed(); }, Qt::DirectConnection);
    connect(_extProc, &QProcess::started, this,
            [this](){ if (_diagCompressing) Diagnostics::instance().event("COMPRESS", QString("process started after %1 ms").arg(_diagTimer.elapsed())); },
            Qt::DirectConnection);
    _phase = Phase::COMPRESSING;
    _diagTimer.start();
    _extProc->start(cmdRar, args);
    if (!passForStdin.isEmpty())
    {
        _extProc->write(passForStdin);
        _extProc->closeWriteChannel();
    }

#ifdef __DEBUG__
    _log("[PostingJob::_compressFiles] compression started...");
#endif

    return true;
}


void PostingJob::_diagRegisterPost()
{
    if (_diagRegistered)
        return;
    _diagRegistered = true;
    Diagnostics &diag = Diagnostics::instance();
    _diagInputSize = 0;
    for (const QFileInfo &fileInfo : (_originalFiles.isEmpty() ? _files : _originalFiles))
    {
        QString label = diag.nextInputLabel();
        diag.addSecret(fileInfo.absoluteFilePath(), label);
        diag.addSecret(QDir::toNativeSeparators(fileInfo.absoluteFilePath()), label);
        diag.addSecret(fileInfo.fileName(), label);
        if (fileInfo.completeBaseName().size() >= 5)
            diag.addSecret(fileInfo.completeBaseName(), label);
        _diagInputSize += fileInfo.isDir() ? _dirSize(fileInfo.absoluteFilePath()) : fileInfo.size();
    }
    diag.addSecret(_nzbName, "<nzb>");
    diag.addSecret(QFileInfo(_nzbName).completeBaseName(), "<nzb>");
    diag.addSecret(_rarName, "<archive>");
    diag.addSecret(_rarPass, "<password>");
}

void PostingJob::onCompressionFinished(int exitCode)
{
    {
        qint64 ms = _diagTimer.isValid() ? _diagTimer.elapsed() : 0;
        QString msg = QString("end: exit code %1 | %2 in %3 s => %4").arg(exitCode).arg(
                    Diagnostics::humanSize(_diagInputSize)).arg(ms / 1000.0, 0, 'f', 1).arg(Diagnostics::speed(_diagInputSize, ms));
        Diagnostics::instance().event("COMPRESS", msg);
        _compressMs = ms;
        if (_diagCompressing)
        {
            QString tail = QString::fromLocal8Bit(_diagOutTail).trimmed().replace('\r', ' ').replace('\n', " | ");
            Diagnostics::instance().event("COMPRESS", QString("compressor exited after %1 ms, slot run after %2 ms | output: %3 chunk(s), first after %4 ms, last after %5 ms | end of the output: %6").arg(
                                              _diagProcDone).arg(ms).arg(_diagNbOut).arg(_diagFirstOut).arg(_diagLastOut).arg(tail.right(300)));
            _diagCompressing = false;
            _diagProcDone    = 0;
        }
        if (exitCode != 0)
            Diagnostics::instance().anomaly(QString("compression failed (exit code %1)").arg(exitCode));
        else if (_diagInputSize >= 20*1024*1024 && ms > 0 && _diagInputSize * 1000. / ms < 20.*1024*1024)
            Diagnostics::instance().anomaly(QString("slow compression: %1 for %2 (expected more than 20 MB/s, "
                                                    "check the temporary folder: synced folder, antivirus, network or slow disk)").arg(
                                                Diagnostics::speed(_diagInputSize, ms), Diagnostics::humanSize(_diagInputSize)));
    }
    if (_ngPost->debugMode())
        _log(tr("=> rar exit code: %1\n").arg(exitCode));
    else
        _log("\n");
    if (exitCode == 0)
        _logAs(NgPost::LOG_PHASE, tr("Compression done in %1").arg(humanDuration(_compressMs)));


#ifdef __DEBUG__
    _log("[PostingJob::_compressFiles] compression finished...");
#endif

    if (_obfuscateFileName && !MB_LoadAtomic(_delFilesAfterPost) )
    {
        for (auto it = _obfuscatedFileNames.cbegin(), itEnd = _obfuscatedFileNames.cend(); it != itEnd; ++it)
            QFile::rename(it.key(), it.value());
    }


    if (exitCode != 0)
    {
        _error(tr("Error during compression: %1").arg(exitCode));
        _cleanCompressDir();
        emit postingFinished();
    }
    else
    {
        if (_doPar2)
        {
            disconnect(_extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                       this, &PostingJob::onCompressionFinished);
            if (!startGenPar2(_tmpPath, _rarName, _par2Pct))
            {
                _cleanCompressDir();
                emit postingFinished();
            }
        }
        else
        {
            _packed = true;
            _cleanExtProc();
            if (this == _ngPost->_activeJob)
                _postFiles();

            emit packingDone();
        }
    }
}

bool PostingJob::startGenPar2(const QString &tmpFolder,
                     const QString &archiveName,
                     uint redundancy)
{
    if (!_canGenPar2())
        return false;

    const bool useParPar = _ngPost->useParPar();

    QStringList args;
    if (_ngPost->_par2Args.isEmpty())
    {
        if (useParPar) // ParPar doesn't understand the par2cmdline syntax
            args << "-s5M" << QString("-r%1%").arg(redundancy) << "-m1024M" << "--progress" << "stdout" << "-q";
        else
            args << "c" << "-l" << "-m1024" << QString("-r%1").arg(redundancy);
    }
    else
        args << NgPost::splitArgs(_ngPost->_par2Args);
    QString archiveTmpFolder = QString("%1/%2").arg(tmpFolder, archiveName);

    // we've already compressed => we gen par2 for the files in the archive folder
    if (_extProc)
    {
        if (useParPar && args.last().trimmed() != "-o")
            args << "-o";
        args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
        if (useParPar)
              args << "-R" << archiveTmpFolder;
        else
        {
            if (_use7z)
                args << QString("%1/%2.7z%3").arg(archiveTmpFolder, archiveName, _splitArchive ? "*": "");
            else
                args << QString("%1/%2*rar").arg(archiveTmpFolder, archiveName);

            if (_ngPost->_par2Args.isEmpty() && (_ngPost->debugMode() || !_postWidget))
                args << "-q"; // remove the progressbar bar
        }
    }
    else
    { // par2 generation only => can't use folders or files from different drive (Windows)
        QString basePath = _files.first().absolutePath();
        if (useParPar)
        {
            args << "-f" << "basename";
            if (args.last().trimmed() != "-o")
                args << "-o";
            args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
        }
        else
        {
#if defined( Q_OS_WIN )
            QString basePathWin(basePath);
            basePathWin.replace("/", "\\");
            if (_ngPost->useMultiPar())
                args << QString("/d%1").arg(basePathWin);
            else
                args <<"-B" << basePathWin;
            QString par2File = QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
            par2File.replace("/", "\\");
            args << par2File;
#else
            args << "-B" << basePath;
            args << QString("%1/%2.par2").arg(archiveTmpFolder, archiveName);
#endif
        }

        for (const QFileInfo &fileInfo : _files)
        {
            if (fileInfo.isDir()) {
                _error(tr("you can't post folders without compression..."));
                return false;
            }
            QString path = fileInfo.absoluteFilePath();
#if defined( Q_OS_WIN )
            if (!useParPar && basePath != fileInfo.absolutePath()){
//                _error(tr("Due to par2 software limitation and to avoid unnecessary copies, all files must be in the same folder..."));
                _error(tr("only ParPar allows to generate par2 for files from different folders... you should consider using it ;)"));
                return false;
            }
            path.replace("/", "\\");
#endif
            args << path;
        }

        QString archiveTmpFolder = _createArchiveFolder(tmpFolder, archiveName);
        if (archiveTmpFolder.isEmpty())
            return false;

        _extProc = new QProcess(this);
        connect(_extProc, &QProcess::readyReadStandardOutput, this, &PostingJob::onExtProcReadyReadStandardOutput, Qt::DirectConnection);
        connect(_extProc, &QProcess::readyReadStandardError,  this, &PostingJob::onExtProcReadyReadStandardError,  Qt::DirectConnection);
    }

    connect(_extProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &PostingJob::onGenPar2Finished, Qt::QueuedConnection);

    if (_ngPost->debugMode() || !_postWidget)
        _logAs(NgPost::LOG_PHASE, QString("[%1] %2: %3 %4").arg(timestamp()).arg(tr("Generating par2")).arg(_ngPost->_par2Path).arg(args.join(" ")));
    else
        _logAs(NgPost::LOG_PHASE, QString("%1...\n").arg(tr("Generating par2")));
    if (!_ngPost->useParPar())
        _limitProcDisplay = true;
    _nbProcDisp = 0;
    _diagRegisterPost();
    _diagCompressing = false;
    Diagnostics::instance().event("PAR2", QString("start: %1 %2").arg(_ngPost->_par2Path, args.join(" ")));
    _phase = Phase::PAR2;
    _diagTimer.start();
    _extProc->start(_ngPost->_par2Path, args);    

    return true;
}

void PostingJob::onGenPar2Finished(int exitCode)
{
    {
        qint64 ms = _diagTimer.isValid() ? _diagTimer.elapsed() : 0;
        _par2Ms = ms;
        Diagnostics::instance().event("PAR2", QString("end: exit code %1 | %2 in %3 s => %4").arg(exitCode).arg(
                                          Diagnostics::humanSize(_diagInputSize)).arg(ms / 1000.0, 0, 'f', 1).arg(
                                          Diagnostics::speed(_diagInputSize, ms)));
        if (exitCode != 0)
            Diagnostics::instance().anomaly(QString("par2 generation failed (exit code %1)").arg(exitCode));
        else if (_diagInputSize >= 20*1024*1024 && ms > 0 && _diagInputSize * 1000. / ms < 10.*1024*1024)
            Diagnostics::instance().anomaly(QString("slow par2 generation: %1 for %2 (is ParPar used?)").arg(
                                                Diagnostics::speed(_diagInputSize, ms), Diagnostics::humanSize(_diagInputSize)));
    }
    if (_ngPost->debugMode())
        _log(tr("=> par2 exit code: %1\n").arg(exitCode));
    else
        _log("\n");
    if (exitCode == 0)
        _logAs(NgPost::LOG_PHASE, tr("par2 generated in %1").arg(humanDuration(_par2Ms)));

    _cleanExtProc();

    if (exitCode != 0)
    {
        _error(tr("Error during par2 generation: %1").arg(exitCode));
        _cleanCompressDir();
        emit postingFinished();
    }
    else
    {
        _packed = true;
        if (this == _ngPost->_activeJob)
            _postFiles();

        emit packingDone();
    }
}


void PostingJob::_cleanExtProc()
{
    delete _extProc;
    _extProc = nullptr;
    if (_ngPost->debugFull())
        _log(tr("External process deleted."));
}

void PostingJob::_cleanCompressDir()
{
    if (!_keepRar)
        _compressDir->removeRecursively();
    if (_ngPost->debugMode())
        _log(tr("Compressed files deleted."));
}

QString PostingJob::_createArchiveFolder(const QString &tmpFolder, const QString &archiveName)
{
    QString archiveTmpFolder = QString("%1/%2").arg(tmpFolder).arg(archiveName);
    _compressDir = new QDir(archiveTmpFolder);
    if (_compressDir->exists() && _resumed)
    {
        // left by the interrupted post that we're starting again: it's ours, we can remove it
        _log(tr("Removing the temporary folder of the interrupted post: %1").arg(archiveTmpFolder));
        _compressDir->removeRecursively();
    }
    if (_compressDir->exists())
    {
        _error(tr("The temporary directory '%1' already exists... (either remove it or change the archive name)").arg(archiveTmpFolder));
        delete _compressDir;
        _compressDir = nullptr;
        return QString();
    }
    else
    {
        if (!_compressDir->mkpath("."))
        {
            _error(tr("Couldn't create the temporary folder: '%1'...").arg(archiveTmpFolder));
            delete _compressDir;
            _compressDir = nullptr;
            return QString();
        }
    }
    return archiveTmpFolder;
}


void PostingJob::onExtProcReadyReadStandardOutput()
{
    const QByteArray data = _extProc->readAllStandardOutput();
    if (_diagCompressing)
    {
        qint64 ms = _diagTimer.elapsed();
        if (_diagFirstOut < 0)
            _diagFirstOut = ms;
        _diagLastOut = ms;
        ++_diagNbOut;
        _diagOutTail.append(data);
        if (_diagOutTail.size() > 1000)
            _diagOutTail = _diagOutTail.right(1000);
    }
    if (_ngPost->debugMode())
        _log(data, false);
    else if (_isActiveJob)
    {
        if (!_limitProcDisplay || ++_nbProcDisp%42 == 0)
            _log("*", false);
    }
}

void PostingJob::onExtProcReadyReadStandardError()
{
    _error(_extProc->readAllStandardError());
}


bool PostingJob::_checkTmpFolder() const
{
    if (_tmpPath.isEmpty())
    {
        _error(tr("NO_POSSIBLE_COMPRESSION: You must define the temporary directory..."));
        return false;
    }

    QFileInfo fi(_tmpPath);
    if (!fi.exists() || !fi.isDir() || !fi.isWritable())
    {
        _error(tr("ERROR: the temporary directory must be a WRITABLE directory..."));
        return false;
    }

    return true;
}

qint64 PostingJob::_dirSize(const QString &path)
{
    qint64 size = 0;
    QDir dir(path);
    for(const QFileInfo &fi: dir.entryInfoList(QDir::Files|QDir::Hidden|QDir::System|QDir::Dirs|QDir::NoDotAndDotDot)) {
        if (fi.isDir())
            size += _dirSize(fi.absoluteFilePath());
        else
            size+= fi.size();
    }
    return size;
}


bool PostingJob::_canCompress() const
{
    //1.: the _tmp_folder must be writable
    if (!_checkTmpFolder())
        return false;

    //2.: check _rarPath is executable
    QFileInfo fi(_rarPath);
    if (!fi.exists() || !fi.isFile() || !fi.isExecutable())
    {
        _error(tr("ERROR: the RAR path is not executable..."));
        return false;
    }

    return true;
}

bool PostingJob::_canGenPar2() const
{
    //1.: the _tmp_folder must be writable
    if (!_checkTmpFolder())
        return false;

    //2.: check _ is executable
    QFileInfo fi(_ngPost->_par2Path);
    if (!fi.exists() || !fi.isFile() || !fi.isExecutable())
    {
        _error(tr("ERROR: par2 is not available..."));
        return false;
    }

    return true;
}
