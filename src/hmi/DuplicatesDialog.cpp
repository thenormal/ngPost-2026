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


#include "DuplicatesDialog.h"
#include "utils/NfoWriter.h"

#include <QCoreApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>

static QString T(const char *text) { return QCoreApplication::translate("AutoPostWidget", text); }

bool DuplicatesDialog::run(QWidget *parent, const QList<Item> &items, QSet<QString> &again)
{
    again.clear();
    if (items.isEmpty())
        return true;

    QDialog dlg(parent);
    dlg.setWindowTitle(T("Already posted"));
    dlg.setMinimumWidth(640);
    QVBoxLayout *v = new QVBoxLayout(&dlg);
    v->setSpacing(10);

    QLabel *intro = new QLabel(items.size() == 1 ? T("This item is already in the history. It is not posted again unless you check it.")
                                                  : T("These items are already in the history. They are not posted again unless you check them."), &dlg);
    intro->setWordWrap(true);
    v->addWidget(intro);

    QTableWidget *table = new QTableWidget(items.size(), 4, &dlg);
    table->setHorizontalHeaderLabels({T("Post again"), T("Name"), T("Size"), T("Posted on")});
    table->verticalHeader()->hide();
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    for (int i = 0; i < items.size(); ++i)
    {
        QTableWidgetItem *box = new QTableWidgetItem();
        box->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        box->setCheckState(Qt::Unchecked);
        table->setItem(i, 0, box);
        QTableWidgetItem *name = new QTableWidgetItem(items[i].name);
        name->setToolTip(items[i].path);
        table->setItem(i, 1, name);
        QTableWidgetItem *size = new QTableWidgetItem(NfoWriter::humanSize(items[i].size));
        size->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(i, 2, size);
        table->setItem(i, 3, new QTableWidgetItem(items[i].postedOn));
    }
    table->setMinimumHeight(qMin(300, 60 + 30 * items.size()));
    v->addWidget(table, 1);

    QHBoxLayout *row = new QHBoxLayout();
    QPushButton *all  = new QPushButton(T("Select all"), &dlg);
    QPushButton *none = new QPushButton(T("Select none"), &dlg);
    QObject::connect(all,  &QPushButton::clicked, &dlg, [table](){ for (int i = 0; i < table->rowCount(); ++i) table->item(i, 0)->setCheckState(Qt::Checked); });
    QObject::connect(none, &QPushButton::clicked, &dlg, [table](){ for (int i = 0; i < table->rowCount(); ++i) table->item(i, 0)->setCheckState(Qt::Unchecked); });
    row->addWidget(all);
    row->addWidget(none);
    row->addStretch();
    QPushButton *cancel = new QPushButton(T("Cancel"), &dlg);
    QPushButton *post   = new QPushButton(T("Post"), &dlg);
    post->setObjectName("accentButton");
    post->setDefault(true);
    QObject::connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(post,   &QPushButton::clicked, &dlg, &QDialog::accept);
    row->addWidget(cancel);
    row->addWidget(post);
    v->addLayout(row);

    if (dlg.exec() != QDialog::Accepted)
        return false;
    for (int i = 0; i < items.size(); ++i)
        if (table->item(i, 0)->checkState() == Qt::Checked)
            again.insert(items[i].path);
    return true;
}
