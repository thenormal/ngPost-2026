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

#ifndef POSTINGJOB_H
#define POSTINGJOB_H
#include "utils/Macros.h"
#include "RepostInfo.h"

#include <QFileInfoList>
#include <QTemporaryDir>
#include <QVector>
#include <QSet>
#include <QQueue>
#include <QPair>
#include <QHash>
#include <QTextStream>
#include <QMutex>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QJsonObject>
class QProcess;
class NgPost;
class NntpConnection;
class NntpFile;
class NntpArticle;
struct NntpServerParams;
class PostingWidget;
class Poster;

using AtomicBool = QAtomicInteger<unsigned short>; // 16 bit only (faster than using 8 bit variable...)

/*!
 * \brief PostingJob is an active object that will do a posting job
 * it will be possible to move it to a thread to not freeze the HMI
 * it will signal it's progressbarion to PostingWidget
 *
 * NgPost will only have one PostingJob active at a time
 * but it will be able to hold a Pending queue
 */
class PostingJob : public QObject
{
    Q_OBJECT
    friend class Poster;
    friend class ArticleBuilder;
    friend class NgPost;

private:
    NgPost *const _ngPost; //!< handle on the application to access global configs
    QFileInfoList _files; //!< populated on constuction using a QStringList of paths

    PostingWidget *const _postWidget;

    QProcess   *_extProc;
    QDir       *_compressDir;
    bool        _limitProcDisplay;
    ushort      _nbProcDisp;

#ifdef __USE_TMP_RAM__
    QString        _tmpPath; //!< can be overwritten by _ngPost->_ramPath
#else
    const QString  _tmpPath;
#endif
    const QString  _rarPath;
    QString        _rarArgs;
    const uint     _rarSize;
    const bool     _useRarMax;
    const uint     _par2Pct;

    const bool    _doCompress;
    const bool    _doPar2;
    const QString _rarName;
    const QString _rarPass;
    const bool    _keepRar;
    bool          _splitArchive;



    QVector<NntpConnection*> _nntpConnections; //!< the NNTP connections (owning the TCP sockets)
    QVector<NntpConnection*> _closedConnections; //!< the NNTP connections (owning the TCP sockets)

    QString               _nzbName; //!< name of nzb that we'll write (without the extension)
    QQueue<NntpFile*>     _filesToUpload;  //!< list of files to upload (that we didn't start)
    QSet<NntpFile*>       _filesInProgress;//!< files in queue to be uploaded (Articles have been produced)
    QSet<NntpFile*>       _filesFailed;//!< files that couldn't be read
    uint                  _nbFiles;  //!< number of files to post in this iteration
    uint                  _nbPosted; //!< number of files posted


    QString      _nzbFilePath;
    QFile       *_nzb;       //!< nzb file that will be filled on the fly when a file is fully posted
    QTextStream  _nzbStream; //!< txt stream for the nzb file

    NntpFile    *_nntpFile; //!< current file that is getting processed
    QFile       *_file;     //!< file handler on the file getting processed
    uint         _part;     //!< part number (Article) on the current file

    QElapsedTimer _timeStart; //!< to get some stats (upload time and avg speed)
    QElapsedTimer _jobTimer;  //!< whole duration of the post (compression, par2 and upload) for the history
public:
    enum class Phase {WAITING, COMPRESSING, PAR2, UPLOADING, DONE};
private:
    Phase   _phase = Phase::WAITING;
    quint64 _plannedSize  = 0;  //!< bytes to post (known when the Articles are prepared, unlike _totalSize)
    qint64  _etaLastMs    = -1;
    quint64 _etaLastBytes = 0;
    double  _etaSpeed     = 0.; //!< bytes/s, smoothed
    qint64 _compressMs = -1; //!< duration of the compression (-1: no compression in this post)
    qint64 _par2Ms     = -1; //!< duration of the par2 generation (-1: no par2)
    QElapsedTimer _diagTimer;               //!< diagnostics: duration of the compression / par2 phase
    qint64        _diagInputSize   = 0;     //!< diagnostics: size of the files to pack
    bool          _diagRegistered  = false; //!< diagnostics: secrets (names, password) already registered
    bool          _waitingLastFiles = false; //!< all the connections are closed, waiting for the queued 'file posted' signals of the last files
    bool          _diagCompressing = false; //!< diagnostics: the external process is the compressor
    qint64        _diagFirstOut    = -1;    //!< diagnostics: ms (since the start) of the first output of the compressor
    qint64        _diagLastOut     = -1;    //!< diagnostics: ms of the last output
    qint64        _diagProcDone    = -1;    //!< diagnostics: ms when the compressor really exited
    int           _diagNbOut       = 0;     //!< diagnostics: number of outputs of the compressor
    QByteArray    _diagOutTail;             //!< diagnostics: end of the output of the compressor
    quint64       _totalSize; //!< total size (in Bytes) to be uploaded
    QElapsedTimer _pauseTimer;   //!< to record the time when ngPost is in pause
    qint64        _pauseDuration; //!< total duration of all pauses

