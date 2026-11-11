#!/usr/bin/env python3
"""End-to-end tests of the command line ngPost against a fake NNTP server.

usage: python3 tests/run_tests.py /path/to/ngPost
"""
import glob
import hashlib
import shutil
import os
import subprocess
import sys
import tempfile
import traceback
import xml.etree.ElementTree as ET

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from nntp_stub import NntpStub  # noqa: E402

NGPOST = None
LAST_OUTPUT = ""            # output of the last ngPost run, printed when a test fails
ARTICLE_SIZE = 50000        # small articles => many articles for small files


def sha(b):
    return hashlib.sha256(b).hexdigest()


def make_file(path, size, seed):
    # deterministic pseudo random bytes that contain every byte value
    # (so the yEnc escaping of =, CR, LF, NUL and '.' is really exercised)
    out, x = bytearray(), seed
    while len(out) < size:
        x = (x * 1103515245 + 12345) & 0x7FFFFFFF
        out.append((x >> 16) & 0xFF)
    with open(path, "wb") as f:
        f.write(out)
    return bytes(out)


def run_ngpost(stub, work, inputs, nzb, extra=(), conns=3, timeout=120, password=None):
    env = dict(os.environ, HOME=work,          # no user config can interfere
               NGPOST_SECOND_ROUND_DELAY_MS="2000")
    cmd = [NGPOST, "-h", "127.0.0.1", "-P", str(stub.port), "-u", stub.user,
           "-p", password or stub.password, "-n", str(conns), "-g", "alt.binaries.test",
           "-a", str(ARTICLE_SIZE), "-o", nzb, "-q"]
    cmd += list(extra)
    for i in inputs:
        cmd += ["-i", i]
    r = subprocess.run(cmd, env=env, cwd=work, capture_output=True,
                       text=True, timeout=timeout)
    global LAST_OUTPUT
    LAST_OUTPUT = "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr)
    if r.returncode < 0:
        raise AssertionError("ngPost crashed (signal %d)\n%s%s" % (-r.returncode, r.stdout, r.stderr))
    return r


def ngpost_conf_cmd(stubs, work, inputs, nzb, conns=2, threads=None, extra=(), env_extra=None, conf_extra=""):
    """command and environment to post with a configuration file declaring one server per stub"""
    conf = os.path.join(work, "test.conf")
    with open(conf, "w") as f:
        f.write("GROUPS = alt.binaries.test\nARTICLE_SIZE = %d\nRESUME_WAIT = 2\nSOCK_TIMEOUT = 10\n" % ARTICLE_SIZE)
        if threads:
            f.write("THREAD = %d\n" % threads)
        f.write(conf_extra)
        for st in stubs:
            f.write("\n[server]\nhost = 127.0.0.1\nport = %d\nssl = false\nuser = %s\npass = %s\n"
                    "connection = %d\nenabled = true\n" % (st.port, st.user, st.password, conns))
    env = dict(os.environ, HOME=work, NGPOST_SECOND_ROUND_DELAY_MS="2000")
    env.update(env_extra or {})
    cmd = [NGPOST, "--conf", conf, "-q"] + (["-o", nzb] if nzb else []) + list(extra)
    for i in inputs:
        cmd += ["-i", i]
    return cmd, env


def run_ngpost_conf(stubs, work, inputs, nzb, conns=2, threads=None, extra=(), timeout=180, env_extra=None,
                    conf_extra=""):
    """post with a configuration file declaring one server per stub"""
    cmd, env = ngpost_conf_cmd(stubs, work, inputs, nzb, conns, threads, extra, env_extra, conf_extra)
    r = subprocess.run(cmd, env=env, cwd=work, capture_output=True, text=True, timeout=timeout)
    global LAST_OUTPUT
    LAST_OUTPUT = "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr)
    if r.returncode < 0:
        raise AssertionError("ngPost crashed (signal %d)\n%s%s" % (-r.returncode, r.stdout, r.stderr))
    return r


def merged_accepted(stubs):
    """the articles accepted by all the stubs, as if it was one server"""
    class All:
        pass
    a = All()
    a.accepted = {}
    for st in stubs:
        a.accepted.update(st.accepted)
    a.rebuild_files = lambda: NntpStub.rebuild_files(a)
    return a


