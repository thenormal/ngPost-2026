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


def test_latency_is_measured(work):
    # the end of a post tells the round trip (POST -> 340): about the delay of the server, not of the size of the data
    import re

    def median_round_trip(delay, name, seed):
        src = os.path.join(work, name + ".bin")
        make_file(src, 200_000, seed)
        stub = NntpStub(post_delay=delay).start()
        try:
            r = run_ngpost_conf([stub], work, [src], os.path.join(work, name + ".nzb"), conns=1, threads=1)
            out = r.stdout + r.stderr
            check(r.returncode == 0, "exit code %d" % r.returncode)
            m = re.search(r"Latency \(median.*?round trip ([0-9.]+) ms", out)
            check(m is not None, "no latency line:\n" + out[-1500:])
            return float(m.group(1))
        finally:
            stub.stop()

    near = median_round_trip(0, "near", 91)
    far = median_round_trip(0.3, "far", 92)
    check(near < 200, "round trip of a near server: %s ms" % near)
    check(250 <= far < 1500, "round trip of a server with a delay of 300 ms: %s ms" % far)


def test_help_pages(work):
    # the pages of the Help section: titled, linked together, and all embedded in the program
    import re
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    help_dir = os.path.join(root, "src", "resources", "help")
    en = sorted(f for f in os.listdir(os.path.join(help_dir, "en")) if f.endswith(".md"))
    check(len(en) >= 13, "only %d pages" % len(en))
    qrc = open(os.path.join(root, "src", "resources", "resources.qrc")).read()
    for lang in sorted(os.listdir(help_dir)):
        for f in sorted(os.listdir(os.path.join(help_dir, lang))):
            text = open(os.path.join(help_dir, lang, f), encoding="utf-8").read()
            check("<file>help/%s/%s</file>" % (lang, f) in qrc, "%s/%s is not in resources.qrc" % (lang, f))
            check(text.lstrip().startswith("# "), "%s/%s has no title" % (lang, f))
            if lang != "en":
                check(f in en, "%s/%s has no English page" % (lang, f))
            for link in re.findall(r"\]\(([^)]+)\)", text):
                if not link.startswith("http"):
                    check(link in en, "%s/%s links to a missing page: %s" % (lang, f, link))


def test_feedback_forms(work):
    # the Feedback window opens a prefilled issue: its template names and field ids must exist in the issue forms
    import re
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    code = open(os.path.join(root, "src", "hmi", "FeedbackDialog.cpp"), encoding="utf-8").read()
    templates = re.findall(r'"(\w+\.yml)"', code)
    check(sorted(templates) == ["bug_report.yml", "feature_request.yml"], "templates used: %s" % templates)
    fields = re.findall(r'addQueryItem\("(?!template)([a-z]+)"', code)
    check(sorted(fields) == ["system", "version"], "prefilled fields: %s" % fields)
    for t in templates:
        form = open(os.path.join(root, ".github", "ISSUE_TEMPLATE", t), encoding="utf-8").read()
        ids = re.findall(r"^\s+id: (\w+)", form, re.M)
        for f in fields:
            check(f in ids, "%s has no field '%s'" % (t, f))
        check(re.search(r"^name: .+", form, re.M) and re.search(r"^description: .+", form, re.M), "%s: name/description missing" % t)
    cfg = open(os.path.join(root, ".github", "ISSUE_TEMPLATE", "config.yml"), encoding="utf-8").read()
    check("blank_issues_enabled: false" in cfg, "blank issues are not disabled")