    int     _nbConnections; //!< available number of NntpConnection (we may loose some)
    int     _nbThreads;     //!< size of the ThreadPool

    uint      _nbArticlesUploaded; //!< number of Articles that have been uploaded (+ failed ones)
    uint      _nbArticlesFailed;   //!< number of Articles that failed to be uploaded
    quint64   _uploadedSize;       //!< bytes posted (to compute the avg speed)
    uint      _nbArticlesTotal;    //!< number of Articles of all the files to post


    AtomicBool  _stopPosting;
    AtomicBool  _noMoreFiles;

    bool _postStarted;
    bool _packed;
    bool _postFinished;

    const bool _obfuscateArticles;
    const bool _obfuscateFileName;


    AtomicBool  _delFilesAfterPost;
    QFileInfoList       _originalFiles;
    QString             _nfoContent;   //!< text of the NFO file (empty if none)
    QTemporaryDir      *_nfoDir = nullptr; //!< the NFO that is posted lives here until the end of the post
    const int           _nbInputFiles; //!< number of files and folders given to post (for the history)


    QMutex _secureDiskAccess;

    QVector<Poster*> _posters;

    const bool  _overwriteNzb;

    QMap<QString, QString> _obfuscatedFileNames;

    const QList<QString> _grpList; //!< Newsgroup where we're posting in a list format to write in the nzb file
    const std::string    _from;               //!< email of poster (if empty, random one will be used for each file)

    bool _use7z;

    bool _isPaused;

    QTimer _resumeTimer;

    // ---- recovery of the failed Articles (shared by all the Posters, used from their threads)
    QMutex               _secureRecovery;   //!< protects the members below
    QQueue<NntpArticle*> _recoveryArticles; //!< Articles to post again (by any connection)
    QVector<NntpArticle*> _heldArticles;    //!< Articles that failed everywhere, kept for the second round
    bool                 _secondRoundDone   = false; //!< the second round has started (only one)
    bool                 _heldOverflow      = false; //!< too many failed Articles: no second round
    int                  _nbEnabledServers  = 1;

    bool _isActiveJob;

    // ---- post again the Articles that failed (and fix the nzb)
    bool       _isRepost = false;      //!< this job only posts again the missing Articles of an earlier post
    RepostInfo _repostSource;          //!< (repost job) what we have to post again
    QVector<QPair<QString, QString>> _repostIds; //!< (repost job) old Message-ID => new Message-ID of the Articles posted
    RepostInfo _repostInfo;            //!< what is still missing at the end of this job (for the "Repost missing" button)
    bool       _repostApplied = false; //!< (repost job) the nzb has been fixed / the remaining Articles computed

    // ---- persistent queue (PostQueue)
    QString             _queueId;           //!< id of the entry of this job in the queue (empty: not saved)
    const QFileInfoList _inputFiles;        //!< files and folders given to post (_files is replaced by the archives)
    bool                _resumed     = false; //!< job posted again from the queue (after a restart)
    bool                _nzbPathFinal = false; //!< _nzbFilePath is the final one (posting started)
    bool                _keepInQueue = false; //!< the post was interrupted (connections lost): try it again at the next start


#ifdef __COMPUTE_IMMEDIATE_SPEED__
    quint64    _immediateSize;       //!< bytes posted (to compute the avg speed)
    QTimer     _immediateSpeedTimer;
    QString    _immediateSpeed;
    const bool _useHMI;
#endif

public:
    PostingJob(NgPost *ngPost,
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
               bool keepRar = false,
               bool delFilesAfterPost = false,
               bool overwriteNzb = true,
               QObject *parent = nullptr);
    ~PostingJob();


    void pause();
    void resume();

    inline QString avgSpeed() const;

    // recovery of failed Articles (thread safe: called from the connection threads)
    void recoverArticle(NntpArticle *article, const NntpServerParams *server, const QString &conPrefix);
    NntpArticle *takeRecoveryArticle(const NntpServerParams *server, bool onlyIfNotFailedThere);
    void addToRecovery(const QQueue<NntpArticle*> &articles); //!< not failed, just stranded

    inline void articlePosted(quint64 size);
    inline void articleFailed(quint64 size);

    inline uint nbArticlesTotal() const;
    inline quint64 totalSizeBytes() const { return _plannedSize; }
    inline quint64 uploadedSizeBytes() const { return _uploadedSize; }

