"""Tiny NNTP server used by the ngPost tests.

It accepts AUTHINFO + POST, decodes every yEnc article it receives, checks the
sizes and CRC32 and keeps the decoded data so the tests can rebuild the posted
files. A "policy" function lets a test refuse some articles (to exercise the
retry logic).
"""
import socketserver
import threading
import zlib


class Article:
    def __init__(self, msg_id, subject, groups):
        self.msg_id, self.subject, self.groups = msg_id, subject, groups
        self.name = None      # file name from =ybegin
        self.part = None
        self.total = None
        self.file_size = None
        self.begin = self.end = None
        self.data = b""
        self.errors = []      # anything wrong found while decoding


def yenc_decode(lines):
    out = bytearray()
    for line in lines:
        if line.startswith(b".."):          # dot-stuffing
            line = line[1:]
        i, n = 0, len(line)
        while i < n:
            c = line[i]
            if c == 0x3D:                    # '=' escape
                i += 1
                if i >= n:
                    break
                c = (line[i] - 64) & 0xFF
            out.append((c - 42) & 0xFF)
            i += 1
    return bytes(out)


def _kv(text):
    res = {}
    for tok in text.split():
        if "=" in tok:
            k, v = tok.split("=", 1)
            res[k] = v
    return res


def parse_article(raw_lines):
    """raw_lines: lines of the article without the final '.' line."""
    head, body, in_body = {}, [], False
    for ln in raw_lines:
        if not in_body:
            if ln in (b"", b"\r\n", b"\n"):
                in_body = True
                continue
            k, _, v = ln.decode("latin-1").partition(":")
            head[k.strip().lower()] = v.strip()
        else:
            body.append(ln.rstrip(b"\r\n"))
    art = Article(head.get("message-id", "").strip("<>"),
                  head.get("subject", ""), head.get("newsgroups", ""))
    data_lines, pcrc, size_end = [], None, None
    for ln in body:
        if ln.startswith(b"=ybegin "):
            kv = _kv(ln[7:].decode("latin-1").split(" name=")[0])
            art.part = int(kv.get("part", 1))
            art.total = int(kv.get("total", 1))
            art.file_size = int(kv.get("size", 0))
            art.name = ln.decode("latin-1").split(" name=", 1)[1] if " name=" in ln.decode("latin-1") else None
        elif ln.startswith(b"=ypart "):
            kv = _kv(ln[7:].decode("latin-1"))
            art.begin, art.end = int(kv["begin"]), int(kv["end"])
        elif ln.startswith(b"=yend "):
            kv = _kv(ln[6:].decode("latin-1"))
            pcrc = kv.get("pcrc32") or kv.get("crc32")
            size_end = int(kv.get("size", -1))
        else:
            data_lines.append(ln)
    art.data = yenc_decode(data_lines)
    if art.name is None:
        art.errors.append("no =ybegin")
    if pcrc is None:
        art.errors.append("no =yend crc32")
    else:
        if zlib.crc32(art.data) & 0xFFFFFFFF != int(pcrc, 16):
            art.errors.append("bad crc32")
    if size_end is not None and size_end != len(art.data):
        art.errors.append("bad size in =yend")
    if art.begin is not None and art.end - art.begin + 1 != len(art.data):
        art.errors.append("size does not match =ypart")
    return art


class NntpStub:
    """policy(article, attempt) -> None to accept, or a response line to refuse.

    attempt counts the tries received for the same (file name, part), because ngPost
    may give a new message id to an article it sends again."""

    def __init__(self, user="user", password="pass", policy=None, refuse_logins=0):
        self.user, self.password = user, password
        self.refuse_logins = refuse_logins   # the first N logins are refused (like a connection limit)
        self.policy = policy or (lambda article, attempt: None)
        self.accepted = {}          # msg_id -> Article
        self.attempts = {}          # (name, part) -> number of tries received
        self.refused = 0
        self.bad_auth = 0
        self._lock = threading.Lock()
        self.events = []            # (seconds, connection number, text) to understand a failed test
        self._t0 = __import__('time').time()
        self._nconn = 0
        stub = self

        class Handler(socketserver.StreamRequestHandler):
            def handle(self):
                w = self.wfile
                with stub._lock:
                    stub._nconn += 1
                    me = stub._nconn
                def ev(text):
                    stub.events.append((round(__import__('time').time() - stub._t0, 3), me, text))
                ev('connected')
                w.write(b"200 stub news server ready\r\n")
                user_ok = False
                while True:
                    line = self.rfile.readline()
                    if not line:
                        ev('closed by ngPost')
                        return
                    parts = line.decode("latin-1").strip().split(" ", 2)
                    cmd = parts[0].upper()
                    if cmd == "AUTHINFO" and parts[1].upper() == "USER":
                        user_ok = parts[2] == stub.user
                        w.write(b"381 PASS required\r\n")
                    elif cmd == "AUTHINFO" and parts[1].upper() == "PASS":
                        with stub._lock:
                            refuse = stub.refuse_logins > 0
                            if refuse:
                                stub.refuse_logins -= 1
                        if user_ok and parts[2] == stub.password and not refuse:
                            ev('login ok')
                            w.write(b"281 Ok\r\n")
                        else:
                            with stub._lock:
                                stub.bad_auth += 1
                            ev('login refused')
                            w.write(b"481 Authentication failed\r\n")
                    elif cmd == "POST":
                        w.write(b"340 send article\r\n")
                        raw = []
                        while True:
                            d = self.rfile.readline()
                            if not d or d == b".\r\n":
                                break
                            raw.append(d)
                        art = parse_article(raw)
                        with stub._lock:
                            key = (art.name, art.part)
                            n = stub.attempts.get(key, 0) + 1
                            stub.attempts[key] = n
                            verdict = stub.policy(art, n)
                            if verdict is None:
                                stub.accepted[art.msg_id] = art
                            else:
                                stub.refused += 1
                        ev('article %s part %s %s' % (art.name, art.part, 'accepted' if verdict is None else 'refused'))
                        w.write(b"240 article posted\r\n" if verdict is None
                                else verdict + b"\r\n")
                    elif cmd == "QUIT":
                        ev('QUIT')
                        w.write(b"205 bye\r\n")
                        return
                    else:
                        w.write(b"500 what\r\n")

        class Server(socketserver.ThreadingTCPServer):
            allow_reuse_address = True
            daemon_threads = True

        self._srv = Server(("127.0.0.1", 0), Handler)
        self.port = self._srv.server_address[1]
        self._thread = threading.Thread(target=self._srv.serve_forever, daemon=True)

    def start(self):
        self._thread.start()
        return self

    def stop(self):
        self._srv.shutdown()
        self._srv.server_close()

    def rebuild_files(self):
        """name -> bytes, rebuilt from the decoded parts (only for named files)."""
        parts = {}
        for a in self.accepted.values():
            if a.name is not None and a.begin is not None:
                parts.setdefault(a.name, {})[a.begin] = a.data
        files = {}
        for name, p in parts.items():
            files[name] = b"".join(p[k] for k in sorted(p))
        return files
