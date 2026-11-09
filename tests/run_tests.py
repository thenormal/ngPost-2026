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
    env = dict(os.environ, HOME=work)          # no user config can interfere
    cmd = [NGPOST, "-h", "127.0.0.1", "-P", str(stub.port), "-u", stub.user,
           "-p", password or stub.password, "-n", str(conns), "-g", "alt.binaries.test",
           "-a", str(ARTICLE_SIZE), "-o", nzb, "-q"]
    cmd += list(extra)
    for i in inputs:
        cmd += ["-i", i]
    r = subprocess.run(cmd, env=env, cwd=work, capture_output=True,
                       text=True, timeout=timeout)
    if r.returncode < 0:
        raise AssertionError("ngPost crashed (signal %d)\n%s%s" % (-r.returncode, r.stdout, r.stderr))
    return r


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
    # known behaviour: with a wrong password ngPost pauses, waits 30 s and retries
    # for ever, so we let it run a few seconds and then stop it
    stub = NntpStub(password="good").start()
    try:
        src = os.path.join(work, "x.bin")
        make_file(src, 60_000, 8)
        nzb = os.path.join(work, "out.nzb")
        try:
            run_ngpost(stub, work, [src], nzb, timeout=8, password="WRONG")
        except subprocess.TimeoutExpired:
            pass
        check(stub.bad_auth > 0, "the server never saw a bad password")
        check(len(stub.accepted) == 0, "articles were accepted with a wrong password")
    finally:
        stub.stop()


TESTS = [test_single_file, test_several_files_and_folder, test_obfuscation,
         test_retry_after_refusals, test_wrong_password]


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
    print("\n%d/%d tests passed" % (len(TESTS) - failed, len(TESTS)))
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
