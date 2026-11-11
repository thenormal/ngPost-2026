//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
// ngPost Deluxe fork: https://github.com/thenormal/ngPost-Deluxe
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

#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include <QWidget>
#include <QStringList>

class QListWidget;
class QTextBrowser;
class QUrl;

//! Help page: offline Markdown pages embedded in the program (resources :/help/<lang>/*.md).
//! The pages are listed from the English folder; for each page the file of the current language
//! is used when it exists, otherwise the English one.
class HelpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HelpWidget(QWidget *parent = nullptr);

    //! reload the list and the current page for this language (e.g. "en", "it")
    void setLanguage(const QString &lang);
    //! the files (e.g. "01-first-steps.md") of the English pages, sorted
    static QStringList pageFiles();
    //! resource path of a page for a language, with the fallback on English ("" if unknown)
    static QString pagePath(const QString &file, const QString &lang);
    //! title of a page: its first "# " heading (the file name if there is none)
    static QString pageTitle(const QString &path);

    void showPage(const QString &file);

private slots:
    void onRowChanged(int row);
    void onAnchorClicked(const QUrl &url);

private:
    QString       _lang;
    QListWidget  *_list;
    QTextBrowser *_browser;
    QStringList   _files;
};

#endif // HELPWIDGET_H
