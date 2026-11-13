#include "PostedMover.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::cerr << "FAILED line " << __LINE__ << ": " #cond "\n"; ++failures; } } while (0)

static void write(const QString &path, const QByteArray &data)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly);
    f.write(data);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QTemporaryDir tmp;
    QDir root(tmp.path());

    // a file goes into a "Posted" folder created next to it
    write(root.filePath("a.mkv"), "AAA");
    auto r = PostedMover::move(QFileInfo(root.filePath("a.mkv")), "Posted");
    CHECK(r.ok);
    CHECK(!QFile::exists(root.filePath("a.mkv")));
    CHECK(QFile::exists(root.filePath("Posted/a.mkv")));

    // the same name again: nothing is overwritten
    write(root.filePath("a.mkv"), "BBBB");
    r = PostedMover::move(QFileInfo(root.filePath("a.mkv")), "Posted");
    CHECK(r.ok);
    CHECK(QFile::exists(root.filePath("Posted/a (2).mkv")));
    write(root.filePath("a.mkv"), "CC");
    r = PostedMover::move(QFileInfo(root.filePath("a.mkv")), "Posted");
    CHECK(QFile::exists(root.filePath("Posted/a (3).mkv")));
    QFile first(root.filePath("Posted/a.mkv"));
    first.open(QIODevice::ReadOnly);
    CHECK(first.readAll() == "AAA");

    // a folder, with its content
    write(root.filePath("show/s1/e1.mkv"), "E1");
    write(root.filePath("show/e2.mkv"), "E2");
    r = PostedMover::move(QFileInfo(root.filePath("show")), "Posted");
    CHECK(r.ok);
    CHECK(!QDir(root.filePath("show")).exists());
    CHECK(QFile::exists(root.filePath("Posted/show/s1/e1.mkv")));
    write(root.filePath("show/x.txt"), "X");
    r = PostedMover::move(QFileInfo(root.filePath("show")), "Posted");
    CHECK(QFile::exists(root.filePath("Posted/show (2)/x.txt")));

    // an absolute folder
    QTemporaryDir other;
    write(root.filePath("b.bin"), "B");
    r = PostedMover::move(QFileInfo(root.filePath("b.bin")), other.path() + "/done");
    CHECK(r.ok);
    CHECK(QFile::exists(other.path() + "/done/b.bin"));

    // the Posted folder inside the folder that is moved is refused, nothing is lost
    write(root.filePath("c/f.txt"), "F");
    r = PostedMover::move(QFileInfo(root.filePath("c")), root.filePath("c/Posted"));
    CHECK(!r.ok);
    CHECK(QFile::exists(root.filePath("c/f.txt")));

    // missing item
    r = PostedMover::move(QFileInfo(root.filePath("nothing")), "Posted");
    CHECK(!r.ok);

    // the scans must ignore the Posted folder
    CHECK(PostedMover::isPostedFolder("Posted", QFileInfo(root.filePath("Posted"))));
    CHECK(!PostedMover::isPostedFolder("Posted", QFileInfo(root.filePath("c"))));
    CHECK(PostedMover::isPostedFolder(root.filePath("Posted"), QFileInfo(root.filePath("Posted"))));
    CHECK(!PostedMover::isPostedFolder("", QFileInfo(root.filePath("Posted"))));
    CHECK(!PostedMover::isPostedFolder("Posted", QFileInfo(root.filePath("Posted/a.mkv")))); // a file is never the folder

    if (failures)
        return 1;
    std::cout << "posted_mover_test: OK\n";
    return 0;
}
