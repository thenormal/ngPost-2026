#ifndef PROGRESSINFO_H
#define PROGRESSINFO_H

#include <QString>

//! what the progress bar (GUI) and the progress line (command line) show about the active post
struct ProgressInfo
{
    enum Phase {IDLE = 0, COMPRESSING, PAR2, UPLOADING, DONE, ABORTED};

    Phase   phase       = IDLE;
    bool    paused      = false;
    uint    nbTotal     = 0;   //!< Articles of the post
    uint    nbUploaded  = 0;   //!< Articles done (the failed ones included)
    uint    nbFailed    = 0;
    quint64 sizeDone    = 0;   //!< bytes
    quint64 sizeTotal   = 0;
    qint64  etaSec      = -1;  //!< estimated remaining time (-1: not known yet)
    QString avgSpeed;
    QString immediateSpeed;
};

#endif // PROGRESSINFO_H
