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

#include "AboutNgPost.h"
#include "ui_AboutNgPost.h"
#include "NgPost.h"
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QPushButton>
#include <QUrl>

AboutNgPost::AboutNgPost(NgPost *ngPost, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutNgPost)
{
    ui->setupUi(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    setWindowFlag(Qt::FramelessWindowHint);
#endif
    setStyleSheet("QDialog {border:1px solid #c4c4c4}");

    const QString repo     = "https://github.com/thenormal/ngPost-Deluxe";
    const QString original = "https://github.com/mbruel/ngPost";

    ui->titleLbl->setText(QString("<span style='font-size:26pt; font-weight:600;'>%1</span>").arg(ngPost->escapeXML(ngPost->appName())));
    ui->versionLbl->setText(QString("<span style='font-size:11pt; color:#6b6b6b;'>%1</span>").arg(tr("Version %1").arg(NgPost::versionLabel())));

    ui->descLbl->setText(tr("A fast and safe Usenet poster with a graphical and a command line interface: "
                            "it compresses (RAR / 7z), generates the par2 and posts your files, "
                            "folders or whole monitored directories, then writes the nzb."));

    ui->linksLbl->setTextFormat(Qt::RichText);
    ui->linksLbl->setOpenExternalLinks(true);
    ui->linksLbl->setText(QString("<a href=\"%1\">%2</a> &nbsp;&middot;&nbsp; <a href=\"%3\">%4</a>")
                          .arg(repo, tr("Project page"), original, tr("Original project")));

    // support of the project: Ko-fi page opened in the browser
    QPushButton *supportButton = new QPushButton(tr("Support on Ko-fi"), this);
    supportButton->setObjectName("accentButton");
    supportButton->setCursor(Qt::PointingHandCursor);
    supportButton->setToolTip(NgPost::supportUrl());
    connect(supportButton, &QAbstractButton::clicked, this, [](){ QDesktopServices::openUrl(QUrl(NgPost::supportUrl())); });
    QHBoxLayout *supportLayout = new QHBoxLayout();
    supportLayout->addStretch();
    supportLayout->addWidget(supportButton);
    supportLayout->addStretch();
    ui->mainLayout->insertLayout(ui->mainLayout->indexOf(ui->linksLbl) + 1, supportLayout);

    ui->copyrightLbl->setText(QString("%1<br/>%2<br/>Copyright &copy; 2020 - Matthieu Bruel<br/>%3")
                              .arg(tr("Based on ngPost by %1").arg("Matthieu Bruel"),
                                   tr("Released under the GPL v3 licence."),
                                   tr("Fork maintained by %1").arg("thenormal")));
    ui->copyrightLbl->setStyleSheet("QLabel { color : #6b6b6b; }");

    connect(ui->closeButton, &QAbstractButton::clicked, this, &QWidget::close);
}

AboutNgPost::~AboutNgPost()
{
    delete ui;
}

void AboutNgPost::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e)
    close();
}

#include <QMouseEvent>
void AboutNgPost::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    if (e->button() == Qt::RightButton)
        close();
}
