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

#include "PostCheckDialog.h"
#include <QCoreApplication>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

static QString tr(const char *text) { return QCoreApplication::translate("PostingWidget", text); }

bool PostCheckDialog::run(QWidget *parent, const std::function<PostChecker::Input()> &makeInput,
                          const std::function<bool(PostChecker::Code)> &applyFix)
{
    for (int round = 0; round < 8; ++round) // a fix can show a new problem (compression on: is the rar program there?)
    {
        const QList<PostChecker::Issue> issues = PostChecker::check(makeInput());
        if (issues.isEmpty())
            return true;

        bool blocking = false;
        for (const PostChecker::Issue &issue : issues)
            blocking = blocking || issue.blocking;

        QDialog dlg(parent);
        dlg.setWindowTitle(tr("Before posting"));
        dlg.setMinimumWidth(520);
        QVBoxLayout *v = new QVBoxLayout(&dlg);
        v->setSpacing(10);
        QLabel *head = new QLabel(blocking ? tr("This post can't start yet:") : tr("Please check before posting:"), &dlg);
        head->setStyleSheet("font-weight: bold;");
        v->addWidget(head);
        for (int i = 0; i < issues.size(); ++i)
        {
            QHBoxLayout *h = new QHBoxLayout();
            QLabel *text = new QLabel(QString::fromUtf8(issues[i].blocking ? "⛔  " : "⚠  ") + issues[i].text, &dlg);
            text->setWordWrap(true);
            h->addWidget(text, 1);
            if (!issues[i].fixLabel.isEmpty())
            {
                QPushButton *fix = new QPushButton(issues[i].fixLabel, &dlg);
                QObject::connect(fix, &QAbstractButton::clicked, &dlg, [&dlg, i](){ dlg.done(2 + i); });
                h->addWidget(fix);
            }
            v->addLayout(h);
        }
        QHBoxLayout *buttons = new QHBoxLayout();
        buttons->addStretch();
        QPushButton *cancel = new QPushButton(QCoreApplication::translate("PostingWidget", "Cancel"), &dlg);
        QObject::connect(cancel, &QAbstractButton::clicked, &dlg, [&dlg](){ dlg.done(0); });
        buttons->addWidget(cancel);
        if (!blocking)
        {
            QPushButton *anyway = new QPushButton(QCoreApplication::translate("PostingWidget", "Post anyway"), &dlg);
            QObject::connect(anyway, &QAbstractButton::clicked, &dlg, [&dlg](){ dlg.done(1); });
            buttons->addWidget(anyway);
        }
        cancel->setDefault(true);
        v->addLayout(buttons);

        int res = dlg.exec();
        if (res == 1)
            return true;
        if (res >= 2)
        {
            if (!applyFix(issues[res - 2].code))
                return false;
            continue;
        }
        return false;
    }
    return false;
}