def test_duplicate_is_skipped(work):
    # a post already in the history (same nzb name and size) is skipped, unless --allow_repost; a different size is posted
    stub = NntpStub().start()
    try:
        src = os.path.join(work, "movie.bin")
        make_file(src, 200_000, 7)
        hist = os.path.join(work, "history.csv")
        nzb = os.path.join(work, "out.nzb")
        conf = os.path.join(work, "dup.conf")
        with open(conf, "w") as f:
            f.write("POST_HISTORY = %s\n" % hist)
        extra = ["--conf", conf]
        r = run_ngpost(stub, work, [src], nzb, extra=extra)
        check(r.returncode == 0 and os.path.exists(hist), "first post failed\n%s%s" % (r.stdout, r.stderr))
        nb = len(stub.accepted)
        check(nb > 0, "nothing posted")

        os.remove(nzb)
        r = run_ngpost(stub, work, [src], nzb, extra=extra, timeout=30)
        check("already in the history" in LAST_OUTPUT, "no skip message\n" + LAST_OUTPUT)
        check(len(stub.accepted) == nb and not os.path.exists(nzb), "the duplicate was posted")

        r = run_ngpost(stub, work, [src], nzb, extra=extra + ["--allow_repost"])
        check(r.returncode == 0 and len(stub.accepted) == 2 * nb, "--allow_repost did not post again\n" + LAST_OUTPUT)

        make_file(src, 200_001, 8)   # same name, other size: not the same post
        r = run_ngpost(stub, work, [src], nzb, extra=extra)
        check(len(stub.accepted) > 2 * nb, "a different size was skipped\n" + LAST_OUTPUT)

        # a compressed post is recognised too (the history keeps the size of the original files, not of the archives)
        seven = shutil.which("7z")
        if seven:
            tmp = os.path.join(work, "tmp")
            os.makedirs(tmp)
            conf_extra = "POST_HISTORY = %s\nTMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (hist, tmp, seven)
            clip = os.path.join(work, "clip.bin")
            make_file(clip, 150_000, 9)
            nzb2 = os.path.join(work, "clip.nzb")
            cextra = ["--compress", "--rar_name", "cliparch"]
            r = run_ngpost_conf([stub], work, [clip], nzb2, conns=1, threads=1, conf_extra=conf_extra, extra=cextra)
            check(r.returncode == 0, "compressed post failed\n" + LAST_OUTPUT)
            nb2 = len(stub.accepted)
            os.remove(nzb2)
            r = run_ngpost_conf([stub], work, [clip], nzb2, conns=1, threads=1, conf_extra=conf_extra, extra=cextra, timeout=30)
            check("already in the history" in LAST_OUTPUT and len(stub.accepted) == nb2,
                  "the compressed duplicate was not skipped\n" + LAST_OUTPUT)
    finally:
        stub.stop()


def test_nfo_file(work):
    # the NFO: next to the nzb, with the password only if asked, in the post if asked, no names when they are hidden
    stub = NntpStub().start()
    try:
        folder = os.path.join(work, "album")
        os.makedirs(folder)
        make_file(os.path.join(folder, "track1.dat"), 120_000, 21)
        make_file(os.path.join(folder, "track2.dat"), 80_000, 22)
        single = os.path.join(work, "movie.bin")
        make_file(single, 100_000, 23)
        text = os.path.join(work, "note.txt")
        with open(text, "w") as f:
            f.write("Hello from the poster\nsecond line")

        # 1. next to the nzb, not in the post, with the free text
        nzb = os.path.join(work, "one.nzb")
        r = run_ngpost(stub, work, [single, folder], nzb, extra=["--gen_nfo", "--nfo_text", text])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        nfo = os.path.join(work, "one.nfo")
        check(os.path.exists(nfo), "no NFO next to the nzb\n" + LAST_OUTPUT)
        content = open(nfo, encoding="utf-8").read()
        for needle in ("one", "movie.bin", "track1.dat", "track2.dat", "3 file(s)", "Hello from the poster", "second line"):
            check(needle in content, "'%s' is missing in the NFO:\n%s" % (needle, content))
        check("Password" not in content, "a password is in the NFO:\n" + content)
        check(".nfo" not in open(nzb, encoding="utf-8").read(), "the NFO is in the nzb but was not asked")
        nb = len(stub.accepted)

        # 2. in the post: the nzb lists it and more articles are posted
        nzb2 = os.path.join(work, "two.nzb")
        r = run_ngpost(stub, work, [single], nzb2, extra=["--nfo_in_post"])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        check(os.path.exists(os.path.join(work, "two.nfo")), "no NFO next to the nzb (nfo_in_post)")
        check("two.nfo" in open(nzb2, encoding="utf-8").read(), "the NFO is not in the nzb")

        # 3. file names hidden: the NFO in the post must not list them
        nzb3 = os.path.join(work, "three.nzb")
        r = run_ngpost(stub, work, [single], nzb3, extra=["--nfo_in_post", "--obfuscate"])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        hidden = open(os.path.join(work, "three.nfo"), encoding="utf-8").read()
        check("movie.bin" not in hidden and "hidden" in hidden, "names are shown in a post that hides them:\n" + hidden)

        # 4. password only with --nfo_password (compressed post)
        seven = shutil.which("7z")
        if seven:
            tmp = os.path.join(work, "tmp")
            os.makedirs(tmp)
            conf_extra = "TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (tmp, seven)
            for flag, expected in (([], False), (["--nfo_password"], True)):
                nzb4 = os.path.join(work, "pw%d.nzb" % expected)
                r = run_ngpost_conf([stub], work, [single], nzb4, conns=1, threads=1, conf_extra=conf_extra,
                                    extra=["--compress", "--rar_name", "nfoarch", "--rar_pass", "topsecret", "--gen_nfo"] + flag)
                check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
                c = open(os.path.join(work, "pw%d.nfo" % expected), encoding="utf-8").read()
                check(("topsecret" in c) == expected, "password in the NFO: %s (expected %s)\n%s" % ("topsecret" in c, expected, c))
                check("Archive" in c, "no archive line in the NFO:\n" + c)
    finally:
        stub.stop()


