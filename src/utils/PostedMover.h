#ifndef POSTEDMOVER_H
#define POSTEDMOVER_H

#include <QFileInfo>
#include <QString>

/*!
 * \brief Moves the files and folders that have been posted into a "Posted" folder.
 * It never overwrites: a name already used becomes "name (2)", "name (3)"...
 * On another disk the item is copied, checked, and only then the original is deleted.
 * No access to the program or to the GUI, so it is easy to test.
 */
class PostedMover
{
public:
    struct Result {
        bool    ok = false;
        QString target;  //!< where the item is now
        QString error;   //!< why it failed (the original is then left untouched)
    };

    //! the folder where \a item goes: \a posted as it is if absolute, else a folder of that name next to \a item
    static QString destinationFolder(const QString &posted, const QFileInfo &item);

    //! a path that does not exist yet in \a dir: "name", "name (2)", "name (3)"... ("base (2).ext" for files)
    static QString uniqueTarget(const QString &dir, const QString &name, bool isDir);

    static Result move(const QFileInfo &item, const QString &posted);

    //! true if \a fi is the "Posted" folder itself (the scans and the folder watching must ignore it)
    static bool isPostedFolder(const QString &posted, const QFileInfo &fi);

private:
    static bool copyRecursively(const QString &from, const QString &to, QString &error);
};

#endif // POSTEDMOVER_H
