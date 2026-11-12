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

#ifndef FEEDBACKDIALOG_H
#define FEEDBACKDIALOG_H

#include <QDialog>
#include <QString>
#include <QUrl>

/*!
 * \brief Feedback window (opened from About): report a problem or suggest an idea.
 * It opens the new-issue page of GitHub in the browser, already filled with the version and the system.
 * Nothing is sent by the program and it holds no access key: the user sees the page and sends it himself.
 */
class FeedbackDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Kind { BUG, IDEA };

    explicit FeedbackDialog(QWidget *parent = nullptr);

    //! address of the new-issue page for this kind (the ids of the fields are those of .github/ISSUE_TEMPLATE)
    static QUrl issueUrl(Kind kind, const QString &version, const QString &system);

signals:
    void saveReportRequested();
};

#endif // FEEDBACKDIALOG_H