def test_nfo_media_info(work):
    # MediaInfo in the NFO: layout, 20 files max, no names when hidden, program missing, can be switched off, real MediaInfo
    import json
    stub = NntpStub().start()
    try:
        fake = os.path.join(work, "fakemediainfo")
        marker = os.path.join(work, "fake_was_called")
        with open(fake, "w") as f:
            f.write("""#!/usr/bin/env python3
import sys, json
open(%r, "a").write(sys.argv[-1] + "\\n")
tracks = [{"@type": "General", "Duration": "6130.4", "FileSize": "8800000000"},
          {"@type": "Video", "Format": "HEVC", "Format_Profile": "Main 10@L5.1@Main", "Width": "3840", "Height": "2160",
           "FrameRate": "23.976", "HDR_Format": "SMPTE ST 2086", "HDR_Format_Compatibility": "HDR10"},
          {"@type": "Audio", "Language": "en", "Format": "E-AC-3", "Channels": "6", "BitRate": "640000"},
          {"@type": "Audio", "Language": "it", "Format": "AC-3", "Channels": "2", "BitRate": "192000"},
          {"@type": "Text", "Language": "en", "Format": "UTF-8"},
          {"@type": "Text", "Language": "it", "Format": "PGS"}]
print(json.dumps({"media": {"@ref": sys.argv[-1], "track": tracks}}))
""" % marker)
        os.chmod(fake, 0o755)

        movie = os.path.join(work, "movie.mkv")
        make_file(movie, 100_000, 31)
        other = os.path.join(work, "readme.dat")
        make_file(other, 10_000, 32)

        # 1. layout
        nzb = os.path.join(work, "m1.nzb")
        r = run_ngpost(stub, work, [movie, other], nzb, extra=["--gen_nfo", "--mediainfo_path", fake])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        c = open(os.path.join(work, "m1.nfo"), encoding="utf-8").read()
        for needle in ("Media information", "movie.mkv - 01:42:10, 8.20 GB", "Video     : HEVC Main 10, 3840x2160, 23.976 fps, HDR10",
                       "Audio     : English, E-AC-3, 5.1, 640 kb/s", "Audio     : Italian, AC-3, 2.0, 192 kb/s",
                       "Subtitles : English (SRT), Italian (PGS)"):
            check(needle in c, "'%s' is missing in the NFO:\n%s" % (needle, c))
        check("readme.dat -" not in c, "a file that is not a media file is described:\n" + c)
        with open(marker) as f:
            check(f.read().count("\n") == 1, "MediaInfo should only be run for the media file")

        # 2. 20 files at most
        many = os.path.join(work, "many")
        os.makedirs(many)
        for i in range(22):
            make_file(os.path.join(many, "e%02d.mkv" % i), 5_000, 40 + i)
        nzb = os.path.join(work, "m2.nzb")
        r = run_ngpost(stub, work, [many], nzb, extra=["--gen_nfo", "--mediainfo_path", fake])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        c = open(os.path.join(work, "m2.nfo"), encoding="utf-8").read()
        check(c.count("Video     :") == 20, "20 files should be described, got %d" % c.count("Video     :"))
        check("... and 2 more media file(s)" in c, "the note about the other files is missing:\n" + c)

        # 3. names hidden: not written, MediaInfo not even run
        os.remove(marker)
        nzb = os.path.join(work, "m3.nzb")
        r = run_ngpost(stub, work, [movie], nzb, extra=["--nfo_in_post", "--obfuscate", "--mediainfo_path", fake])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        c = open(os.path.join(work, "m3.nfo"), encoding="utf-8").read()
        check("movie.mkv" not in c and "HEVC" not in c and "not written" in c, "media information with hidden names:\n" + c)
        check(not os.path.exists(marker), "MediaInfo was run although the names are hidden")

        # 4. switched off
        nzb = os.path.join(work, "m4.nzb")
        r = run_ngpost(stub, work, [movie], nzb, extra=["--gen_nfo", "--no_nfo_media", "--mediainfo_path", fake])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        c = open(os.path.join(work, "m4.nfo"), encoding="utf-8").read()
        check("Media information" not in c, "media information although --no_nfo_media:\n" + c)
        check(not os.path.exists(marker), "MediaInfo was run although --no_nfo_media")

        # 5. program not found: the post works, the NFO has no media section, one line in the log
        nzb = os.path.join(work, "m5.nzb")
        r = run_ngpost(stub, work, [movie], nzb, extra=["--gen_nfo", "--mediainfo_path", os.path.join(work, "nothing_here")])
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        c = open(os.path.join(work, "m5.nfo"), encoding="utf-8").read()
        check("movie.mkv" in c and "Media information" not in c, "unexpected NFO without MediaInfo:\n" + c)
        check("MediaInfo was not found" in LAST_OUTPUT, "no message about the missing MediaInfo\n" + LAST_OUTPUT)

        # 6. the real MediaInfo, on a small video made by ffmpeg
        real, ffmpeg = shutil.which("mediainfo"), shutil.which("ffmpeg")
        if real and ffmpeg:
            video = os.path.join(work, "clip.mkv")
            subprocess.run([ffmpeg, "-loglevel", "error", "-f", "lavfi", "-i", "testsrc=d=1:s=320x240:r=25",
                            "-f", "lavfi", "-i", "sine=d=1", "-c:v", "mpeg4", "-c:a", "mp3",
                            "-metadata:s:a:0", "language=ita", video], check=True)
            nzb = os.path.join(work, "m6.nzb")
            r = run_ngpost(stub, work, [video], nzb, extra=["--gen_nfo"])
            check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
            c = open(os.path.join(work, "m6.nfo"), encoding="utf-8").read()
            for needle in ("clip.mkv - 00:00:01", "320x240", "25 fps", "Italian"):
                check(needle in c, "'%s' is missing in the NFO made with the real MediaInfo:\n%s" % (needle, c))
        else:
            print("      (real MediaInfo or ffmpeg not installed: this part is skipped)")
    finally:
        stub.stop()


