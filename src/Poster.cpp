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

#include "Poster.h"
#include "NgPost.h"
#include "PostingJob.h"
#include "ArticleBuilder.h"
#include "NntpConnection.h"
#include "nntp/NntpArticle.h"

Poster::Poster(PostingJob *job, ushort id):
    _id(id),
    _ngPost(job->_ngPost),
    _job(job),
    _builderThread(),
    _connectionsThread(),
    _articleBuilder(new ArticleBuilder(this)),
    _nntpConnections(),
    _articles(),
    _secureArticles()
{
    _builderThread.setObjectName(QString("Builder #%1").arg(id));
    _connectionsThread.setObjectName(QString("Poster #%1").arg(id));
    _articleBuilder->moveToThread(&_builderThread);
}

Poster::~Poster()
{
#if defined(__DEBUG__) && defined(LOG_CONSTRUCTORS)
    qDebug() << "Destruction Poster #" << _id;
#endif

    stopThreads();

    if (_articleBuilder)
        delete _articleBuilder;
}

void Poster::addConnection(NntpConnection *connection)
{
    _nntpConnections.append(connection);
    connection->setPoster(this);
    connection->moveToThread(&_connectionsThread);
    emit connection->startConnection();
}

#ifdef __RELEASE_ARTICLES_WHEN_CON_FAILS__
uint Poster::nbActiveConnections() const
{
    uint nbActives = 0;
    for (NntpConnection *con : _nntpConnections)
    {
        if (con->isConnected())
            ++nbActives;
    }
    return nbActives;
}
#endif

NntpArticle *Poster::getNextArticle(const QString &conPrefix, const NntpServerParams *server)
{

    QMutexLocker lock(&_secureArticles); // thread safety (coming from a posting thread)

    if (MB_LoadAtomic(_job->_stopPosting))
        return nullptr;

    // 1.: an Article that failed elsewhere (another connection or another server) and was never tried
    //     on our server, or an Article given back by a Poster that lost all its connections
    NntpArticle *article = _job->takeRecoveryArticle(server, true);
    if (article)
    {
        if (_ngPost->debugMode())
            _job->_log(QString("[%1] taking back Article: %2").arg(conPrefix).arg(article->str()));
        return article;
    }

    if (_ngPost->debugFull())
        _job->_log(QString("[%1][Poster::getNextArticle] _articles.size() = %2").arg(conPrefix).arg(_articles.size()));

    // 2.: our own Articles
    if (_articles.size())
        article = _articles.dequeue();
    else
    {
        if (!MB_LoadAtomic(_job->_noMoreFiles))
        {
            // we should never come here as the goal is to have articles prepared in advance in the queue
            if (_ngPost->debugFull())
                _job->_log(QString("[%1][Poster::getNextArticle] no article prepared...").arg(conPrefix));

            article = _prepareNextArticle(conPrefix, false);
        }
    }

    if (article)
    {
        emit _articleBuilder->scheduleNextArticle(); // schedule the preparation of another Article in the _builderThread
        return article;
    }

    // 3.: nothing else to do: an Article waiting for a recovery, even if it already failed on our server
    //     (only when it failed on every server, otherwise we leave it to the other servers)
    article = _job->takeRecoveryArticle(server, false);
    if (article && _ngPost->debugMode())
        _job->_log(QString("[%1] trying again Article: %2").arg(conPrefix).arg(article->str()));
    return article;
}

void Poster::moveArticlesToRecovery()
{
    QMutexLocker lock(&_secureArticles);
    _forwardToRecovery = true; // the ones the builder is preparing will follow
    if (_articles.size())
    {
        _job->addToRecovery(_articles);
        _articles.clear();
    }
}

#ifdef __RELEASE_ARTICLES_WHEN_CON_FAILS__
void Poster::releaseArticle(const QString &conPrefix, NntpArticle *article, const NntpServerParams *server)
{
    if (_ngPost->debugMode())
        _job->_log(QString("[%1] releasing Article: %2").arg(conPrefix).arg(article->str()));

    // the PostingJob gives it to another connection (preferably on another server),
    // keeps it for a second round, or declares it failed
    _job->recoverArticle(article, server, conPrefix);
}
#endif

bool Poster::prepareArticlesInAdvance()
{
    bool canProduceAll = true;
    int nbArticlesToPrepare = _nntpConnections.size();
    for (int i = 0; i < nbArticlesToPrepare ; ++i)
    {
        if (!_prepareNextArticle(_builderThread.objectName()))
        {
#ifdef __DEBUG__
            _job->_log(QString("[%1] prepareArticlesInAdvance : no more Articles to produce after i = %1").arg(_builderThread.objectName()).arg(i));
#endif
            canProduceAll = false;
            break;
        }
    }
#ifdef __DEBUG__
    _job->_log(QString("[%1] prepareArticlesInAdvance: Article queue size:  %1").arg(_builderThread.objectName()).arg(_articles.size()));
#endif

    return canProduceAll;
}

NntpArticle *Poster::_prepareNextArticle(const QString &threadName, bool fillQueue)
{
    NntpArticle *article = _articleBuilder->getNextArticle(threadName);
    if (article && fillQueue)
        _articles.enqueue(article);
    return article;
}


bool Poster::isPosting() const
{
    return _job->isPosting();
}


void Poster::stopThreads()
{
    _builderThread.quit();
    _connectionsThread.quit();

    if (_ngPost->debugMode())
        _job->_log(QString("[Poster #%1] threads quit").arg(_id));

    _connectionsThread.wait();
    if (_ngPost->debugMode())
        _job->_log(QString("[Poster #%1] connection thread done").arg(_id));

    _builderThread.wait();

    if (_ngPost->debugMode())
        _job->_log(QString("[Poster #%1] threads stopped").arg(_id));
}