    inline Phase phase() const { return _phase; }
    inline bool uploadFinished() const { return _postFinished; }
    void setDone() { _phase = Phase::DONE; }
    qint64 etaSeconds(); //!< remaining time of the upload (smoothed on the last seconds), -1 if not known
    static QString humanDuration(qint64 ms); //!< "1 min 30 s"
    inline uint nbArticlesUploaded() const;
    inline uint nbArticlesFailed() const;
    inline bool hasUploaded() const;

    inline const QString &nzbName() const;
    inline const QString &rarName() const;
    inline const QString &rarPass() const;
    inline QString postSize() const;
    inline int nbOriginalFiles() const { return _nbInputFiles; }
    inline const QFileInfoList &originalFiles() const { return _inputFiles; }
    inline qint64 jobDurationMs() const { return _jobTimer.isValid() ? _jobTimer.elapsed() - _pauseDuration : 0; }

    inline bool hasCompressed() const;
    inline bool hasPacking() const;
    inline bool isPacked() const;
    inline bool hasPostStarted() const;
    inline bool hasPostFinished() const;
    inline bool hasPostFinishedSuccessfully() const;

    inline PostingWidget *widget() const;

    inline QString getFirstOriginalFile() const;

    inline void setDelFilesAfterPosted(bool delFiles);

    inline QString groups() const;
    inline QString from() const;

    inline bool isPosting() const;

    //! job that posts again the Articles listed in info (same nzb, groups, ...) and fixes the nzb
    static PostingJob *createRepostJob(NgPost *ngPost, const RepostInfo &info, PostingWidget *postWidget);
    inline bool isRepost() const { return _isRepost; }
    //! Articles that couldn't be posted (valid once the job is finished)
    inline const RepostInfo &repostInfo() const { return _repostInfo; }
    inline bool isRepostApplied() const { return _repostApplied; }

    // persistent queue
    inline const QString &queueId() const { return _queueId; }
    inline void setQueueId(const QString &id) { _queueId = id; }
    inline bool keepInQueue() const { return _keepInQueue; }
    QJsonObject toQueueEntry() const; //!< everything needed to post it again from the beginning
    //! job rebuilt from an entry of the queue (nullptr and an error if its files are not there anymore)
    static PostingJob *fromQueueEntry(NgPost *ngPost, const QJsonObject &entry, PostingWidget *postWidget, QString &error);

    inline bool isPaused() const;

    inline const QString &nzbFilePath() const;

    inline static QString humanSize(double size);


#ifdef __COMPUTE_IMMEDIATE_SPEED__
    inline const QString &immediateSpeed() const;
#endif

    static QString sslSupportInfo();
    static bool supportsSsl();

signals:
    void startPosting(bool isActiveJob);    //!< connected to onStartPosting (to be able to run on a different Thread)
    void stopPosting();

    void postingStarted();  //!< emitted at the end of onStartPosting
    void noMoreConnection();
    void postingFinished();

    void archiveFileNames(QStringList paths);
    void articlesNumber(uint nbArticles);

    void filePosted(QString filePath, uint nbArticles, uint nbFailed);

    void packingDone();


public slots:
    void onStopPosting(); //!< for HMI

private slots:
    void onStartPosting(bool isActiveJob);
    void onDisconnectedConnection(NntpConnection *con);

    void onNntpFileStartPosting();
    void onNntpFilePosted();
    void onNntpErrorReading();

    void onExtProcReadyReadStandardOutput();
    void onExtProcReadyReadStandardError();

    void onCompressionFinished(int exitCode);
    void onGenPar2Finished(int exitCode);

    void onResumeTriggered();

#ifdef __COMPUTE_IMMEDIATE_SPEED__
    void onImmediateSpeedComputation();
#endif


private:
    void _log(const QString &aMsg, bool newline = true) const; //!< log function for QString
    void _logAs(int level, const QString &aMsg) const;          //!< same with a color in the GUI (NgPost::LOG_LEVEL)
    void _error(const QString &error) const;

    int  _createNntpConnections();
    void _preparePostersArticles();

    NntpArticle *_readNextArticleIntoBufferPtr(const QString &threadName, char **bufferPtr);


    void _delOriginalFiles();

    inline NntpFile *_getNextFile();

    void _initPosting();
    void _prepareNfo();          //!< builds the NFO file (and adds it to the files to post if asked)
    void _saveNfoNextToNzb();
    bool _nfoHidesNames() const;
    void _postFiles();
    void _finishPosting();

    void _closeNzb();
    void _fillRepostMeta();                    //!< (end of a post) fills _repostInfo with the settings of this post
    void _collectRepostIds(NntpFile *nntpFile); //!< (repost job) keeps the new Message-IDs of a file
    void _applyRepost();                       //!< (repost job) fixes the nzb and cleans when everything is posted
    bool _patchNzb(const QVector<QPair<QString, QString>> &ids, QSet<QString> &fixed);
    bool _startSecondRoundIfNeeded();   //!< all the connections are done: post again the failed Articles (once)
    void _failRecoveryArticles();       //!< give up the Articles still waiting for a recovery
    void _restartConnectionsForSecondRound();
    void _printStats() const;    


