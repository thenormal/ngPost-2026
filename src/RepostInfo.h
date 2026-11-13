//========================================================================
//
// ngPost Deluxe: everything needed to post again the Articles that failed
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

#ifndef REPOSTINFO_H
#define REPOSTINFO_H

#include <QString>
#include <QList>
#include <QVector>
#include <QFileInfoList>
#include <string>

/*!
 * \brief One Article that could not be posted: where it is in the file and
 * the Message-ID that was written in the nzb for it (that Message-ID doesn't exist on the server)
 */
struct RepostPart
{
    uint    part  = 0;  //!< part number (Article number) in the file
    qint64  pos   = 0;  //!< position in the file
    qint64  bytes = 0;  //!< number of bytes of the file that are in this Article
    QString oldMsgId;   //!< Message-ID currently written in the nzb
};

/*!
 * \brief A file that has some Articles to post again
 */
struct RepostFile
{
    QString       path;
    uint          num      = 0; //!< number of the file in the nzb: [num/nbFiles]
    uint          nbFiles  = 0;
    int           padding  = 1;
    QList<QString> groups;      //!< groups of the file in the nzb
    uint          nbArticles = 0; //!< number of Articles of the whole file (yEnc header: total=)
    qint64        size     = 0; //!< size and modification time of the file when it was posted:
    qint64        mtimeMs  = 0; //!< we must not post again parts of a file that has changed since
    QVector<RepostPart> parts;
};

/*!
 * \brief Everything we need to post again the missing Articles of a post and to fix its nzb.
 * It's a plain value, so it stays valid when the PostingJob is deleted (it is kept by the tab).
 */
struct RepostInfo
{
    QString        queueId;  //!< id of the entry in the persistent queue (PostQueue)
    QString        nzbPath;
    QList<QString> grpList;
    std::string    from;
    bool           obfuscateArticles = false;

    // what to clean when everything has been posted
    QString        compressDir;           //!< folder with the archives and par2 (empty if nothing to clean)
    bool           keepRar       = false;
    bool           delOriginals  = false; //!< the user asked to delete (or move) the original files after the post
    QString        movePostedTo;          //!< not empty: they are moved to this folder instead of deleted
    QFileInfoList  originals;

    QVector<RepostFile> files;

    bool isEmpty() const { return files.isEmpty(); }
    int  nbArticles() const
    {
        int nb = 0;
        for (const RepostFile &file : files)
            nb += file.parts.size();
        return nb;
    }
};

#endif // REPOSTINFO_H
