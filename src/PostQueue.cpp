//========================================================================
//
// ngPost Deluxe: persistent queue of the posts
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

#include "PostQueue.h"
#include <QFile>
#include <QSaveFile>
#include <QLockFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QCoreApplication>

static const int sQueueVersion = 1;

PostQueue::PostQueue() : _path(), _lock(nullptr), _isOpen(false), _entries() {}

PostQueue::~PostQueue()
{
    delete _lock; // releases the lock
}

bool PostQueue::open(const QString &path, QString &error)
{
    _path = path;
    _lock = new QLockFile(path + ".lock");
    _lock->setStaleLockTime(0); // a lock is only stale if its process is not running anymore
    if (!_lock->tryLock(0))
    {
        error = QCoreApplication::translate("PostQueue",
                                            "the queue of the posts is used by another ngPost (%1): "
                                            "this one won't save its posts").arg(path);
        delete _lock;
        _lock = nullptr;
        return false;
    }

    QFile file(path);
    if (file.exists())
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            error = QCoreApplication::translate("PostQueue", "can't read the queue of the posts: %1").arg(path);
            return false;
        }
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            // never lose the user's data: keep the broken file aside
            QString broken = path + ".broken";
            QFile::remove(broken);
            QFile::rename(path, broken);
            error = QCoreApplication::translate("PostQueue",
                                                "the queue of the posts is damaged, it has been renamed %1").arg(broken);
        }
        else
        {
            for (const QJsonValue &val : doc.object().value("posts").toArray())
            {
                QJsonObject entry = val.toObject();
                if (!entry.value("id").toString().isEmpty())
                    _entries << entry;
            }
        }
    }
    _isOpen = true;
    return true;
}

void PostQueue::put(const QJsonObject &entry)
{
    if (!_isOpen)
        return;
    const QString id = entry.value("id").toString();
    for (int i = 0 ; i < _entries.size() ; ++i)
    {
        if (_entries.at(i).value("id").toString() == id)
        {
            _entries[i] = entry;
            _save();
            return;
        }
    }
    _entries << entry;
    _save();
}

void PostQueue::remove(const QString &id)
{
    if (!_isOpen)
        return;
    for (int i = 0 ; i < _entries.size() ; ++i)
    {
        if (_entries.at(i).value("id").toString() == id)
        {
            _entries.removeAt(i);
            _save();
            return;
        }
    }
}

void PostQueue::clear()
{
    if (!_isOpen)
        return;
    _entries.clear();
    _save();
}

QString PostQueue::newId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool PostQueue::_save()
{
    if (_entries.isEmpty())
    {
        QFile::remove(_path);
        return true;
    }

    QJsonArray posts;
    for (const QJsonObject &entry : _entries)
        posts << entry;
    QJsonObject root;
    root.insert("version", sQueueVersion);
    root.insert("warning", "this file contains the passwords of the archives in clear text: keep it private");
    root.insert("posts", posts);

    QSaveFile file(_path); // written in a temporary file, then renamed: never half written
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return file.commit();
}

static QJsonArray toArray(const QList<QString> &list)
{
    QJsonArray arr;
    for (const QString &str : list)
        arr << str;
    return arr;
}

static QList<QString> toStringList(const QJsonValue &val)
{
    QList<QString> list;
    for (const QJsonValue &v : val.toArray())
        list << v.toString();
    return list;
}

QJsonObject PostQueue::repostToJson(const RepostInfo &info)
{
    QJsonObject obj;
    obj.insert("nzb", info.nzbPath);
    obj.insert("groups", toArray(info.grpList));
    obj.insert("from", QString::fromStdString(info.from));
    obj.insert("obfuscateArticles", info.obfuscateArticles);
    obj.insert("compressDir", info.compressDir);
    obj.insert("keepRar", info.keepRar);
    obj.insert("delOriginals", info.delOriginals);
    obj.insert("movePostedTo", info.movePostedTo);
    QList<QString> originals;
    for (const QFileInfo &fi : info.originals)
        originals << fi.absoluteFilePath();
    obj.insert("originals", toArray(originals));

    QJsonArray files;
    for (const RepostFile &file : info.files)
    {
        QJsonObject f;
        f.insert("path", file.path);
        f.insert("num", static_cast<int>(file.num));
        f.insert("nbFiles", static_cast<int>(file.nbFiles));
        f.insert("padding", file.padding);
        f.insert("groups", toArray(file.groups));
        f.insert("nbArticles", static_cast<int>(file.nbArticles));
        f.insert("size", QString::number(file.size));
        f.insert("mtime", QString::number(file.mtimeMs));
        QJsonArray parts;
        for (const RepostPart &part : file.parts)
        {
            QJsonObject p;
            p.insert("part", static_cast<int>(part.part));
            p.insert("pos", QString::number(part.pos)); // qint64: as string, json numbers are doubles
            p.insert("bytes", QString::number(part.bytes));
            p.insert("msgId", part.oldMsgId);
            parts << p;
        }
        f.insert("parts", parts);
        files << f;
    }
    obj.insert("files", files);
    return obj;
}

RepostInfo PostQueue::repostFromJson(const QJsonObject &obj)
{
    RepostInfo info;
    info.nzbPath           = obj.value("nzb").toString();
    info.grpList           = toStringList(obj.value("groups"));
    info.from              = obj.value("from").toString().toStdString();
    info.obfuscateArticles = obj.value("obfuscateArticles").toBool();
    info.compressDir       = obj.value("compressDir").toString();
    info.keepRar           = obj.value("keepRar").toBool();
    info.delOriginals      = obj.value("delOriginals").toBool();
    info.movePostedTo      = obj.value("movePostedTo").toString();
    for (const QString &path : toStringList(obj.value("originals")))
        info.originals << QFileInfo(path);

    for (const QJsonValue &val : obj.value("files").toArray())
    {
        QJsonObject f = val.toObject();
        RepostFile file;
        file.path       = f.value("path").toString();
        file.num        = static_cast<uint>(f.value("num").toInt());
        file.nbFiles    = static_cast<uint>(f.value("nbFiles").toInt());
        file.padding    = f.value("padding").toInt(1);
        file.groups     = toStringList(f.value("groups"));
        file.nbArticles = static_cast<uint>(f.value("nbArticles").toInt());
        file.size       = f.value("size").toString().toLongLong();
        file.mtimeMs    = f.value("mtime").toString().toLongLong();
        for (const QJsonValue &pv : f.value("parts").toArray())
        {
            QJsonObject p = pv.toObject();
            RepostPart part;
            part.part     = static_cast<uint>(p.value("part").toInt());
            part.pos      = p.value("pos").toString().toLongLong();
            part.bytes    = p.value("bytes").toString().toLongLong();
            part.oldMsgId = p.value("msgId").toString();
            file.parts << part;
        }
        if (!file.path.isEmpty() && !file.parts.isEmpty())
            info.files << file;
    }
    return info;
}
