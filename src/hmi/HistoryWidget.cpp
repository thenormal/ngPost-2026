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

#include "HistoryWidget.h"
#include "NgPost.h"
#include "utils/Diagnostics.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QSaveFile>
#include <QMessageBox>
#include <QRegularExpression>

static const int sSortRole  = Qt::UserRole + 1;
static const int sEntryRole = Qt::UserRole + 2;
static const QString sHiddenPass = QString(8, QChar(0x2022)); // ••••••••

//! table item sorted on a value (date, bytes, speed...) instead of its text
class SortItem : public QTableWidgetItem
{
public:
    SortItem(const QString &text, const QVariant &sortValue) : QTableWidgetItem(text)
    {
        setData(sSortRole, sortValue);
        setFlags(flags() & ~Qt::ItemIsEditable);
    }
    bool operator<(const QTableWidgetItem &other) const override
    {
        QVariant a = data(sSortRole), b = other.data(sSortRole);
        if (a.type() == QVariant::DateTime)
            return a.toDateTime() < b.toDateTime();
        if (a.type() == QVariant::String)
            return QString::localeAwareCompare(a.toString(), b.toString()) < 0;
        return a.toDouble() < b.toDouble();
    }
};

HistoryWidget::HistoryWidget(NgPost *ngPost, QWidget *parent) :
    QWidget(parent),
    _ngPost(ngPost),
    _entries(),
    _showPasswords(false)
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 18, 24, 18);
    root->setSpacing(4);

    _title = new QLabel(this);
    _title->setObjectName("pageTitle");
    _subtitle = new QLabel(this);
    _subtitle->setObjectName("pageSubtitle");
    _subtitle->setTextInteractionFlags(Qt::TextSelectableByMouse);
    root->addWidget(_title);
    root->addWidget(_subtitle);
    root->addSpacing(8);

    _warning = new QLabel(this);
    _warning->setObjectName("infoBanner");
    _warning->setWordWrap(true);
    root->addWidget(_warning);
    root->addSpacing(8);

    // tool bar: search, period, passwords, export, open the file
    QHBoxLayout *bar = new QHBoxLayout();
    bar->setSpacing(8);
    _search = new QLineEdit(this);
    _search->setClearButtonEnabled(true);
    _search->setMinimumWidth(220);
    _period = new QComboBox(this);
    _period->addItems({QString(), QString(), QString(), QString()});
    _passButton = new QPushButton(this);
    _passButton->setCheckable(true);
    _passButton->setIcon(QIcon(":/icons/eye.png"));
    _exportButton = new QPushButton(this);
    _openButton   = new QPushButton(this);
    bar->addWidget(_search, 1);
    bar->addWidget(_period);
    bar->addStretch();
    bar->addWidget(_passButton);
    bar->addWidget(_exportButton);
    bar->addWidget(_openButton);
    root->addLayout(bar);
    root->addSpacing(6);

    _summary = new QLabel(this);
    _summary->setObjectName("cardTitle");
    root->addWidget(_summary);
    root->addSpacing(4);

    _table = new QTableWidget(0, NB_COLUMNS, this);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _table->setAlternatingRowColors(true);
    _table->setShowGrid(false);
    _table->setWordWrap(false);
    _table->verticalHeader()->hide();
    _table->verticalHeader()->setDefaultSectionSize(30);
    _table->horizontalHeader()->setHighlightSections(false);
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->setContextMenuPolicy(Qt::CustomContextMenu);
    _table->setSortingEnabled(true);
    _table->sortByColumn(COL_DATE, Qt::DescendingOrder);
    root->addWidget(_table, 1);

    _emptyLbl = new QLabel(_table->viewport());
    _emptyLbl->setObjectName("hint");
    _emptyLbl->setAlignment(Qt::AlignCenter);
    _emptyLbl->setWordWrap(true);

    connect(_search, &QLineEdit::textChanged, this, &HistoryWidget::onFilter);
    connect(_period, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HistoryWidget::onFilter);
    connect(_passButton,   &QAbstractButton::toggled, this, &HistoryWidget::onShowPasswords);
    connect(_exportButton, &QAbstractButton::clicked, this, &HistoryWidget::onExport);
    connect(_openButton,   &QAbstractButton::clicked, this, &HistoryWidget::onOpenFile);
    connect(_table, &QWidget::customContextMenuRequested, this, &HistoryWidget::onContextMenu);
    connect(_table, &QTableWidget::cellDoubleClicked, this, [this](int row, int col){
        if (col == COL_PASS)
            if (const Entry *e = _entryAt(row))
                QApplication::clipboard()->setText(e->pass);
    });

    retranslate();
}