def read_nzb(path):
    """-> {file subject: [message ids]}"""
    ns = "{http://www.newzbin.com/DTD/2003/nzb}"
    root = ET.parse(path).getroot()
    files = {}
    for f in root.iter(ns + "file"):
        files[f.get("subject")] = [s.text for s in f.iter(ns + "segment")]
    return files


def check(cond, msg):
    if not cond:
        raise AssertionError(msg)


def common_checks(stub, nzb, originals):
    check(os.path.exists(nzb), "the nzb file was not written")
    bad = [(a.msg_id, a.errors) for a in stub.accepted.values() if a.errors]
    check(not bad, "articles with yEnc errors: %s" % bad[:3])
    nzb_ids = {m for ids in read_nzb(nzb).values() for m in ids}
    check(nzb_ids == set(stub.accepted), "the message ids of the nzb (%d) differ from the "
          "articles received by the server (%d)" % (len(nzb_ids), len(stub.accepted)))
    if originals is not None:
        rebuilt = stub.rebuild_files()
        check(set(rebuilt) == set(originals), "files on the server %s, expected %s"
              % (sorted(rebuilt), sorted(originals)))
        for name, data in originals.items():
            check(sha(rebuilt[name]) == sha(data), "content of %s is different" % name)


# ------------------------------------------------------------------ scenarios
def test_single_file(work):
    stub = NntpStub().start()
    try:
        src = os.path.join(work, "movie.bin")
        data = make_file(src, 420_000, 1)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [src], nzb)
        check(r.returncode == 0, "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr))
        common_checks(stub, nzb, {"movie.bin": data})
        check(len(stub.accepted) == -(-len(data) // ARTICLE_SIZE), "unexpected number of articles")
    finally:
        stub.stop()


def test_several_files_and_folder(work):
    stub = NntpStub().start()
    try:
        folder = os.path.join(work, "album")
        os.makedirs(folder)
        originals = {}
        for name, size, seed in (("a.dat", 123_457, 2), ("b.dat", 49_999, 3), ("c.dat", 1, 4)):
            originals[name] = make_file(os.path.join(folder, name), size, seed)
        other = os.path.join(work, "z.dat")
        originals["z.dat"] = make_file(other, 200_001, 5)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [folder, other], nzb, conns=4)
        check(r.returncode == 0, "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr))
        common_checks(stub, nzb, originals)
    finally:
        stub.stop()


def test_obfuscation(work):
    stub = NntpStub().start()
    try:
        src = os.path.join(work, "secret.bin")
        data = make_file(src, 260_000, 6)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [src], nzb, extra=["-x"])
        check(r.returncode == 0, "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr))
        common_checks(stub, nzb, None)
        check(not any("secret" in a.subject for a in stub.accepted.values()),
              "the real file name appears in an article subject")
        check(sum(len(a.data) for a in stub.accepted.values()) == len(data),
              "the total size received is not the size of the file")
    finally:
        stub.stop()


def test_retry_after_refusals(work):
    # the server refuses the first try of every third article (part 3, 6, 9...)
    def policy(article, attempt):
        if article.part % 3 == 0 and attempt == 1:
            return b"441 posting failed"
        return None

    stub = NntpStub(policy=policy).start()
    try:
        src = os.path.join(work, "retry.bin")
        data = make_file(src, 600_000, 7)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [src], nzb)
        check(stub.refused > 0, "the test did not refuse any article")
        check(r.returncode == 0, "exit code %d\n%s%s" % (r.returncode, r.stdout, r.stderr))
        common_checks(stub, nzb, {"retry.bin": data})
    finally:
        stub.stop()


def test_wrong_password(work):
    # a refused login must stop the post (exit code != 0), not wait and retry for ever,
    # and the password must never be written in the output
    stub = NntpStub(password="good").start()
    try:
        src = os.path.join(work, "x.bin")
        make_file(src, 60_000, 8)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [src], nzb, timeout=40, password="WRONGpass42")
        check(r.returncode != 0, "ngPost exited with 0 although the login was refused")
        check(stub.bad_auth > 0, "the server never saw a bad password")
        check(len(stub.accepted) == 0, "articles were accepted with a wrong password")
        check("WRONGpass42" not in r.stdout + r.stderr, "the password appears in the output")
    finally:
        stub.stop()


def test_some_logins_refused(work):
    # only some connections are refused (like a connection limit): the others must finish the post
    stub = NntpStub(refuse_logins=2).start()
    try:
        src = os.path.join(work, "partial.bin")
        data = make_file(src, 300_000, 9)
        nzb = os.path.join(work, "out.nzb")
        run_ngpost(stub, work, [src], nzb, conns=4, timeout=60)
        check(stub.bad_auth >= 1, "no login was refused")
        try:
            common_checks(stub, nzb, {"partial.bin": data})
        except AssertionError as e:
            raise AssertionError("%s\nserver events (s, connection, event):\n%s" % (
                e, "\n".join("  %s" % (x,) for x in stub.events)))
    finally:
        stub.stop()


def test_article_always_refused_on_one_connection(work):
    # connection #1 refuses every article: the others must post them
    def policy(article, attempt, connection):
        return b"441 posting failed" if connection == 1 else None

    stub = NntpStub(policy=policy).start()
    try:
        src = os.path.join(work, "onecon.bin")
        data = make_file(src, 500_000, 10)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost(stub, work, [src], nzb, conns=3)
        check(stub.refused > 0, "connection #1 did not refuse anything")
        check(r.returncode == 0, "exit code %d" % r.returncode)
        common_checks(stub, nzb, {"onecon.bin": data})
    finally:
        stub.stop()


def test_server_going_down(work):
    # 2 servers, 2 threads: the second server goes down after 2 articles, so one Poster loses all its
    # connections; its Articles must be posted by the other one
    good = NntpStub().start()
    bad = NntpStub(die_after=2).start()
    try:
        src = os.path.join(work, "down.bin")
        data = make_file(src, 1_500_000, 11)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([good, bad], work, [src], nzb, conns=2, threads=2)
        check(bad.dead, "the second server never went down")
        # exit code 1 is fine here (the lost connections are reported as errors), what matters is the content
        common_checks(merged_accepted([good, bad]), nzb, {"down.bin": data})
    finally:
        good.stop()
        bad.stop()


def test_other_server_used(work):
    # the first server refuses every article: they must go to the second one
    refusing = NntpStub(policy=lambda article, attempt: b"441 posting failed").start()
    good = NntpStub().start()
    try:
        src = os.path.join(work, "other.bin")
        data = make_file(src, 400_000, 12)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([refusing, good], work, [src], nzb, conns=2, threads=1)
        check(refusing.refused > 0, "the first server did not refuse anything")
        check(r.returncode == 0, "exit code %d" % r.returncode)
        common_checks(good, nzb, {"other.bin": data})
    finally:
        refusing.stop()
        good.stop()


def test_second_round(work):
    # the server refuses everything during 3 seconds after the first article: the articles that
    # failed everywhere must be posted by the second round
    import time
    state = {"start": None}

    def policy(article, attempt):
        if article.part == 1:
            state["start"] = state["start"] or time.time()
            return None
        if state["start"] and time.time() - state["start"] < 3:
            return b"441 posting failed"
        return None

    stub = NntpStub(policy=policy).start()
    try:
        src = os.path.join(work, "round2.bin")
        data = make_file(src, 300_000, 13)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([stub], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "4000"})
        out = r.stdout + r.stderr
        check("new attempt" in out, "no second round announced")
        check(r.returncode == 0, "exit code %d" % r.returncode)
        common_checks(stub, nzb, {"round2.bin": data})
    finally:
        stub.stop()


