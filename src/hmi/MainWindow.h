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
#include <QMap>
class QTimer;
class ServersWidget;
class NgPost;
struct NntpServerParams;
class NntpFile;
class PostingWidget;
class AutoPostWidget;
class QStackedWidget;
class QButtonGroup;
class QPushButton;
class QLabel;
class QComboBox;
class QGridLayout;
class QFrame;
class QLineEdit;
class QCheckBox;
class HistoryWidget;

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
    ServersWidget  *_servers; //!< server section (list + details)

    // autosave: the settings are saved automatically shortly after each change made by the user
    QTimer         *_autoSaveTimer;
    bool            _autoSaveReady;     //!< false during the initialisation (the widgets are filled by the code)
    bool            _autoSaveQuickTab;  //!< a setting of the default Quick Post tab changed
    bool            _autoSaveAutoTab;   //!< a setting of the Auto Posting tab changed
    static constexpr int sAutoSaveDelayMs = 1000;

    // new layout of ngPost Deluxe: side bar + pages
    enum Page {PAGE_POSTS = 0, PAGE_SERVERS, PAGE_SETTINGS, PAGE_HISTORY};
    enum Nav  {NAV_QUICK = 0, NAV_AUTO, NAV_SERVERS, NAV_SETTINGS, NAV_HISTORY, NB_NAV};
    QStackedWidget *_pages;
    QButtonGroup   *_nav;
    QPushButton    *_navButtons[NB_NAV];
    HistoryWidget  *_history;
    int             _jobNumber;   //!< post shown in the bottom bar (-1: none)
    bool            _pauseIcon;   //!< the pause button shows pause (true) or play
    QCheckBox      *_historyCB;
    QLineEdit      *_historyPathEdit;
    QPushButton    *_historyPathButton;
    QLabel         *_historyHint;
    QLabel         *_supportHint;
    QPushButton    *_supportButton;
    QPushButton    *_themeButton;  //!< side bar: switch Auto / Light / Dark
    QPushButton    *_aboutButton;
    QLabel         *_postsTitle;
    QLabel         *_serversTitle;
    QLabel         *_serversSubtitle;
    QLabel         *_settingsTitle;
    QLabel         *_settingsSubtitle;
    QLabel         *_appVersion;
    QComboBox      *_themeCB;
    QLabel         *_themeLbl;
    QLabel         *_autoSaveHint;
    QMap<QString, QLabel*> _cardTitles;




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

    static QColor        sPostingColor;
    static const QString sPostingIcon;
    static QColor        sPendingColor;
    static const QString sPendingIcon;
    static QColor        sDoneOKColor;
    static const QString sDoneOKIcon;
    static QColor        sDoneKOColor;
    static const QString sDoneKOIcon;
    static QColor        sArticlesFailedColor;

    static void setThemeColors(bool dark); //!< status colours readable on the light or the dark theme
    void onThemeApplied(); //!< called by NgPost::applyTheme (icons of the side bar...)
    void onHistoryUpdated(); //!< a post has been added in the history file

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
    void onNavClicked(int id);
    void onPostTabChanged(int index);
    void onThemeSelected(int index);
    void onThemeButtonClicked();
    void onHistoryToggled(bool checked);
    void onHistoryPathClicked();
    void onAutoSave();
    void onDiagnosticReport();

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
    QString _diagnosticReport() const;

    void _buildShell();          //!< side bar + pages (replaces the splitters of the original window)
    QWidget *_buildSettingsPage();
    QFrame *_card(const QString &key, QGridLayout *&grid);
    void _retranslateShell();
    void _fixIconButtons();
    void _adaptIcons();
    void _adaptTabIcons();
    QColor _tabNormalColor() const;
    QIcon  _tabIcon(const QString &path) const;
    void _updateThemeButton();
    void _updateHistorySettings();
    static QIcon _navIcon(const QString &path, const QColor &normal, const QColor &checked);

    void _initServerBox();
    void _initPostingBox();

    PostingWidget *_getPostWidget(int tabIndex) const;
    int _getPostWidgetIndex(PostingWidget *postWidget) const;


};

AutoPostWidget *MainWindow::autoWidget() const { return _autoPostTab; }

#endif // MAINWINDOW_H
