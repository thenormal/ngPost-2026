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

#ifndef NntpFile_H
#define NntpFile_H
#include <QObject>
#include <QVector>
#include <QSet>
#include <QFileInfo>
#include <QHash>
#include "RepostInfo.h"
class NntpArticle;
class QTextStream;
class PostingJob;

/*!
 * \brief The NntpFile class represent a File uploaded on Usenet thus it will holds all its Articles
 * All NntpFiles will live (active objects) in the Main Thread so they can write into the nzb without concurrency
 * when all its articles are posted (slot onArticlePosted) it emits allArticlesArePosted
 */
class NntpFile : public QObject
{
    Q_OBJECT

public:
    /*!
     * \param forceNbArticles  number of Articles of the whole file (0: computed from the size of the file)
     * \param nbToPost         number of Articles that will be posted (0: all of them).
     *                         Only used to post again the Articles that failed.
     */
    NntpFile(PostingJob *postingJob, const QFileInfo &file,
             uint num, uint nbFiles, int padding,
             const QList<QString> &grpList,
             uint forceNbArticles = 0, uint nbToPost = 0);
    ~NntpFile();

    inline void addArticle(NntpArticle *article);

    void writeToNZB(QTextStream &stream, const QString &from);

    /*!
     * \brief what we need to post again the Articles that failed (nothing if all were posted)
     * Must be called before the Articles are deleted.
     */
    bool collectRepost(RepostFile &out) const;

    //! Message-IDs of the Articles that have been posted (part number => Message-ID)
    void postedMsgIds(QHash<uint, QString> &out) const;

    //! posting again only some Articles of a file that has already been posted
    inline void setRepostParts(const QVector<RepostPart> &parts);
    inline const QVector<RepostPart> &repostParts() const;
    inline bool isRepost() const;

    inline QString stats() const;
    inline QString path() const;
    inline QString name() const;
    inline QString nameWithQuotes() const;
    inline std::string fileName() const;
    inline qint64 fileSize() const;
    inline uint nbArticles() const;
    inline uint nbExpectedArticles() const;
    inline uint nbFailedArticles() const;
    inline bool hasFailedArticles() const;
    inline const std::string &groups() const;

    QString missingArticles() const;

signals:
    void allArticlesArePosted();
    void startPosting();
    void scheduleDeletion();
    void errorReadingFile();

public slots:
    void onArticlePosted(quint64 size);
    void onArticleFailed(quint64 size);


private:
    PostingJob       *const _postingJob;
    const QFileInfo         _file;      //!< original file
    const uint              _num;       //!< file number
    const uint              _nbFiles;   //!< total number of file
    const int               _padding;
    const QList<QString>    _grpList;   //!< groups where the file is posted
    const std::string       _groups;
    const uint              _nbAticles; //!< total number of articles
    const uint              _nbExpected;//!< number of articles that will be posted (all, except when we post again the failed ones)
    QVector<NntpArticle*>   _articles;  //!< all articles (that are yEnc encoded)
    QVector<RepostPart>     _repostParts; //!< only filled when we post again the failed Articles of this file
    bool                    _isRepost = false;

    QSet<uint> _posted; //!< part number of the Articles that have been posted (uploaded on the socket)
    QSet<uint> _failed; //!< part number of the Articles that FAILED to be posted (uploaded on the socket)
};

void NntpFile::addArticle(NntpArticle *article) { _articles.push_back(article); }

void NntpFile::setRepostParts(const QVector<RepostPart> &parts) { _repostParts = parts; _isRepost = true; }
const QVector<RepostPart> &NntpFile::repostParts() const { return _repostParts; }
bool NntpFile::isRepost() const { return _isRepost; }

QString NntpFile::stats() const
{
    return QString("[%1 ok / %2] %3").arg(_posted.size()).arg(_nbExpected).arg(_file.absoluteFilePath());
}
QString NntpFile::path() const { return _file.absoluteFilePath(); }
QString NntpFile::name() const {
    return QString("[%1/%2] %3").arg(_num, _padding, 10,  QChar('0')).arg(
                _nbFiles).arg(_file.fileName());
}
QString NntpFile::nameWithQuotes() const {
    return QString("[%1/%2] \"%3\"").arg(_num, _padding, 10,  QChar('0')).arg(
                _nbFiles).arg(_file.fileName());
}

std::string NntpFile::fileName() const { return _file.fileName().toStdString(); }
qint64 NntpFile::fileSize() const { return _file.size(); }
uint NntpFile::nbArticles() const { return _nbAticles; }
uint NntpFile::nbExpectedArticles() const { return _nbExpected; }
uint NntpFile::nbFailedArticles() const { return static_cast<uint>(_failed.size()); }
bool NntpFile::hasFailedArticles() const { return _failed.size() != 0; }
const std::string &NntpFile::groups() const { return _groups; }

#endif // NntpFile_H
