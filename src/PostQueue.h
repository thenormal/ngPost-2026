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

#ifndef POSTQUEUE_H
#define POSTQUEUE_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include "RepostInfo.h"
class QLockFile;

/*!
 * \brief The posts that are not finished, saved in a json file (ngPost_queue.json next to the config)
 * so they can be posted again after the program has been closed or has crashed.
 *
 * Each entry is a json object with an "id" and a "state":
 *  - "waiting": a post (in the queue or being posted). It is posted again from the beginning.
 *  - "missing": a post that ended with Articles that couldn't be posted (key "missing": RepostInfo)
 * The file is rewritten atomically on each change and removed when the queue is empty.
 * A lock file prevents two instances of ngPost from using the same queue.
 * Only used from the main thread.
 */
class PostQueue
{
public:
    static constexpr const char *sStateWaiting = "waiting";
    static constexpr const char *sStateMissing = "missing";

    PostQueue();
    ~PostQueue();
    PostQueue(const PostQueue &) = delete;
    PostQueue &operator=(const PostQueue &) = delete;

    //! loads the queue and locks it. Returns false (with an error) if it can't be used.
    bool open(const QString &path, QString &error);
    inline bool isOpen() const { return _isOpen; }
    inline const QString &path() const { return _path; }

    inline const QList<QJsonObject> &entries() const { return _entries; }
    inline int size() const { return _entries.size(); }

    void put(const QJsonObject &entry); //!< add (at the end) or replace the entry with the same id
    void remove(const QString &id);
    void clear();

    static QString newId();

    static QJsonObject repostToJson(const RepostInfo &info);
    static RepostInfo  repostFromJson(const QJsonObject &obj);

private:
    bool _save();

    QString            _path;
    QLockFile         *_lock;
    bool               _isOpen;
    QList<QJsonObject> _entries;
};

#endif // POSTQUEUE_H
