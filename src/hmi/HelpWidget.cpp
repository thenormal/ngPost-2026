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

#include "HelpWidget.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextBrowser>
#include <QScrollBar>
#include <QFileInfo>
#include <QTextDocument>
#include <QUrl>

HelpWidget::HelpWidget(QWidget *parent) :
    QWidget(parent),
    _lang("en"),
    _list(new QListWidget(this)),
    _browser(new QTextBrowser(this))
{
    _list->setObjectName("helpList");
    _list->setFixedWidth(230);
    _list->setFrameShape(QFrame::NoFrame);
    _browser->setObjectName("helpBrowser");
    _browser->setOpenLinks(false);
    _browser->document()->setDefaultStyleSheet(
        "h1 {margin-bottom: 8px;} h2 {margin-top: 18px;} code {font-family: monospace;} "
        "pre {margin-left: 12px;}");

    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(24, 18, 24, 12);
    l->setSpacing(16);
    l->addWidget(_list);
    l->addWidget(_browser, 1);

    connect(_list,    &QListWidget::currentRowChanged, this, &HelpWidget::onRowChanged);
    connect(_browser, &QTextBrowser::anchorClicked,    this, &HelpWidget::onAnchorClicked);

    setLanguage(_lang);
}

QStringList HelpWidget::pageFiles()
{
    return QDir(":/help/en").entryList({"*.md"}, QDir::Files, QDir::Name);
}

QString HelpWidget::pagePath(const QString &file, const QString &lang)
{
    QString path = QString(":/help/%1/%2").arg(lang, file);
    if (QFile::exists(path))
        return path;
    path = QString(":/help/en/%1").arg(file);
    return QFile::exists(path) ? path : QString();
}

QString HelpWidget::pageTitle(const QString &path)
{
    QFile f(path);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!f.atEnd())
        {
            QString line = QString::fromUtf8(f.readLine()).trimmed();
            if (line.startsWith("# "))
                return line.mid(2).trimmed();
        }
    }
    return QFileInfo(path).completeBaseName();
}

void HelpWidget::setLanguage(const QString &lang)
{
    _lang = lang;
    QString current = _list->currentRow() >= 0 && _list->currentRow() < _files.size() ? _files[_list->currentRow()] : QString();
    _files = pageFiles();
    _list->blockSignals(true);
    _list->clear();
    for (const QString &file : _files)
        _list->addItem(pageTitle(pagePath(file, _lang)));
    _list->blockSignals(false);
    showPage(current.isEmpty() && !_files.isEmpty() ? _files.first() : current);
}

void HelpWidget::showPage(const QString &file)
{
    int row = _files.indexOf(file);
    if (row < 0)
        return;
    _list->blockSignals(true);
    _list->setCurrentRow(row);
    _list->blockSignals(false);
    QFile f(pagePath(file, _lang));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        _browser->setMarkdown(QString::fromUtf8(f.readAll()));
    else
        _browser->clear();
    _browser->verticalScrollBar()->setValue(0);
}

void HelpWidget::onRowChanged(int row)
{
    if (row >= 0 && row < _files.size())
        showPage(_files[row]);
}

void HelpWidget::onAnchorClicked(const QUrl &url)
{
    // link between pages: [text](02-servers.md)
    QString target = url.toString();
    if (target.endsWith(".md") && _files.contains(target))
        showPage(target);
    else if (url.scheme() == "http" || url.scheme() == "https")
        QDesktopServices::openUrl(url);
}
