## ngPost Deluxe 1.2

Third release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3). The theme of this version is **reliability**: a post should not be lost because of a bad connection, a crash or a restart, and it should be easy to see what is going on.

### What's new
- **Failed Articles are recovered**: an Article that fails is tried again on another server first, then in a second round. If some are still missing, the button **Repost missing Articles** posts only those and fixes the NZB file. The original files are kept when Articles failed.
- **Queue that survives a restart**: the posts that are not finished are saved and offered again at the next start (Resume / Discard / Decide later). On the command line: `--resume_queue`. A post that was being sent starts again from the beginning.
- **Wrong password**: the post now stops, with a clear message, when the server refuses the login on every connection (the login retries still work for a provider that limits the connections).
- **Progress bar and log**: the bar shows the phase (compression, par2, upload), the **remaining time** and colors (paused, completed, some Articles failed, error). The log is colored and gives the **duration of each phase** at the end of a post.
- **Newsgroups with "chips"**: a box where each group is a chip, with suggestions and checks. A post does not start with no group or with an invalid one. A menu chooses where the posts go when there are several groups.
- **Help page** inside the program (offline, in English for now, more languages later) and a **guide for Docker / Unraid behind a VPN** (gluetun): see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md).
- **Latency** in the diagnostic report (connection, SSL, login, round trip with the server), and the median latency at the end of each post.
- **Fixes**: crash and missing files in the NZB at the end of an upload; spaces around the newsgroups in the settings.
- Behind the scenes: automatic tests post to a fake NNTP server on GitHub at every change (articles, yEnc, NZB, retries, queue...), and a [ROADMAP](https://github.com/thenormal/ngPost-Deluxe/blob/master/ROADMAP.md) shows what is done, planned and under evaluation.

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.2-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.2-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.2-win64.zip` / `ngPostDeluxe-v1.2-win32.zip` | Portable version, no installation |
| `ghcr.io/thenormal/ngpost-deluxe` | **Docker** image (Linux amd64), see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md) |

**Updating from 1.1 or 1.0**: run the new installer, it replaces the old version. Your settings and history stay in `%APPDATA%\ngPost Deluxe`.

Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

If you already use ngPost: copy your `ngPost.conf` into `%APPDATA%\ngPost Deluxe` (the folder is created at the first start) to keep your servers and settings with the installed version. The portable version, started from the folder that contains `ngPost.conf`, keeps using it.

**Known limits**: a post resumed after a restart starts again from the beginning, the pause is not remembered after a restart, and the 32 bit version has not been tested on a real computer. See the Help page for more.

Please test it and report what does not work, or suggest improvements, on the [GitHub page](https://github.com/thenormal/ngPost-Deluxe/issues). If you like the program, there is also a small optional [Ko-fi](https://ko-fi.com/thenormal) page.

### Also in versions 1.1 and 1.0
Docker image with a template for Unraid, new interface (side bar, light and dark themes), History page, Servers page with **Test connection**, much faster compression on Windows, faster par2 with ParPar, Windows installer with 7-Zip included, diagnostic report, interface in 8 languages.
