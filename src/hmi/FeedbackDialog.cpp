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

#include "FeedbackDialog.h"
#include "NgPost.h"
#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QSysInfo>
#include <QUrlQuery>
#include <QVBoxLayout>

static const char *sIssuesUrl = "https://github.com/thenormal/ngPost-Deluxe/issues/new";

QUrl FeedbackDialog::issueUrl(Kind kind, const QString &version, const QString &system)
{
    QUrlQuery query;
    query.addQueryItem("template", kind == Kind::BUG ? "bug_report.yml" : "feature_request.yml");
    query.addQueryItem("version", version);
    query.addQueryItem("system", system);
    QUrl url(sIssuesUrl);
    url.setQuery(query);
    return url;
}

FeedbackDialog::FeedbackDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Feedback"));
    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *intro = new QLabel(tr("Found a problem or have an idea? The page of GitHub opens in your browser, "
                                  "already filled with the version and the system. Nothing is sent by the program: "
                                  "you check the page and send it yourself (a free GitHub account is needed)."), this);
    intro->setWordWrap(true);
    layout->addWidget(intro);

    const QString version = NgPost::versionLabel();
    const QString system  = QString("%1, Qt %2").arg(QSysInfo::prettyProductName(), qVersion());

    QPushButton *bugButton  = new QPushButton(tr("Report a problem"), this);
    QPushButton *ideaButton = new QPushButton(tr("Suggest an idea"), this);
    bugButton->setObjectName("accentButton");
    bugButton->setCursor(Qt::PointingHandCursor);
    ideaButton->setCursor(Qt::PointingHandCursor);
    connect(bugButton,  &QAbstractButton::clicked, this, [=](){ QDesktopServices::openUrl(issueUrl(Kind::BUG,  version, system)); });
    connect(ideaButton, &QAbstractButton::clicked, this, [=](){ QDesktopServices::openUrl(issueUrl(Kind::IDEA, version, system)); });
    layout->addWidget(bugButton);
    layout->addWidget(ideaButton);

    QLabel *reportHint = new QLabel(tr("For a problem, save the diagnostic report and attach it to the page. "
                                       "Passwords, usernames and file names are masked in it."), this);
    reportHint->setWordWrap(true);
    layout->addWidget(reportHint);
    QPushButton *reportButton = new QPushButton(tr("Save the diagnostic report"), this);
    reportButton->setCursor(Qt::PointingHandCursor);
    connect(reportButton, &QAbstractButton::clicked, this, &FeedbackDialog::saveReportRequested);
    layout->addWidget(reportButton);

    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
    setMinimumWidth(440);
}
