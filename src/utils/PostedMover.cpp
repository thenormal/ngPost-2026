#include "PostedMover.h"

#include <QDir>
#include <QFile>

QString PostedMover::destinationFolder(const QString &posted, const QFileInfo &item)
{
    if (QDir::isAbsolutePath(posted))
        return QDir::cleanPath(posted);
    return QDir::cleanPath(item.absoluteDir().absoluteFilePath(posted));
}

QString PostedMover::uniqueTarget(const QString &dir, const QString &name, bool isDir)
{
    QDir d(dir);
    if (!QFileInfo::exists(d.absoluteFilePath(name)))
        return d.absoluteFilePath(name);

    QString base = name, ext;
    if (!isDir)
    {
        QFileInfo fi(name);
        base = fi.completeBaseName();
        ext  = fi.suffix().isEmpty() ? QString() : "." + fi.suffix();
    }
    for (int i = 2; ; ++i)
    {
        QString candidate = d.absoluteFilePath(QString("%1 (%2)%3").arg(base).arg(i).arg(ext));
        if (!QFileInfo::exists(candidate))
            return candidate;
    }
}

bool PostedMover::copyRecursively(const QString &from, const QString &to, QString &error)
{
    QFileInfo src(from);
    if (src.isDir())
    {
        if (!QDir().mkpath(to))
        {
            error = QString("can't create %1").arg(to);
            return false;
        }
        QDir dir(from);
        for (const QFileInfo &child : dir.entryInfoList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System))
            if (!copyRecursively(child.absoluteFilePath(), QDir(to).absoluteFilePath(child.fileName()), error))
                return false;
        return true;
    }
    if (!QFile::copy(from, to))
    {
        error = QString("can't copy %1").arg(from);
        return false;
    }
    if (QFileInfo(to).size() != src.size())
    {
        error = QString("the copy of %1 is not complete").arg(from);
        return false;
    }
    return true;
}

PostedMover::Result PostedMover::move(const QFileInfo &item, const QString &posted)
{
    Result res;
    if (!item.exists())
    {
        res.error = QString("%1 does not exist").arg(item.absoluteFilePath());
        return res;
    }
    const QString destDir = destinationFolder(posted, item);
    const QString srcPath = QDir::cleanPath(item.absoluteFilePath());
    if (item.isDir() && (destDir == srcPath || destDir.startsWith(srcPath + "/")))
    {
        res.error = QString("the Posted folder can't be inside the folder that is moved (%1)").arg(srcPath);
        return res;
    }
    if (!QDir().mkpath(destDir))
    {
        res.error = QString("can't create the folder %1").arg(destDir);
        return res;
    }
    const QString target = uniqueTarget(destDir, item.fileName(), item.isDir());

    // same disk: a simple rename
    if (item.isDir() ? QDir().rename(srcPath, target) : QFile::rename(srcPath, target))
    {
        res.ok = true;
        res.target = target;
        return res;
    }

    // another disk: copy, check, then delete the original
    QString error;
    if (!copyRecursively(srcPath, target, error))
    {
        if (QFileInfo(target).isDir())
            QDir(target).removeRecursively();
        else
            QFile::remove(target);
        res.error = error;
        return res;
    }
    bool removed = item.isDir() ? QDir(srcPath).removeRecursively() : QFile::remove(srcPath);
    res.target = target;
    if (!removed)
    {
        res.error = QString("copied to %1 but the original could not be deleted").arg(target);
        return res;
    }
    res.ok = true;
    return res;
}

bool PostedMover::isPostedFolder(const QString &posted, const QFileInfo &fi)
{
    if (posted.isEmpty() || !fi.isDir())
        return false;
#ifdef Q_OS_WIN
    const Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
    if (QDir::isAbsolutePath(posted))
        return QString::compare(QDir::cleanPath(posted), QDir::cleanPath(fi.absoluteFilePath()), cs) == 0;
    return QString::compare(posted, fi.fileName(), cs) == 0;
}
