#include "PostChecker.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <iostream>

static int failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << "\n"; ++failures; } } while (0)

static bool has(const QList<PostChecker::Issue> &l, PostChecker::Code c, bool blocking)
{
    for (const auto &i : l)
        if (i.code == c && i.blocking == blocking)
            return true;
    return false;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    using C = PostChecker::Code;
    QTemporaryDir dir;
    QString prog = dir.filePath("prog");
    QFile f(prog);
    f.open(QIODevice::WriteOnly);
    f.write("#!/bin/sh\n");
    f.close();
    f.setPermissions(f.permissions() | QFileDevice::ExeUser);

    PostChecker::Input ok;
    ok.nbServers = 1;
    ok.nzbFile = dir.filePath("a.nzb");
    CHECK(PostChecker::check(ok).isEmpty(), "a plain post has no issue");

    PostChecker::Input in = ok;
    in.nbServers = 0;
    in.hasGroups = false;
    auto l = PostChecker::check(in);
    CHECK(has(l, C::NoServer, true) && has(l, C::NoGroup, true), "no server and no group are errors");

    in = ok;
    in.invalidGroups = "bad group";
    CHECK(has(PostChecker::check(in), C::NoGroup, true), "invalid group is an error");

    in = ok;
    in.hasFolder = true;
    l = PostChecker::check(in);
    CHECK(has(l, C::FoldersNoCompress, true) && !l.isEmpty() && !l[0].fixLabel.isEmpty(), "folders need compression, with a fix");

    in = ok;
    in.compress = true;
    in.tmpPath = dir.path();
    in.rarPath = dir.filePath("missing");
    CHECK(has(PostChecker::check(in), C::RarMissing, true), "missing rar is an error");
    in.rarPath = prog;
    CHECK(PostChecker::check(in).isEmpty(), "rar found, tmp ok: no issue");

    in.passwordChecked = true;
    CHECK(has(PostChecker::check(in), C::PasswordEmpty, true), "empty password is an error");
    in.password = "x";
    CHECK(PostChecker::check(in).isEmpty(), "password set: no issue");

    in = ok;
    in.par2 = true;
    in.tmpPath = dir.path();
    CHECK(has(PostChecker::check(in), C::Par2Missing, true), "missing par2 is an error");
    in.par2Path = prog;
    CHECK(PostChecker::check(in).isEmpty(), "par2 found: no issue");

    in = ok;
    in.nzbFile = dir.filePath("nothere/a.nzb");
    CHECK(has(PostChecker::check(in), C::NzbFolderNotWritable, true), "nzb folder must exist");

    in = ok;
    in.par2 = true;
    in.par2Path = prog;
    in.par2Pct = 10;
    in.sourceBytes = 1000000;
    in.tmpPath = dir.path();
    in.freeBytesTmp = 1000;
    l = PostChecker::check(in);
    CHECK(has(l, C::LowSpace, false), "low space is a warning");
    in.freeBytesTmp = 1000000000;
    CHECK(PostChecker::check(in).isEmpty(), "enough space: no issue");
    in.tmpPath = dir.filePath("nothere");
    CHECK(has(PostChecker::check(in), C::TmpNotWritable, false), "bad temporary folder is a warning");

    in = ok;
    in.mediaInfoWanted = true;
    in.hasMediaFiles = true;
    in.mediaInfoFound = false;
    CHECK(has(PostChecker::check(in), C::MediaInfoMissing, false), "MediaInfo missing is a warning");
    in.hasMediaFiles = false;
    CHECK(PostChecker::check(in).isEmpty(), "no media file: MediaInfo is not needed");

    CHECK(PostChecker::uploadBytes(1000, false, 10) == 1000, "no par2: same size");
    CHECK(PostChecker::uploadBytes(1000, true, 10) == 1100, "par2 10 %");
    CHECK(PostChecker::durationText(1000, 0).isEmpty(), "unknown speed: no time");
    CHECK(PostChecker::durationText(100LL * 1000 * 1000, 1000 * 1000) == "2 min", "100 MB at 1 MB/s = 2 min (rounded)");
    CHECK(PostChecker::durationText(3600LL * 1000 * 1000, 1000 * 1000) == "1 h 00 min", "one hour");

    if (failures == 0)
        std::cout << "post_checker_test: all good\n";
    return failures ? 1 : 0;
}
