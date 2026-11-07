//========================================================================
//
// ngPost 2026 fork: https://github.com/thenormal/ngPost-2026
// (ngPost: Copyright (C) 2020 Matthieu Bruel, https://github.com/mbruel/ngPost)
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

#include "Diagnostics.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <atomic>

//========================================================================
// FreezeWatchdog: a thread that checks that the GUI thread is still alive
//========================================================================
class FreezeWatchdog : public QThread
{
public:
    explicit FreezeWatchdog(QObject *heartbeatParent) :
        QThread(nullptr), _lastBeat(QDateTime::currentMSecsSinceEpoch()), _stop(false)
    {
        // the heartbeat is updated by a timer of the GUI thread
        _heartbeat = new QTimer(heartbeatParent);
        _heartbeat->setInterval(500);
        QObject::connect(_heartbeat, &QTimer::timeout, _heartbeat, [this](){ _lastBeat = QDateTime::currentMSecsSinceEpoch(); });
        _heartbeat->start();
    }
    ~FreezeWatchdog() override
    {
        _stop = true;
        wait();
    }
    void stopHeartbeat() { _heartbeat->stop(); }

protected:
    void run() override
    {
        bool    frozen = false, reported10s = false;
        qint64  freezeStart = 0;
        while (!_stop)
        {
            QThread::msleep(500);
            qint64 now = QDateTime::currentMSecsSinceEpoch(), last = _lastBeat;
            qint64 delay = now - last;
            if (!frozen && delay > sFreezeMs)
            {
                frozen = true;
                reported10s = false;
                freezeStart = last;
            }
            else if (frozen && delay <= sFreezeMs)
            {
                frozen = false;
                Diagnostics::instance().anomaly(QString("GUI not responding during %1 s").arg(
                                                    (last - freezeStart) / 1000.0, 0, 'f', 1));
            }
            else if (frozen && !reported10s && now - freezeStart > 10000)
            {   // still frozen: write it now in case the user kills the program
                reported10s = true;
                Diagnostics::instance().anomaly("GUI not responding for more than 10 s (still frozen)");
            }
        }
    }

private:
    QTimer              *_heartbeat;
    std::atomic<qint64>  _lastBeat;
    std::atomic<bool>    _stop;
    static constexpr qint64 sFreezeMs = 3000;
};

//========================================================================
// Diagnostics
//========================================================================
Diagnostics &Diagnostics::instance()
{
    static Diagnostics diag;
    return diag;
}

Diagnostics::Diagnostics() :
    _mutex(), _file(), _filePath(), _previousPath(),
    _events(), _anomalies(), _secrets(), _nbInputs(0), _watchdog(nullptr)
{}

Diagnostics::~Diagnostics()
{
    // the heartbeat timer may already be deleted with the application: only stop the thread
    delete _watchdog;
}

void Diagnostics::start(const QString &header)
{
    // next to the program if possible (that's where Windows users look), otherwise in the user data folder
    QString dir = QCoreApplication::applicationDirPath();
#if !defined(Q_OS_WIN)
    dir = QDir::homePath();
#endif
    if (!QFileInfo(dir).isWritable())
        dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
#if defined(Q_OS_WIN)
    QString prefix = "ngPost_diagnostics";
#else
    QString prefix = ".ngPost_diagnostics";
#endif

    addSecret(QDir::homePath(), "<home>");
    addSecret(QDir::toNativeSeparators(QDir::homePath()), "<home>");

    QMutexLocker lock(&_mutex);
    _filePath     = QString("%1/%2.log").arg(dir, prefix);
    _previousPath = QString("%1/%2.previous.log").arg(dir, prefix);
    if (QFileInfo::exists(_filePath))
    {
        QFile::remove(_previousPath);
        QFile::rename(_filePath, _previousPath);
    }
    _file.setFileName(_filePath);
    _file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    lock.unlock();

    event("SESSION", header);
}

void Diagnostics::stop()
{
    stopFreezeWatchdog();
    event("SESSION", sEndOfSession);
    QMutexLocker lock(&_mutex);
    _file.close();
}

void Diagnostics::event(const QString &category, const QString &msg)
{
    QMutexLocker lock(&_mutex);
    QString line = QString("%1 [%2] %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"),
                                             category, _maskNoLock(msg.trimmed()));
    _events << line;
    if (_events.size() > sMaxEvents)
        _events.removeFirst();
    _writeNoLock(line);
}

void Diagnostics::anomaly(const QString &msg)
{
    event("ANOMALY", msg);
    QMutexLocker lock(&_mutex);
    _anomalies << QString("%1 %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss"), _maskNoLock(msg));
}

void Diagnostics::addSecret(const QString &secret, const QString &label)
{
    if (secret.size() < 3) // too short, we would mask everything
        return;
    QMutexLocker lock(&_mutex);
    for (const auto &s : _secrets)
        if (s.first == secret)
            return;
    _secrets << qMakePair(secret, label);
    std::sort(_secrets.begin(), _secrets.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b){
        return a.first.size() > b.first.size();
    });
}

QString Diagnostics::mask(const QString &str) const
{
    QMutexLocker lock(&_mutex);
    return _maskNoLock(str);
}

QString Diagnostics::nextInputLabel()
{
    QMutexLocker lock(&_mutex);
    return QString("<input %1>").arg(++_nbInputs);
}

QStringList Diagnostics::events() const
{
    QMutexLocker lock(&_mutex);
    return _events;
}

QStringList Diagnostics::anomalies() const
{
    QMutexLocker lock(&_mutex);
    return _anomalies;
}

QString Diagnostics::previousSessionLog(bool *closedProperly) const
{
    QString path;
    {
        QMutexLocker lock(&_mutex);
        path = _previousPath;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (closedProperly)
            *closedProperly = true;
        return QString();
    }
    QString content = QString::fromUtf8(file.readAll());
    if (closedProperly)
        *closedProperly = content.contains(sEndOfSession);
    return content;
}

QString Diagnostics::logFilePath() const
{
    QMutexLocker lock(&_mutex);
    return _filePath;
}

void Diagnostics::startFreezeWatchdog()
{
    if (_watchdog)
        return;
    _watchdog = new FreezeWatchdog(QCoreApplication::instance());
    _watchdog->start(QThread::LowPriority);
}

void Diagnostics::stopFreezeWatchdog()
{
    if (_watchdog)
    {
        _watchdog->stopHeartbeat();
        delete _watchdog;
        _watchdog = nullptr;
    }
}

QString Diagnostics::humanSize(double bytes)
{
    QString unit = "B";
    if (bytes > 1024) { bytes /= 1024; unit = "kB"; }
    if (bytes > 1024) { bytes /= 1024; unit = "MB"; }
    if (bytes > 1024) { bytes /= 1024; unit = "GB"; }
    return QString("%1 %2").arg(bytes, 0, 'f', 2).arg(unit);
}

QString Diagnostics::speed(double bytes, qint64 ms)
{
    if (ms <= 0)
        ms = 1;
    return QString("%1/s").arg(humanSize(bytes * 1000. / ms));
}

QString Diagnostics::_maskNoLock(const QString &str) const
{
    QString res = str;
    for (const auto &s : _secrets)
        res.replace(s.first, s.second, Qt::CaseInsensitive);
    return res;
}

void Diagnostics::_writeNoLock(const QString &line)
{
    if (_file.isOpen())
    {
        _file.write(line.toUtf8());
        _file.write("\n");
        _file.flush(); // always flushed: the file must be complete even after a crash
    }
}