def test_profiles_and_par2_default(work):
    # the profiles of the GUI are lines "profile = ..." of the config file: bad lines are ignored, the post works;
    # par2 starts at 10 %: --gen_par2 without a percentage is no longer refused for that reason
    stub = NntpStub().start()
    try:
        f = os.path.join(work, "data.bin")
        make_file(f, 50_000, 51)
        conf_extra = "profile = Film|1|1|0|1|5|1\nprofile = broken line\nprofile = |1|1|1|1|10|0\nprofile = Big|1|1|1|1|900|0\n"
        nzb = os.path.join(work, "prof.nzb")
        r = run_ngpost_conf([stub], work, [f], nzb, conns=1, threads=1, conf_extra=conf_extra)
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        check(os.path.exists(nzb), "no nzb")

        nzb2 = os.path.join(work, "par.nzb")
        r = run_ngpost_conf([stub], work, [f], nzb2, conns=1, threads=1, extra=["--gen_par2"])
        check("redundancy percentage is null" not in LAST_OUTPUT, "par2 should start at 10 %\n" + LAST_OUTPUT)
    finally:
        stub.stop()


def test_post_checker_unit(work):
    # the checks done before a Quick Post (logic without GUI): a small program is built with qmake and run
    qmake = shutil.which("qmake") or shutil.which("qmake-qt5")
    if not qmake:
        print("      (qmake not found: this test is skipped)")
        return
    here = os.path.dirname(os.path.abspath(__file__))
    build = os.path.join(work, "unit_build")
    os.makedirs(build)
    r = subprocess.run([qmake, os.path.join(here, "unit", "post_checker_test.pro")], cwd=build, capture_output=True, text=True)
    check(r.returncode == 0, "qmake failed\n" + r.stdout + r.stderr)
    r = subprocess.run(["make", "-j4"], cwd=build, capture_output=True, text=True)
    check(r.returncode == 0, "build failed\n" + r.stdout[-2000:] + r.stderr[-2000:])
    r = subprocess.run([os.path.join(build, "post_checker_test")], capture_output=True, text=True)
    check(r.returncode == 0, "unit test failed\n" + r.stdout + r.stderr)


