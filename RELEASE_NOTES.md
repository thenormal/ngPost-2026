## ngPost Deluxe 1.1

Second release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3).

### What's new
- **Docker image** of the command line version, for a NAS or a server: it watches a folder and posts everything that arrives in it (7-Zip and ParPar included), with a **template for Unraid**. See [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md)
- The **Post Files / Generate Posts button is now centered** in Quick Post and Auto Posting
- **Dark theme**: the icons of the Quick Post, Auto Posting and New tabs are visible again, and finished tabs keep a readable text colour
- Behind the scenes: the release is attached to GitHub automatically (installers and ZIP files), and the build no longer depends on a download of 7-Zip

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.1-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.1-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.1-win64.zip` / `ngPostDeluxe-v1.1-win32.zip` | Portable version, no installation |
| `ghcr.io/thenormal/ngpost-deluxe` | **Docker** image (Linux amd64), see [docker/README.md](https://github.com/thenormal/ngPost-Deluxe/blob/master/docker/README.md) |

**Updating from 1.0**: run the new installer, it replaces the old version. Your settings and history stay in `%APPDATA%\ngPost Deluxe`.

Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

If you already use ngPost: copy your `ngPost.conf` into `%APPDATA%\ngPost Deluxe` (the folder is created at the first start) to keep your servers and settings with the installed version. The portable version, started from the folder that contains `ngPost.conf`, keeps using it.

Please test it and report what does not work, or suggest improvements, on the [GitHub page](https://github.com/thenormal/ngPost-Deluxe/issues). If you like the program, there is also a small optional [Ko-fi](https://ko-fi.com/thenormal) page.

### Also in version 1.0
New interface (side bar, light and dark themes), History page, Servers page with **Test connection**, much faster compression on Windows, faster par2 with ParPar, Windows installer with 7-Zip included, diagnostic report, interface in 8 languages.