void HistoryWidget::retranslate()
{
    _title->setText(tr("History"));
    _warning->setText(tr("<b>Keep this file private:</b> the history also records the <b>password of each archive</b> "
                         "in clear text, so you can always open your releases again. It stays on this computer "
                         "and is never sent anywhere, but anyone using this computer can read it. "
                         "You can turn the history off in the Settings."));
    _search->setPlaceholderText(tr("Search a name, an archive, a group..."));
    _period->setItemText(0, tr("All"));
    _period->setItemText(1, tr("Today"));
    _period->setItemText(2, tr("Last 7 days"));
    _period->setItemText(3, tr("Last 30 days"));
    _passButton->setText(_showPasswords ? tr("Hide passwords") : tr("Show passwords"));
    _exportButton->setText(tr("Export..."));
    _exportButton->setToolTip(tr("Save the uploads shown in the list in a CSV file"));
    _openButton->setText(tr("Open the file"));
    _openButton->setToolTip(tr("Open the history file with the default program (Excel...)"));
    _table->setHorizontalHeaderLabels({tr("Date"), tr("Name"), tr("Size"), tr("Speed"), tr("Status"), tr("Files"),
                                       tr("Duration"), tr("Archive"), tr("Password"), tr("Groups"), tr("Poster")});
    _table->setToolTip(tr("Right click for more actions, double click on a password to copy it"));
    reload();
}