def test_too_many_failures(work):
    # more than 200 articles fail everywhere: no second round, the post ends with errors
    global ARTICLE_SIZE
    stub = NntpStub(policy=lambda article, attempt: b"441 posting failed" if article.part > 1 else None).start()
    saved = ARTICLE_SIZE
    ARTICLE_SIZE = 2000
    try:
        src = os.path.join(work, "many.bin")
        make_file(src, 2000 * 230, 14)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([stub], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "500"}, timeout=240)
        out = r.stdout + r.stderr
        check("there won't be a second attempt" in out, "the limit of 200 failed articles was not applied")
        check("new attempt" not in out, "a second round was started anyway")
        check(r.returncode != 0, "exit code 0 although articles failed")
        check(len(stub.accepted) == 1, "%d articles accepted, expected 1" % len(stub.accepted))
    finally:
        ARTICLE_SIZE = saved
        stub.stop()


def test_repost_missing(work):
    # two articles fail even in the second round: the post ends with errors and its nzb lists
    # two Message-IDs that don't exist. Posting them again must fix the nzb.
    stub = NntpStub(policy=lambda article, attempt: b"441 posting failed"
                    if article.part in (3, 5) and stub.gaps < 2 else None).start()
    try:
        src = os.path.join(work, "repost.bin")
        data = make_file(src, 420_000, 21)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([stub], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "1000", "NGPOST_REPOST_MISSING": "1"})
        out = r.stdout + r.stderr
        check("new attempt" in out, "no second round announced")
        check("give up on Article" in out, "the 2 articles were not given up before the repost")
        check("2 missing Articles posted" in out, "the missing Articles were not posted again:\n" + out[-1500:])
        check("All the Articles are now posted" in out, "the repost did not finish cleanly")
        common_checks(stub, nzb, {"repost.bin": data})
        check(len(stub.accepted) == 9, "%d articles accepted, expected 9" % len(stub.accepted))
        check(sum(len(ids) for ids in read_nzb(nzb).values()) == 9, "the nzb has not 9 segments")
    finally:
        stub.stop()