def _post_and_move(work, fail, extra_name_in_posted=False):
    """--auto --compress --move_posted Posted, with an article that fails everywhere if fail is on"""
    stub = NntpStub(policy=lambda article, attempt: b"441 posting failed" if fail and article.part == 2 else None).start()
    try:
        folder = os.path.join(work, "in")
        tmp = os.path.join(work, "tmp")
        os.makedirs(os.path.join(folder, "Posted"))
        os.makedirs(tmp)
        make_file(os.path.join(folder, "Posted", "old.bin"), 1000, 41)   # an old post: must be ignored
        src = os.path.join(folder, "mv.bin")
        make_file(src, 200_000, 42)
        if extra_name_in_posted:
            make_file(os.path.join(folder, "Posted", "mv.bin"), 1234, 43)  # same name already there
        r = run_ngpost_conf([stub], work, [], os.path.join(work, "out"), conns=2, threads=1,
                            extra=["--auto", folder, "--compress", "--move_posted", "Posted"],
                            env_extra={"NGPOST_SECOND_ROUND_DELAY_MS": "500"},
                            conf_extra="TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\n" % (tmp, shutil.which("7z")))
        return r, folder, src
    finally:
        stub.stop()


def test_move_posted(work):
    # --move_posted: the posted file goes to the Posted folder next to it, the Posted folder is not posted itself
    if not shutil.which("7z"):
        print("SKIP  (7z is not installed)")
        return
    r, folder, src = _post_and_move(work, fail=False)
    out = r.stdout + r.stderr
    check(r.returncode == 0, "the post failed:\n" + out[-1500:])
    check(not os.path.exists(src), "the file is still in the folder:\n" + out[-1500:])
    check(os.path.exists(os.path.join(folder, "Posted", "mv.bin")), "the file is not in the Posted folder")
    check(os.path.exists(os.path.join(folder, "Posted", "old.bin")), "the old content of the Posted folder was touched")
    check("Moved the posted file" in out, "no message about the move")
    nzbs = glob.glob(os.path.join(work, "**", "*.nzb"), recursive=True)
    check(not any("Posted" in os.path.basename(n) for n in nzbs), "the Posted folder was posted as a post: %s" % nzbs)


def test_move_posted_never_overwrites(work):
    if not shutil.which("7z"):
        print("SKIP  (7z is not installed)")
        return
    r, folder, src = _post_and_move(work, fail=False, extra_name_in_posted=True)
    check(r.returncode == 0, "the post failed:\n" + (r.stdout + r.stderr)[-1500:])
    check(os.path.getsize(os.path.join(folder, "Posted", "mv.bin")) == 1234, "a file of the Posted folder was overwritten")
    check(os.path.getsize(os.path.join(folder, "Posted", "mv (2).bin")) == 200_000, "the file was not moved as 'mv (2).bin'")


def test_move_posted_kept_when_articles_failed(work):
    if not shutil.which("7z"):
        print("SKIP  (7z is not installed)")
        return
    r, folder, src = _post_and_move(work, fail=True)
    out = r.stdout + r.stderr
    check(os.path.exists(src), "the file was moved although an article failed:\n" + out[-1500:])
    check(not os.path.exists(os.path.join(folder, "Posted", "mv.bin")), "a copy is in the Posted folder")
    check("have NOT been moved" in out, "no message about the file that was kept")


