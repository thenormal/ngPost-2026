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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfoList>
class QTimer;
class ServersWidget;
class NgPost;
struct NntpServerParams;
class NntpFile;
class PostingWidget;
class AutoPostWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    enum class STATE {IDLE, POSTING, STOPPING};

    Ui::MainWindow *_ui;
    NgPost         *_ngPost;
    STATE           _state;
    PostingWidget  *_quickJobTab;
    AutoPostWidget *_autoPostTab;

    // autosave: the settings are saved automatically shortly after each change made by the user
    QTimer         *_autoSaveTimer;
    bool            _autoSaveReady;     //!< false during the initialisation (the widgets are filled by the code)
    bool            _autoSaveQuickTab;  //!< a setting of the default Quick Post tab changed
    bool            _autoSaveAutoTab;   //!< a setting of the Auto Posting tab changed
    static constexpr int sAutoSaveDelayMs = 1000;

    ServersWidget  *_servers; //!< server section (list + details)



public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void init(NgPost *ngPost);

    void updateProgressBar(uint nbArticlesTotal, uint nbArticlesUploaded, const QString &avgSpeed = "0 B/s"
                       #ifdef __COMPUTE_IMMEDIATE_SPEED__
                           , const QString &immediateSpeed = "0 B/s"
                       #endif
                           );

    void updateServers();
    void updateParams();
    void updateAutoPostingParams();

    QString fixedArchivePassword() const;

    PostingWidget *addNewQuickTab(int lastTabIdx, const QFileInfoList &files = QFileInfoList());

    void setTab(QWidget *postWidget);
    void clearJobTab(QWidget *postWidget);
    void updateJobTab(QWidget *postWidget, const QColor &color, const QIcon &icon, const QString &tooltip = "");

    void setJobLabel(int jobNumber);

    void log(const QString &aMsg, bool newline = true) const; //!< log function for QString
    void logError(const QString &error) const; //!< log function for QString

    bool useFixedPassword() const;
    bool hasAutoCompress() const;


    bool hasFinishedPosts() const;

    inline AutoPostWidget *autoWidget() const;
    void closeTab(PostingWidget *postWidget);

    void setPauseIcon(bool pause);

    static const QColor  sPostingColor;
    static const QString sPostingIcon;
    static const QColor  sPendingColor;
    static const QString sPendingIcon;
    static const QColor  sDoneOKColor;
    static const QString sDoneOKIcon;
    static const QColor  sDoneKOColor;
    static const QString sDoneKOIcon;
    static const QColor  sArticlesFailedColor;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *e) override;

    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent* event) override;

public slots:
    void onSetProgressBarRange(int nbArticles);
    void onCloseAllFinishedQuickTabs();


public slots:
    void scheduleAutoSave(); //!< to be called after any change of a setting (the save is delayed and grouped)

private slots:
    void onAutoSave();

    void onObfucateToggled(bool checked);

    void onTabContextMenu(const QPoint &point);



    void onGenPoster();
    void onUniqueFromToggled(bool checked);
    void onRarPassToggled(bool checked);
    void onRarPassUpdated(const QString &fixedPass);
    void onArchivePass();
    void onAutoCompressToggled(bool checked);

    void onDebugToggled(bool checked);
    void onDebugValue(int value);

    void onSaveConfig();

    void onJobTabClicked(int index);
    void onCloseJob(int index);

    void toBeImplemented();

    void onNzbPathClicked();

    void onLangChanged(const QString &lang);

    void onShutdownToggled(bool checked);

    void onPauseClicked();

private:
    void _watchForAutoSave(QWidget *root, int tab); //!< tab: -1 global settings, 0 Quick Post, 1 Auto Posting
    void _saveConfig(bool quickTab, bool autoTab);

    void _initServerBox();
    void _initPostingBox();

    PostingWidget *_getPostWidget(int tabIndex) const;
    int _getPostWidgetIndex(PostingWidget *postWidget) const;


};

AutoPostWidget *MainWindow::autoWidget() const { return _autoPostTab; }

#endif // MAINWINDOW_H
