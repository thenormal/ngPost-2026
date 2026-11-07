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

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <QString>
#include <QStringList>
#include <QMutex>
#include <QFile>
#include <QVector>
#include <QPair>

class FreezeWatchdog;

/*!
 * \brief Built-in diagnostics: records what ngPost does (phases with their duration and speed,
 * errors, GUI freezes...) in memory and in a file next to the program (ngPost_diagnostics.log),
 * so that a report can be generated after a problem, even after a crash
 * (the file of the previous session is kept as ngPost_diagnostics.previous.log).
 *
 * Everything that is recorded is masked first: archive passwords, server credentials,
 * names and paths of the posted files, home folder (cf addSecret()).
 * Nothing is sent anywhere: the report is only saved where the user chooses.
 */
class Diagnostics
{
public:
    static Diagnostics &instance();

    //! open the log file (the previous one is kept as .previous.log) and write the session header
    void start(const QString &header);
    //! write the end of session marker (used to detect a crash at the next start)
    void stop();

    void event(const QString &category, const QString &msg);
    void anomaly(const QString &msg); //!< event that will be listed at the top of the report

    //! every future occurrence of secret will be replaced by label (longest secrets are replaced first)
    void addSecret(const QString &secret, const QString &label);
    QString mask(const QString &str) const;
    QString nextInputLabel(); //!< <input 1>, <input 2>...

    QStringList events() const;
    QStringList anomalies() const;
    QString previousSessionLog(bool *closedProperly) const;
    QString logFilePath() const;

    void startFreezeWatchdog(); //!< GUI only: detects when the interface doesn't respond
    void stopFreezeWatchdog();

    static QString humanSize(double bytes);
    static QString speed(double bytes, qint64 ms); //!< "12.3 MB/s"

    static constexpr const char *sEndOfSession = "=== end of session ===";

private:
    Diagnostics();
    ~Diagnostics();
    Diagnostics(const Diagnostics &) = delete;
    Diagnostics &operator=(const Diagnostics &) = delete;

    QString _maskNoLock(const QString &str) const;
    void    _writeNoLock(const QString &line);

    mutable QMutex                   _mutex;
    QFile                            _file;
    QString                          _filePath;
    QString                          _previousPath;
    QStringList                      _events;    //!< last events of the session (limited)
    QStringList                      _anomalies;
    QVector<QPair<QString, QString>> _secrets;   //!< (secret, label) sorted by decreasing length
    int                              _nbInputs;
    FreezeWatchdog                  *_watchdog;

    static constexpr int sMaxEvents = 2000;
};

#endif // DIAGNOSTICS_H