def test_repost_nothing_to_do(work):
    # a post without failures must not start any repost
    stub = NntpStub().start()
    try:
        src = os.path.join(work, "fine.bin")
        data = make_file(src, 150_000, 22)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([stub], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_REPOST_MISSING": "1"})
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("missing Articles" not in out, "a repost was started for nothing")
        common_checks(stub, nzb, {"fine.bin": data})
    finally:
        stub.stop()


def test_repost_still_failing(work):
    # the server still refuses the articles when we post them again: the nzb must stay valid and the
    # post must say that articles are still missing
    stub = NntpStub(policy=lambda article, attempt: b"441 posting failed" if article.part == 2 else None).start()
    try:
        src = os.path.join(work, "still.bin")
        make_file(src, 200_000, 23)
        nzb = os.path.join(work, "out.nzb")
        r = run_ngpost_conf([stub], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "500", "NGPOST_REPOST_MISSING": "1"})
        out = r.stdout + r.stderr
        check("1 Articles are still missing" in out, "the missing Article was not reported:\n" + out[-1500:])
        check(r.returncode != 0, "exit code 0 although an article is missing")
        check(sum(len(ids) for ids in read_nzb(nzb).values()) == 4, "the nzb has not 4 segments")
    finally:
        stub.stop()


def _post_and_delete(work, repost):
    """--auto --compress --rm_posted with an article that fails everywhere (until the repost if repost is on)"""
    stub = NntpStub(policy=lambda article, attempt: b"441 posting failed"
                    if article.part == 2 and (not repost or stub.gaps < 2) else None).start()
    try:
        folder = os.path.join(work, "in")
        tmp = os.path.join(work, "tmp")
        os.makedirs(folder)
        os.makedirs(tmp)
        src = os.path.join(folder, "del.bin")
        make_file(src, 200_000, 31)
        env = {"NGPOST_SECOND_ROUND_DELAY_MS": "500"}
        if repost:
            env["NGPOST_REPOST_MISSING"] = "1"
        r = run_ngpost_conf([stub], work, [], os.path.join(work, "out"), conns=2, threads=1,
                            extra=["--auto", folder, "--compress", "--rm_posted"], env_extra=env,
                            conf_extra="TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (tmp, shutil.which("7z")))
        return r, src, tmp
    finally:
        stub.stop()


def test_originals_kept_when_articles_failed(work):
    # "delete the files once posted" must not delete anything when some articles couldn't be posted,
    # and the archives must stay so the missing articles can be posted again
    if not shutil.which("7z"):
        print("SKIP  (7z is not installed)")
        return
    r, src, tmp = _post_and_delete(work, repost=False)
    out = r.stdout + r.stderr
    check(os.path.exists(src), "the original file was deleted although an article failed:\n" + out[-1500:])
    check(glob.glob(os.path.join(tmp, "*", "*.7z")), "the archive was deleted although an article failed")
    check("have NOT been deleted" in out, "no message about the files that were kept")


def test_repost_cleans_and_deletes_originals(work):
    # once the missing articles are posted: archives removed and (as asked) the original file deleted
    if not shutil.which("7z"):
        print("SKIP  (7z is not installed)")
        return
    r, src, tmp = _post_and_delete(work, repost=True)
    out = r.stdout + r.stderr
    check("All the Articles are now posted" in out, "the repost did not finish cleanly:\n" + out[-1500:])
    check(not os.path.exists(src), "the original file was not deleted after the repost")
    check(not glob.glob(os.path.join(tmp, "*", "*")), "the archive was not deleted after the repost")


# ---------------------------------------------------------------- persistent queue
def queue_entries(work):
    import json
    path = os.path.join(work, "ngPost_queue.json")
    if not os.path.exists(path):
        return []
    with open(path) as f:
        return json.load(f)["posts"]


def start_and_kill(stub, work, inputs, nzb, after, conf_extra="", extra=()):
    """start a post and kill ngPost (SIGKILL, like a crash) once the server has accepted `after` articles"""
    import time
    cmd, env = ngpost_conf_cmd([stub], work, inputs, nzb, conns=1, threads=1, extra=extra, conf_extra=conf_extra)
    proc = subprocess.Popen(cmd, env=env, cwd=work, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    t0 = time.time()
    while len(stub.accepted) < after and time.time() - t0 < 60 and proc.poll() is None:
        time.sleep(0.02)
    check(proc.poll() is None, "ngPost finished before it could be interrupted")
    proc.kill()
    proc.wait()


def slow(article, attempt):
    import time
    time.sleep(0.15)
    return None


def test_queue_resume_after_crash(work):
    # ngPost is killed in the middle of a compressed post: the post stays in the queue, a normal start
    # only says so, and --resume_queue posts it again from the beginning (new archive, nzb overwritten)
    seven = shutil.which("7z")
    if not seven:
        print("SKIP  (7z is not installed)")
        return
    tmp = os.path.join(work, "tmp")
    os.makedirs(tmp)
    conf_extra = "TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (tmp, seven)
    src = os.path.join(work, "crash.bin")
    data = make_file(src, 600_000, 51)
    nzb = os.path.join(work, "crash.nzb")

    stub = NntpStub(policy=slow).start()
    try:
        start_and_kill(stub, work, [src], nzb, after=3, conf_extra=conf_extra,
                       extra=["--compress", "--rar_name", "crasharch", "--rar_pass", "secret"])
    finally:
        stub.stop()
    entries = queue_entries(work)
    check(len(entries) == 1 and entries[0]["state"] == "waiting", "the interrupted post is not in the queue: %s" % entries)
    check(entries[0]["rarPass"] == "secret", "the archive password was not saved")
    check(os.path.isdir(os.path.join(tmp, "crasharch")), "the archive of the interrupted post is not there (test setup)")

    # a normal post does not touch the waiting post, it only tells about it
    stub = NntpStub().start()
    try:
        other = os.path.join(work, "other.bin")
        make_file(other, 100_000, 52)
        r = run_ngpost_conf([stub], work, [other], os.path.join(work, "other.nzb"), conns=1, threads=1)
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("--resume_queue" in out, "nothing said about the unfinished post:\n" + out[-1500:])
        check(len(queue_entries(work)) == 1, "the queue changed: %s" % queue_entries(work))
    finally:
        stub.stop()

    # resume
    stub = NntpStub().start()
    try:
        r = run_ngpost_conf([stub], work, [], None, conns=2, threads=1, extra=["--resume_queue"],
                            conf_extra=conf_extra)
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, out[-2000:]))
        check("posting again from the beginning" in out, "the post was not started again")
        check(queue_entries(work) == [], "the queue is not empty after the post: %s" % queue_entries(work))
        common_checks(stub, nzb, None)
        rebuilt = stub.rebuild_files()
        check(set(rebuilt) == {"crasharch.7z"}, "files on the server: %s" % sorted(rebuilt))
        out_dir = os.path.join(work, "extract")
        os.makedirs(out_dir)
        arch = os.path.join(work, "check.7z")
        with open(arch, "wb") as f:
            f.write(rebuilt["crasharch.7z"])
        subprocess.run([seven, "x", "-psecret", "-o" + out_dir, arch], capture_output=True, check=True)
        with open(os.path.join(out_dir, "crash.bin"), "rb") as f:
            check(sha(f.read()) == sha(data), "the file in the archive is different")
    finally:
        stub.stop()