    bool startCompressFiles(const QString &cmdRar,
                            const QString &tmpFolder,
                            const QString &archiveName,
                            const QString &pass,
                            uint volSize = 0);
    bool startGenPar2(const QString &tmpFolder,
                      const QString &archiveName,
                      uint redundancy = 0);

    bool _canCompress() const;
    bool _canGenPar2() const;

    void _cleanExtProc();
    void _cleanCompressDir();

    QString _createArchiveFolder(const QString &tmpFolder, const QString &archiveName);

    bool _checkTmpFolder() const;


    qint64 _dirSize(const QString &path);
    void   _diagRegisterPost(); //!< diagnostics: mask the names of the files and the password of this post

    inline QString timestamp() const;
};


QString PostingJob::avgSpeed() const
{
    QString power = " ";
    double bandwidth = 0.;

    if (_timeStart.isValid())
    {
        double sec = (_timeStart.elapsed()-_pauseDuration)/1000.;
        bandwidth = _uploadedSize / sec;

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
    }

    return QString("%1 %2B/s").arg(bandwidth, 6, 'f', 2).arg(power);
}

NntpFile *PostingJob::_getNextFile()
{
    if (_filesToUpload.size())
    {
        NntpFile *file = _filesToUpload.dequeue();
        _filesInProgress.insert(file);
        return file;
    }
    else
        return nullptr;
}

QString PostingJob::timestamp() const
{
    return QTime::currentTime().toString("hh:mm:ss.zzz");
}

void PostingJob::articlePosted(quint64 size)
{
    _uploadedSize += size;
    ++_nbArticlesUploaded;
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    if (_useHMI)
        _immediateSize += size;
#endif
}

void PostingJob::articleFailed(quint64 size)
{
    _uploadedSize += size;
    ++_nbArticlesUploaded;
    ++_nbArticlesFailed;
#ifdef __COMPUTE_IMMEDIATE_SPEED__
    if (_useHMI)
        _immediateSize += size;
#endif
}

uint PostingJob::nbArticlesTotal() const { return _nbArticlesTotal; }
uint PostingJob::nbArticlesUploaded() const { return _nbArticlesUploaded; }
uint PostingJob::nbArticlesFailed() const{ return _nbArticlesFailed; }
bool PostingJob::hasUploaded() const{ return _nbArticlesTotal > 0; }

const QString &PostingJob::nzbName() const { return _nzbName; }
const QString &PostingJob::rarName() const { return _rarName; }
const QString &PostingJob::rarPass() const { return _rarPass; }
QString PostingJob::postSize() const { return humanSize(static_cast<double>(_totalSize)); }

QString PostingJob::humanSize(double size)
{
    QString unit = "B";
    if (size > 1024)
    {
        size /= 1024;
        unit = "kB";
    }
    if (size > 1024)
    {
        size /= 1024;
        unit = "MB";
    }
    if (size > 1024)
    {
        size /= 1024;
        unit = "GB";
    }
    return QString("%1 %2").arg(size, 0, 'f', 2).arg(unit);
}

bool PostingJob::hasCompressed() const { return _doCompress; }
inline bool PostingJob::hasPacking() const { return _doCompress || _doPar2; }
bool PostingJob::isPacked() const { return _packed; }
bool PostingJob::hasPostStarted() const { return _postStarted; }
bool PostingJob::hasPostFinished() const { return _postFinished; }
bool PostingJob::hasPostFinishedSuccessfully() const { return _postFinished && !_nbArticlesFailed; }

PostingWidget *PostingJob::widget() const { return _postWidget; }

QString PostingJob::getFirstOriginalFile() const
{
    if (_originalFiles.isEmpty())
        return QString();
    else
        return _originalFiles.first().absoluteFilePath();
}

void PostingJob::setDelFilesAfterPosted(bool delFiles)
{
    _delFilesAfterPost = delFiles ? 0x1 : 0x0;
}

QString PostingJob::groups() const { return _grpList.join(","); }
QString PostingJob::from() const { return _obfuscateArticles ? QString() : QString::fromStdString(_from); }

bool PostingJob::isPosting() const
{
    return MB_LoadAtomic(_stopPosting) == 0x0;
}
bool PostingJob::isPaused() const { return _isPaused; }

const QString &PostingJob::nzbFilePath() const { return _nzbFilePath; }

#ifdef __COMPUTE_IMMEDIATE_SPEED__
const QString &PostingJob::immediateSpeed() const { return _immediateSpeed; }
#endif

#endif // POSTINGJOB_H
