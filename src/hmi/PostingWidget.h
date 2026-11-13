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

#ifndef POSTINGWIDGET_H
#define POSTINGWIDGET_H

#include <QWidget>
#include "utils/PostProfile.h"
#include "utils/PostChecker.h"
class NgPost;
class NntpFile;
class MainWindow;
class ProfileBar;
class NfoCard;
class PostingJob;
class QLabel;
class QPushButton;
#include <QFileInfoList>
#include "RepostInfo.h"
#include <QJsonObject>

namespace Ui {
class PostingWidget;
}

class PostingWidget : public QWidget
{
    Q_OBJECT

private:
    enum class STATE {IDLE, POSTING, STOPPING};

    Ui::PostingWidget *_ui;
    MainWindow        *_hmi;
    NgPost            *_ngPost;
    const uint         _jobNumber;
    PostingJob        *_postingJob;
    STATE              _state;
    bool               _postingFinished;

    // ngPost Deluxe layout (built in code over the widgets of the .ui)
    QLabel            *_filesTitle;
    QLabel            *_releaseInfo;   //!< shown when the list has several items (they make one release)
    QLabel            *_nzbTitle;
    QLabel            *_archiveTitle;
    QLabel            *_advancedTitle;
    QWidget           *_advancedCard;
    QLabel            *_summary;       //!< "12 items, 8.4 GB -> about ... to upload"
    qint64             _sourceBytes;   //!< size of the list (computed when the list changes)
    ProfileBar        *_profileBar;
    NfoCard           *_nfoCard;
    QPushButton       *_advancedToggle;
    QPushButton       *_repostButton;  //!< post again the Articles that failed (only after a post with errors)
    RepostInfo         _repostInfo;    //!< what is missing after the last post (kept even if the PostingJob is deleted)
    static bool        sShowAdvanced; //!< same state for all the Quick Post tabs
    void _buildLayout();
    void _retranslateLayout();
    void _showAdvanced(bool show);
    bool    _afterAct = false;
    QString _afterMoveTo;
    bool _checkBeforePosting(const QFileInfoList &files, bool hasFolder); //!< checks (and fixes) before a post started with the button
    bool _applyFix(PostChecker::Code code); //!< false: the user has to continue elsewhere (the post is cancelled)
    void _updateReleaseInfo();
    void _updateRepostButton();
    void _refreshFileItems(const RepostInfo &postedAgain);
    void _showJobStarted(bool hasStarted); //!< texts of the button and of the tab once the job is created

public:
    explicit PostingWidget(NgPost *ngPost, MainWindow *hmi, uint jobNumber);
    ~PostingWidget();

    void setIDLE();
    void setPosting();

    void init();
    void genNameAndPassword(bool genName, bool genPass, bool doPar2, bool useRarMax);
    //! what to do with the original files once posted (Auto Posting): nothing, delete them, or move them to \a moveTo
    void setAfterPost(bool act, const QString &moveTo) { _afterAct = act; _afterMoveTo = moveTo; }
    inline uint jobNumber() const;

    void handleDropEvent(QDropEvent *e);
    void handleKeyEvent(QKeyEvent *keyEvent);

    void addPath(const QString &path, int currentNbFiles, int isDir = false);

    inline bool isPosting() const;
    inline bool isPostingFinished() const;
    inline MainWindow *hmi() const;

    void udatePostingParams();

    void retranslate();

    void setNzbPassword(const QString &pass);
    void setPackingAuto(bool enabled, const QStringList &keys);

    void postFiles(bool updateMainParams);

    //! tab rebuilt from the persistent queue: a post posted again from the beginning (started at once)
    bool restoreFromQueue(const QJsonObject &entry, QString &error);
    //! tab rebuilt from the persistent queue: Articles that couldn't be posted (shows the repost button)
    void restoreMissing(const RepostInfo &info);
    void forgetQueue(); //!< the user closes the tab: its missing Articles are removed from the queue

    //! true if some Articles couldn't be posted and can still be posted again
    inline bool hasMissingArticles() const;
    inline int  nbMissingArticles() const;


signals:
    void postingDone(bool uploadedWithoutError); //!< emitted when the post is finished (used by the Auto Posting tab)

public slots: // for PostingJob
    void onFilePosted(QString filePath, uint nbArticles, uint nbFailed);
    void onArchiveFileNames(QStringList paths);
    void onArticlesNumber(int nbArticles);
    void onPostingJobDone();

    void onPostFiles(); //!< for the post button but also can be used by the AutoPostWidget
    void onRepostMissing(); //!< post again the Articles that failed and fix the nzb

private slots: // for the HMI

    void onNzbPassToggled(bool checked);
    void onGenNzbPassword();
    void _updateSummary(bool recomputeSize = true);
    PostProfile currentProfile() const;            //!< the options of the Archive card (to save them as a profile)
    void applyProfile(const PostProfile &profile); //!< a profile was chosen


    void onSelectFilesClicked();
    void onSelectFolderClicked();
    void onClearFilesClicked();
    void onCompressCB(bool checked);
    void onGenCompressName();
    void onCompressPathClicked();
    void onNzbFileClicked();
    void onRarPathClicked();


private:
    void _buildFilesList(QFileInfoList &files, bool &hasFolder);
    bool _fileAlreadyInList(const QString &fileName, int currentNbFiles) const;


};

uint PostingWidget::jobNumber() const { return _jobNumber; }
bool PostingWidget::isPosting() const { return _postingJob != nullptr; }
bool PostingWidget::hasMissingArticles() const { return !_repostInfo.isEmpty(); }
int  PostingWidget::nbMissingArticles() const { return _repostInfo.nbArticles(); }
bool PostingWidget::isPostingFinished() const { return _postingFinished; }

MainWindow *PostingWidget::hmi() const { return _hmi; }

#endif // POSTINGWIDGET_H
