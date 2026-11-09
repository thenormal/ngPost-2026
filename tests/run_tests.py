#!/usr/bin/env python3
"""End-to-end tests of the command line ngPost against a fake NNTP server.

usage: python3 tests/run_tests.py /path/to/ngPost
"""
import hashlib
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


def run_ngpost_conf(stubs, work, inputs, nzb, conns=2, threads=None, extra=(), timeout=180, env_extra=None):
    """post with a configuration file declaring one server per stub"""
    conf = os.path.join(work, "test.conf")
    with open(conf, "w") as f:
        f.write("GROUPS = alt.binaries.test\nARTICLE_SIZE = %d\nRESUME_WAIT = 2\nSOCK_TIMEOUT = 10\n" % ARTICLE_SIZE)
        if threads:
            f.write("THREAD = %d\n" % threads)
        for st in stubs:
            f.write("\n[server]\nhost = 127.0.0.1\nport = %d\nssl = false\nuser = %s\npass = %s\n"
                    "connection = %d\nenabled = true\n" % (st.port, st.user, st.password, conns))
    env = dict(os.environ, HOME=work, NGPOST_SECOND_ROUND_DELAY_MS="2000")
    env.update(env_extra or {})
    cmd = [NGPOST, "--conf", conf, "-o", nzb, "-q"] + list(extra)
    for i in inputs:
        cmd += ["-i", i]
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


TESTS = [test_single_file, test_several_files_and_folder, test_obfuscation,
         test_retry_after_refusals, test_wrong_password, test_some_logins_refused,
         test_article_always_refused_on_one_connection, test_server_going_down,
         test_other_server_used, test_second_round, test_too_many_failures]


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