double HistoryWidget::parseSize(const QString &str)
{
    static const QRegularExpression re("([0-9]+(?:[.,][0-9]+)?)\\s*([kKMGT]?)B", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch m = re.match(str);
    if (!m.hasMatch())
        return 0;
    double val = m.captured(1).replace(',', '.').toDouble();
    QString unit = m.captured(2).toUpper();
    if (unit == "K") val *= 1024.;
    else if (unit == "M") val *= 1024.*1024.;
    else if (unit == "G") val *= 1024.*1024.*1024.;
    else if (unit == "T") val *= 1024.*1024.*1024.*1024.;
    return val;
}

double HistoryWidget::parseSpeed(const QString &str)
{
    return parseSize(str); // same format followed by "/s"
}

// the date format of the history changed over the versions:
// old ngPost lines use "dd/MM/yyyy hh:mm", the recent ones "yyyy/MM/dd hh:mm:ss"
static QDateTime parseDate(const QString &str)
{
    static const QStringList formats = {
        "yyyy/MM/dd hh:mm:ss", "yyyy/MM/dd hh:mm",
        "dd/MM/yyyy hh:mm:ss", "dd/MM/yyyy hh:mm",
        "yyyy-MM-dd hh:mm:ss", "yyyy-MM-dd hh:mm",
        "dd-MM-yyyy hh:mm:ss", "dd-MM-yyyy hh:mm",
        "dd.MM.yyyy hh:mm:ss", "dd.MM.yyyy hh:mm",
        "MM/dd/yyyy hh:mm:ss", "MM/dd/yyyy hh:mm"
    };
    for (const QString &fmt : formats)
    {
        QDateTime date = QDateTime::fromString(str, fmt);
        if (date.isValid())
            return date;
    }
    return QDateTime::fromString(str, Qt::ISODate);
}

void HistoryWidget::reload()
{
    _entries.clear();
    QString path = _ngPost->historyFile();
    if (path.isEmpty())
        _subtitle->setText(tr("The history is turned off (Settings)."));
    else
        _subtitle->setText(tr("File: %1").arg(QDir::toNativeSeparators(path)));
    _openButton->setEnabled(!path.isEmpty() && QFileInfo::exists(path));

    QFile file(path);
    if (!path.isEmpty() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString sep = _ngPost->historyFieldSeparator();
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        while (!stream.atEnd())
        {
            QString line = stream.readLine();
            if (line.trimmed().isEmpty())
                continue;
            QStringList f = line.split(sep);
            QDateTime date = parseDate(f.value(0).trimmed());
            if (!date.isValid())
                continue; // header line
            Entry e;
            e.date     = date;
            e.name     = f.value(1).trimmed();
            if (e.name.endsWith(".nzb", Qt::CaseInsensitive))
                e.name.chop(4);
            e.size     = f.value(2).trimmed();
            e.bytes    = parseSize(e.size);
            e.speed    = f.value(3).trimmed();
            e.speedBps = parseSpeed(e.speed);
            e.archive  = f.value(4).trimmed();
            e.pass     = f.value(5);
            e.groups   = f.value(6).trimmed();
            e.from     = f.value(7).trimmed();
            e.status   = f.value(8).trimmed();
            bool ok = false;
            int nb = f.value(9).trimmed().toInt(&ok);
            e.nbFiles  = ok ? nb : -1;
            e.duration = f.value(10).trimmed();
            QStringList hms = e.duration.split(':');
            if (hms.size() == 3)
                e.durationSec = hms[0].toLongLong() * 3600 + hms[1].toLongLong() * 60 + hms[2].toLongLong();
            e.nzbPath  = f.value(11).trimmed();
            e.inputSize = f.value(12).trimmed();
            _entries << e;
        }
    }
    _fillTable();
}

QList<int> HistoryWidget::_visibleEntries() const
{
    QList<int> res;
    QString search = _search->text().trimmed();
    QDateTime now = QDateTime::currentDateTime();
    QDateTime from;
    switch (_period->currentIndex())
    {
    case 1: from = QDateTime(now.date(), QTime(0, 0)); break;
    case 2: from = now.addDays(-7); break;
    case 3: from = now.addDays(-30); break;
    default: break;
    }
    for (int i = 0; i < _entries.size(); ++i)
    {
        const Entry &e = _entries.at(i);
        if (from.isValid() && e.date < from)
            continue;
        if (!search.isEmpty()
                && !e.name.contains(search, Qt::CaseInsensitive)
                && !e.archive.contains(search, Qt::CaseInsensitive)
                && !e.groups.contains(search, Qt::CaseInsensitive)
                && !e.from.contains(search, Qt::CaseInsensitive)
                && !e.nzbPath.contains(search, Qt::CaseInsensitive))
            continue;
        res << i;
    }
    return res;
}

void HistoryWidget::_fillTable()
{
    int sortColumn = _table->horizontalHeader()->sortIndicatorSection();
    Qt::SortOrder order = _table->horizontalHeader()->sortIndicatorOrder();
    _table->setSortingEnabled(false);
    _table->setRowCount(0);

    QList<int> visible = _visibleEntries();
    double totalBytes = 0, sumSpeed = 0;
    int nbSpeed = 0;
    _table->setRowCount(visible.size());
    int row = 0;
    for (int idx : visible)
    {
        const Entry &e = _entries.at(idx);
        totalBytes += e.bytes;
        if (e.speedBps > 0)
        {
            sumSpeed += e.speedBps;
            ++nbSpeed;
        }

        SortItem *date = new SortItem(e.date.toString("yyyy-MM-dd  hh:mm"), e.date);
        date->setData(sEntryRole, idx);
        _table->setItem(row, COL_DATE, date);
        _table->setItem(row, COL_NAME,  new SortItem(e.name, e.name));
        _table->setItem(row, COL_SIZE,  new SortItem(e.size, e.bytes));
        _table->setItem(row, COL_SPEED, new SortItem(e.speed, e.speedBps));

        QString status = e.status.isEmpty() ? QString("-") :
                         e.status == "OK" ? tr("OK") :
                         e.status.startsWith("ERRORS") ? tr("errors %1").arg(e.status.mid(6).trimmed()) : e.status;
        SortItem *statusItem = new SortItem(status, e.status);
        if (e.status == "OK")
            statusItem->setForeground(_ngPost->isDarkTheme() ? QColor("#6ccb5f") : QColor("#107c10"));
        else if (e.status.startsWith("ERRORS"))
            statusItem->setForeground(_ngPost->isDarkTheme() ? QColor("#ff99a4") : QColor("#c42b1c"));
        _table->setItem(row, COL_STATUS, statusItem);

        _table->setItem(row, COL_FILES, new SortItem(e.nbFiles >= 0 ? QString::number(e.nbFiles) : QString("-"), e.nbFiles));
        _table->setItem(row, COL_DURATION, new SortItem(e.duration.isEmpty() ? QString("-") : e.duration, e.durationSec));
        _table->setItem(row, COL_ARCHIVE, new SortItem(e.archive, e.archive));
        QString pass = e.pass.isEmpty() ? QString() : (_showPasswords ? e.pass : sHiddenPass);
        _table->setItem(row, COL_PASS,   new SortItem(pass, e.pass));
        _table->setItem(row, COL_GROUPS, new SortItem(e.groups, e.groups));
        _table->setItem(row, COL_FROM,   new SortItem(e.from, e.from));
        for (int col = 0; col < NB_COLUMNS; ++col)
            if (QTableWidgetItem *item = _table->item(row, col))
                item->setToolTip(col == COL_PASS && !_showPasswords ? QString() : item->text());
        ++row;
    }

    _table->setSortingEnabled(true);
    _table->sortByColumn(sortColumn, order);

    static bool firstFill = true;
    if (firstFill && !visible.isEmpty())
    {
        _table->resizeColumnsToContents();
        if (_table->columnWidth(COL_NAME) > 320)
            _table->setColumnWidth(COL_NAME, 320);
        if (_table->columnWidth(COL_PASS) < 90)
            _table->setColumnWidth(COL_PASS, 90);
        firstFill = false;
    }

    if (visible.isEmpty())
        _summary->setText(tr("No upload"));
    else
        _summary->setText(tr("%1 upload(s)  ·  total %2  ·  average speed %3").arg(visible.size())
                          .arg(Diagnostics::humanSize(static_cast<qint64>(totalBytes)))
                          .arg(nbSpeed ? Diagnostics::humanSize(static_cast<qint64>(sumSpeed / nbSpeed)) + "/s" : QString("-")));

    // message in the middle of the table when it's empty
    QString empty;
    if (_ngPost->historyFile().isEmpty())
        empty = tr("The history is turned off.\nTurn it on in the Settings to record your uploads.");
    else if (_entries.isEmpty())
        empty = tr("No upload recorded yet.\nEach finished post will appear here.");
    else if (visible.isEmpty())
        empty = tr("No upload matches the search.");
    _emptyLbl->setText(empty);
    _emptyLbl->setVisible(!empty.isEmpty());
    _emptyLbl->setGeometry(0, 0, _table->viewport()->width(), qMax(120, _table->viewport()->height()));
}

const HistoryWidget::Entry *HistoryWidget::_entryAt(int row) const
{
    QTableWidgetItem *item = _table->item(row, COL_DATE);
    if (!item)
        return nullptr;
    int idx = item->data(sEntryRole).toInt();
    return idx >= 0 && idx < _entries.size() ? &_entries.at(idx) : nullptr;
}

void HistoryWidget::onFilter()
{
    _fillTable();
}

void HistoryWidget::onShowPasswords(bool show)
{
    _showPasswords = show;
    _passButton->setText(_showPasswords ? tr("Hide passwords") : tr("Show passwords"));
    _fillTable();
}

void HistoryWidget::onContextMenu(const QPoint &pos)
{
    int row = _table->rowAt(pos.y());
    const Entry *e = _entryAt(row);
    if (!e)
        return;
    Entry entry = *e; // copy: the table can be reloaded while the menu is open
    QMenu menu(this);
    QAction *copyPass = menu.addAction(tr("Copy the password"));
    copyPass->setEnabled(!entry.pass.isEmpty());
    QAction *copyName    = menu.addAction(tr("Copy the name"));
    QAction *copyArchive = menu.addAction(tr("Copy the archive name"));
    copyArchive->setEnabled(!entry.archive.isEmpty());
    menu.addSeparator();
    QAction *openFolder = menu.addAction(tr("Open the NZB folder"));

    QAction *res = menu.exec(_table->viewport()->mapToGlobal(pos));
    if (res == copyPass)
        QApplication::clipboard()->setText(entry.pass);
    else if (res == copyName)
        QApplication::clipboard()->setText(entry.name);
    else if (res == copyArchive)
        QApplication::clipboard()->setText(entry.archive);
    else if (res == openFolder)
    {
        QString folder;
        if (!entry.nzbPath.isEmpty())
            folder = QFileInfo(QDir::fromNativeSeparators(entry.nzbPath)).absolutePath();
        if (folder.isEmpty() || !QFileInfo(folder).isDir())
            folder = _ngPost->nzbPath().isEmpty() ? QString() : QFileInfo(_ngPost->nzbPath()).absolutePath();
        if (!folder.isEmpty() && QFileInfo(folder).isDir())
            QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
        else
            QMessageBox::information(this, tr("NZB folder"), tr("The folder of this NZB was not found."));
    }
}

void HistoryWidget::onExport()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export the history"),
                                                QDir::home().absoluteFilePath("ngPost_history_export.csv"),
                                                tr("CSV files (*.csv)"));
    if (path.isEmpty())
        return;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Export the history"), tr("Can't write the file %1").arg(path));
        return;
    }
    QString sep = _ngPost->historyFieldSeparator();
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << NgPost::historyHeader().join(sep) << "\n";
    for (int row = 0; row < _table->rowCount(); ++row) // in the order shown
    {
        const Entry *e = _entryAt(row);
        if (!e)
            continue;
        stream << e->date.toString("yyyy/MM/dd hh:mm:ss") << sep << e->name << sep << e->size << sep << e->speed
               << sep << e->archive << sep << e->pass << sep << e->groups << sep << e->from << sep << e->status
               << sep << (e->nbFiles >= 0 ? QString::number(e->nbFiles) : QString()) << sep << e->duration
               << sep << e->nzbPath << sep << e->inputSize << "\n";
    }
    stream.flush();
    if (!file.commit())
        QMessageBox::warning(this, tr("Export the history"), tr("Can't write the file %1").arg(path));
}

void HistoryWidget::onOpenFile()
{
    QString path = _ngPost->historyFile();
    if (!path.isEmpty() && QFileInfo::exists(path))
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
}