def test_move_posted_options(work):
    # --move_posted needs --auto or --monitor, and can't be used with --rm_posted
    stub = NntpStub().start()
    try:
        folder = os.path.join(work, "in")
        os.makedirs(folder)
        src = os.path.join(work, "x.bin")
        make_file(src, 1000, 1)
        r = run_ngpost_conf([stub], work, [src], os.path.join(work, "o"), extra=["--move_posted", "Posted"])
        check(r.returncode != 0 and "--move_posted option is only available" in r.stdout + r.stderr,
              "--move_posted accepted without --auto:\n" + (r.stdout + r.stderr)[-800:])
        r = run_ngpost_conf([stub], work, [], os.path.join(work, "o"),
                            extra=["--auto", folder, "--compress", "--rm_posted", "--move_posted", "Posted"])
        check(r.returncode != 0 and "can't be used together" in r.stdout + r.stderr,
              "--rm_posted with --move_posted accepted:\n" + (r.stdout + r.stderr)[-800:])
    finally:
        stub.stop()


def test_posted_mover_unit(work):
    # the move of the posted files (names, other disk, never overwrite): a small program built with qmake
    qmake = shutil.which("qmake") or shutil.which("qmake-qt5")
    if not qmake:
        print("      (qmake not found: this test is skipped)")
        return
    here = os.path.dirname(os.path.abspath(__file__))
    build = os.path.join(work, "unit_mover")
    os.makedirs(build)
    r = subprocess.run([qmake, os.path.join(here, "unit", "posted_mover_test.pro")], cwd=build, capture_output=True, text=True)
    check(r.returncode == 0, "qmake failed\n" + r.stdout + r.stderr)
    r = subprocess.run(["make", "-j4"], cwd=build, capture_output=True, text=True)
    check(r.returncode == 0, "build failed\n" + r.stdout[-2000:] + r.stderr[-2000:])
    r = subprocess.run([os.path.join(build, "posted_mover_test")], capture_output=True, text=True)
    check(r.returncode == 0, "unit test failed\n" + r.stdout + r.stderr)


class HttpSink:
    """small HTTP server that keeps the POST requests it receives (for the notifications)"""
    def __init__(self):
        import http.server
        import threading
        self.requests = []
        sink = self

        class Handler(http.server.BaseHTTPRequestHandler):
            def do_POST(self):
                n = int(self.headers.get("Content-Length", 0))
                sink.requests.append((self.path, self.rfile.read(n).decode("utf-8")))
                self.send_response(200)
                self.send_header("Content-Length", "0")
                self.end_headers()

            def log_message(self, *a):
                pass

        self.server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        self.port = self.server.server_address[1]
        threading.Thread(target=self.server.serve_forever, daemon=True).start()

    def stop(self):
        self.server.shutdown()
        self.server.server_close()


def test_notifications(work):
    # a message when the posts end: generic webhook, Discord, Telegram; nothing is sent when nothing is set
    import json
    stub = NntpStub().start()
    sink = HttpSink()
    try:
        src = os.path.join(work, "notif.bin")
        make_file(src, 90_000, 31)
        base = "http://127.0.0.1:%d" % sink.port

        # nothing configured: nothing sent
        run_ngpost_conf([stub], work, [src], os.path.join(work, "n0.nzb"), conns=1, threads=1)
        check(not sink.requests, "a notification was sent without any setting")

        # generic webhook, after each post
        r = run_ngpost_conf([stub], work, [src], os.path.join(work, "n1.nzb"), conns=1, threads=1,
                            conf_extra="notify_webhook = %s/hook\nnotify_when = each\n" % base)
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        check(len(sink.requests) == 1 and sink.requests[0][0] == "/hook", "webhook requests: %s" % sink.requests)
        body = json.loads(sink.requests[0][1])
        check("n1.nzb" in body["text"] and "finished" in body["title"], "unexpected message: %s" % body)

        # Discord: {"content": ...}
        sink.requests.clear()
        run_ngpost_conf([stub], work, [src], os.path.join(work, "n2.nzb"), conns=1, threads=1,
                        conf_extra="notify_discord = %s/discord\n" % base)
        check(len(sink.requests) == 1 and "n2.nzb" in json.loads(sink.requests[0][1]).get("content", ""),
              "Discord requests: %s" % sink.requests)

        # Telegram: the address of the API is replaced for the test
        sink.requests.clear()
        cmd, env = ngpost_conf_cmd([stub], work, [src], os.path.join(work, "n3.nzb"), 1, 1,
                                   env_extra={"NGPOST_TELEGRAM_API": base},
                                   conf_extra="notify_telegram_token = tok123\nnotify_telegram_chat = 4242\n")
        subprocess.run(cmd, env=env, cwd=work, capture_output=True, text=True, timeout=120)
        check(len(sink.requests) == 1 and sink.requests[0][0] == "/bottok123/sendMessage", "Telegram requests: %s" % sink.requests)
        check(json.loads(sink.requests[0][1])["chat_id"] == "4242", "wrong chat id")

        # the notifications of errors can be turned off, the post is not affected
        sink.requests.clear()
        run_ngpost_conf([stub], work, [src], os.path.join(work, "n4.nzb"), conns=1, threads=1,
                        conf_extra="notify_webhook = %s/hook\nnotify_ok = off\n" % base)
        check(not sink.requests, "a success was notified with notify_ok = off")

        # the server of the notifications is down: the post still ends well
        dead = HttpSink()
        dead_port = dead.port
        dead.stop()
        r = run_ngpost_conf([stub], work, [src], os.path.join(work, "n5.nzb"), conns=1, threads=1,
                            conf_extra="notify_webhook = http://127.0.0.1:%d/x\n" % dead_port)
        check(r.returncode == 0 and os.path.exists(os.path.join(work, "n5.nzb")), "the post failed because of the notification\n" + LAST_OUTPUT)
    finally:
        sink.stop()
        stub.stop()