def test_queue_file_gone(work):
    # the file of an unfinished post has been deleted: the post is dropped with a clear message
    src = os.path.join(work, "gone.bin")
    make_file(src, 400_000, 53)
    stub = NntpStub(policy=slow).start()
    try:
        start_and_kill(stub, work, [src], os.path.join(work, "gone.nzb"), after=2)
    finally:
        stub.stop()
    check(len(queue_entries(work)) == 1, "the interrupted post is not in the queue")
    os.remove(src)
    stub = NntpStub().start()
    try:
        r = run_ngpost_conf([stub], work, [], None, extra=["--resume_queue"])
        out = r.stdout + r.stderr
        check("not there anymore" in out, "no message about the missing file:\n" + out[-1500:])
        check(queue_entries(work) == [], "the post is still in the queue")
        check(len(stub.accepted) == 0, "something was posted")
    finally:
        stub.stop()


def test_queue_missing_articles(work):
    # a post ends with an Article that couldn't be posted: it is kept in the queue as "missing",
    # and --resume_queue posts only that Article and fixes the nzb
    stub1 = NntpStub(policy=lambda article, attempt: b"441 posting failed" if article.part == 2 else None).start()
    src = os.path.join(work, "miss.bin")
    data = make_file(src, 200_000, 54)
    nzb = os.path.join(work, "miss.nzb")
    try:
        r = run_ngpost_conf([stub1], work, [src], nzb, conns=2, threads=1,
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "500"})
        check(r.returncode != 0, "exit code 0 although an article failed")
    finally:
        stub1.stop()
    entries = queue_entries(work)
    check(len(entries) == 1 and entries[0]["state"] == "missing", "the missing Article is not in the queue: %s" % entries)

    stub2 = NntpStub().start()
    try:
        r = run_ngpost_conf([stub2], work, [], None, extra=["--resume_queue"])
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, out[-2000:]))
        check("1 missing Articles posted" in out, "the missing Article was not posted:\n" + out[-1500:])
        check(len(stub2.accepted) == 1, "%d articles posted, expected 1" % len(stub2.accepted))
        check(queue_entries(work) == [], "the queue is not empty")
        both = merged_accepted([stub1, stub2])
        common_checks(both, nzb, {"miss.bin": data})
    finally:
        stub2.stop()