def test_notification_when_all_done(work):
    # two posts (auto posting) => ONE message at the end by default, two with notify_when = each; no password in it
    import json
    seven = shutil.which("7z")
    if not seven:
        print("SKIP  (7z is not installed)")
        return
    stub = NntpStub().start()
    sink = HttpSink()
    try:
        tmp = os.path.join(work, "tmp")
        os.makedirs(tmp)
        os.makedirs(os.path.join(work, "inbox"))
        for name, seed in (("alpha", 41), ("beta", 42)):
            make_file(os.path.join(work, "inbox", name + ".bin"), 60_000, seed)
        nzbdir = os.path.join(work, "nzbs")
        os.makedirs(nzbdir)
        base = "TMP_DIR = %s\nRAR_PATH = %s\nRAR_SIZE = 0\nnzbPath = %s\n" % (tmp, seven, nzbdir)
        hook = "notify_webhook = http://127.0.0.1:%d/hook\n" % sink.port
        extra = ["--auto", os.path.join(work, "inbox"), "--compress", "--rar_pass", "topsecret"]
        r = run_ngpost_conf([stub], work, [], None, conns=1, threads=1, conf_extra=base + hook, extra=extra)
        check(r.returncode == 0, "exit code %d\n%s" % (r.returncode, LAST_OUTPUT))
        check(len(sink.requests) == 1, "%d messages instead of one: %s" % (len(sink.requests), sink.requests))
        text = json.loads(sink.requests[0][1])
        check("alpha" in text["text"] and "beta" in text["text"] and "2 post" in text["title"], "unexpected message: %s" % text)
        check("topsecret" not in sink.requests[0][1], "the password is in the notification")
    finally:
        sink.stop()
        stub.stop()


TESTS = [test_single_file, test_several_files_and_folder, test_obfuscation,
         test_retry_after_refusals, test_wrong_password, test_some_logins_refused,
         test_article_always_refused_on_one_connection, test_server_going_down,
         test_other_server_used, test_second_round, test_too_many_failures,
         test_repost_missing, test_repost_nothing_to_do, test_repost_still_failing,
         test_originals_kept_when_articles_failed, test_repost_cleans_and_deletes_originals,
         test_queue_resume_after_crash, test_queue_file_gone, test_queue_missing_articles,
         test_queue_used_by_another_ngpost, test_phase_durations, test_progress_line_has_eta, test_groups_are_cleaned,
         test_help_pages, test_latency_is_measured, test_feedback_forms,
         test_duplicate_is_skipped, test_nfo_file, test_nfo_media_info, test_profiles_and_par2_default, test_post_checker_unit, test_move_posted, test_move_posted_never_overwrites,
         test_move_posted_kept_when_articles_failed, test_move_posted_options, test_posted_mover_unit, test_notifications,
         test_notification_when_all_done]


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