def test_queue_used_by_another_ngpost(work):
    # two ngPost with the same config: only the first one uses the queue
    import time
    src = os.path.join(work, "first.bin")
    make_file(src, 400_000, 55)
    stub = NntpStub(policy=slow).start()
    cmd, env = ngpost_conf_cmd([stub], work, [src], os.path.join(work, "first.nzb"), conns=1, threads=1)
    first = subprocess.Popen(cmd, env=env, cwd=work, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    try:
        t0 = time.time()
        while len(stub.accepted) < 1 and time.time() - t0 < 30:
            time.sleep(0.02)
        second = os.path.join(work, "second.bin")
        make_file(second, 60_000, 56)
        r = run_ngpost_conf([stub], work, [second], os.path.join(work, "second.nzb"), conns=1, threads=1)
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("used by another ngPost" in out, "no message about the queue in use:\n" + out[-1500:])
        entries = queue_entries(work)
        check(len(entries) == 1 and entries[0]["files"] == [src], "the second ngPost wrote in the queue: %s" % entries)
    finally:
        first.kill()
        first.wait()
        stub.stop()


def test_phase_durations(work):
    # a compressed post tells how long each phase took, a plain post has no such summary
    seven = shutil.which("7z")
    if not seven:
        print("SKIP  (7z is not installed)")
        return
    tmp = os.path.join(work, "tmp")
    os.makedirs(tmp)
    conf_extra = "TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (tmp, seven)
    src = os.path.join(work, "dur.bin")
    make_file(src, 300_000, 61)
    stub = NntpStub().start()
    try:
        r = run_ngpost_conf([stub], work, [src], os.path.join(work, "dur.nzb"), conns=1, threads=1,
                            conf_extra=conf_extra, extra=["--compress", "--rar_name", "durarch"])
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("Compression done in" in out, "no end of compression line:\n" + out[-1500:])
        check("Compression " in out and "Upload " in out and "Total " in out,
              "no summary of the durations:\n" + out[-1500:])

        plain = os.path.join(work, "plain.bin")
        make_file(plain, 100_000, 62)
        r = run_ngpost_conf([stub], work, [plain], os.path.join(work, "plain.nzb"), conns=1, threads=1)
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("Total " not in out, "a plain post should not print the summary:\n" + out[-1500:])
    finally:
        stub.stop()


def test_progress_line_has_eta(work):
    # the progress line of the command line tells the remaining time (smoothed on the last seconds)
    src = os.path.join(work, "eta.bin")
    make_file(src, 1_000_000, 71)
    stub = NntpStub(policy=slow).start()
    try:
        r = run_ngpost_conf([stub], work, [src], os.path.join(work, "eta.nzb"), conns=1, threads=1,
                            extra=["--disp_progress", "BAR"])
        out = r.stdout + r.stderr
        check(r.returncode == 0, "exit code %d" % r.returncode)
        check("ETA " in out, "no remaining time in the progress line:\n" + out[-1500:])
        check("nan" not in out.lower(), "invalid number in the progress line:\n" + out[-1500:])
        check("100 %" in out, "the progress line did not reach 100 %:\n" + out[-1500:])
    finally:
        stub.stop()


def test_groups_are_cleaned(work):
    # spaces around the groups, empty names and doubles are removed (a space in a group name is refused by the server)
    import re
    src = os.path.join(work, "grp.bin")
    make_file(src, 100_000, 81)
    nzb = os.path.join(work, "grp.nzb")
    stub = NntpStub().start()
    try:
        r = run_ngpost_conf([stub], work, [src], nzb, conns=1, threads=1,
                            extra=["-g", "alt.test.one, alt.test.two ,, alt.test.one"])
        check(r.returncode == 0, "exit code %d" % r.returncode)
        groups = sorted(set(re.findall(r"<group>(.*?)</group>", open(nzb).read())))
        check(groups == ["alt.test.one", "alt.test.two"], "groups in the nzb: %s" % groups)
        for art in stub.accepted.values():
            check(all(g == g.strip() and g for g in art.groups), "group with a space: %s" % art.groups)
    finally:
        stub.stop()


TESTS = [test_single_file, test_several_files_and_folder, test_obfuscation,
         test_retry_after_refusals, test_wrong_password, test_some_logins_refused,
         test_article_always_refused_on_one_connection, test_server_going_down,
         test_other_server_used, test_second_round, test_too_many_failures,
         test_repost_missing, test_repost_nothing_to_do, test_repost_still_failing,
         test_originals_kept_when_articles_failed, test_repost_cleans_and_deletes_originals,
         test_queue_resume_after_crash, test_queue_file_gone, test_queue_missing_articles,
         test_queue_used_by_another_ngpost, test_phase_durations, test_progress_line_has_eta, test_groups_are_cleaned]


def main():
    global NGPOST
    if len(sys.argv) != 2 or not os.access(sys.argv[1], os.X_OK):
        sys.exit(__doc__)
    NGPOST = os.path.abspath(sys.argv[1])
    failed = 0
    for t in TESTS:
        with tempfile.TemporaryDirectory(prefix="ngpost-test-") as work:
            try:
                t(work)
                print("PASS  %s" % t.__name__)
            except Exception as e:  # noqa: BLE001
                failed += 1
                print("FAIL  %s: %s" % (t.__name__, e))
                if not isinstance(e, AssertionError):
                    traceback.print_exc()
                print("---- output of the last ngPost run ----\n%s----" % LAST_OUTPUT[-9000:])
    print("\n%d/%d tests passed" % (len(TESTS) - failed, len(TESTS)))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
