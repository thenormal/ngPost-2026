<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

**English** | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**A fast, free and modern Usenet poster for Windows, Linux and Docker.**<br/>
Compress, add par2, obfuscate and post your files from a clean interface (light and dark themes) or from the command line.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** is a revisited and redesigned version of the famous Usenet uploader **ngPost**.<br/>
It is a fork of [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licence GPL v3). All the credit for the original program goes to its author, Matthieu Bruel.<br/>

### Get started
| Windows installer | Windows portable | Docker / Unraid |
|---|---|---|
| Download `ngPostDeluxe-v…-win64-setup.exe` (or `win32`) from the [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest) page. 7-Zip is included. | Unzip `ngPostDeluxe-v…-win64.zip` (or `win32`) anywhere and start `ngPostDeluxe.exe`. The settings are in the `ngPost.conf` next to the program. | Image `ghcr.io/thenormal/ngpost-deluxe` with an Unraid template. See [docker/README.md](docker/README.md). |

> Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**.

### Quick start
1. Install and start ngPost Deluxe.
2. Open **Servers**, add your Usenet server and press **Test connection**.
3. Open **Quick Post** and drag your files or folders into the window.
4. Press **Post Files**. The NZB file is written in the folder chosen in the Settings, and every upload is listed in **History**.

Already using ngPost? Copy your `ngPost.conf` into `%APPDATA%\ngPost Deluxe` and your servers and settings are there.

### Screenshots
| Quick Post (light theme) | History |
|---|---|
| ![Quick Post (light theme)](docs/screenshots/quick-post.png) | ![History](docs/screenshots/history.png) |
| **Settings (dark theme)** | **Servers (dark theme)** |
| ![Settings (dark theme)](docs/screenshots/settings-dark.png) | ![Servers (dark theme)](docs/screenshots/servers-dark.png) |

### What is different in this fork

**Interface**
-   **New interface**: side bar (Quick Post, Auto Posting, Servers, Settings, History), pages with cards, advanced options hidden until needed, a new icon and an empty screen that explains what to do. **Light and dark themes**, or automatic like Windows (`theme` key). Set the environment variable `NGPOST_NO_THEME` to get the native look back
-   **Italian interface**: the GUI/CMD is now also translated in Italian (it was already available in Chinese, Dutch, English, French, German, Portuguese and Spanish)
-   **ngPost Deluxe v1.0**: new name and version (the Windows program is now `ngPostDeluxe.exe`; the configuration file does not change), redesigned About window, the original donation buttons are replaced by a **Ko-fi** button for this fork (in the About window and in the Settings)
-   **New Servers section**: list of the servers with a status dot and an on/off box, details form (password shown on demand) and a **Test connection** button that checks the login. Editing the servers while posting can no longer crash ngPost
-   **Auto Posting tab**: new buttons **Remove completed**, **Remove All** and **Close finished Tabs**
-   **History page**: every upload is recorded (`ngPost_history.csv` next to the configuration, or `POST_HISTORY`) with its status, number of files and duration. The page lists them with search, period filter, totals, passwords hidden until asked, copy buttons, export and the folder of the NZB. Be careful: the file contains the archive passwords in clear text
-   **Quick Post**: a note reminds that all the items of the list are posted as one release (one NZB, one archive)

**Speed**
-   **ParPar** (a much faster par2 generator) is shipped for Windows 64bit (v0.4.6, public domain) and used automatically when `parpar.exe` is in the ngPost folder. The 64bit par2cmdline has been removed from the package, the 32bit one is kept for Windows 32bit
-   **Faster compression on Windows**: the output of 7-Zip / rar is now read as it arrives (a 115 MB post waited 200 s before, 1 s now)

**Windows packages and Docker**
-   **ngPost Deluxe v1.1**: **Docker image** of the command line version (it watches a folder and posts everything that arrives in it, with a template for **Unraid**), the Post button is centered in Quick Post and Auto Posting, tab icons and finished tabs are readable in the dark theme
-   **Windows installer** (64 bit and 32 bit, with **7-Zip included**) and portable ZIP packages, built automatically on GitHub and published in the Releases (`ngPostDeluxe.exe`)
-   **Windows**: the console window closes by itself when ngPost is started with a double click; the package ships OpenSSL 1.1.1w (latest version supported by Qt 5)

**Security and fixes**
-   **Security fix**: possible heap overflow in the yEnc encoding buffer (the output could be bigger than the buffer with specific data)
-   **yEnc fix**: a space or tab at the end of a line is now escaped correctly (some servers strip it, which corrupted the data)
-   **Safer passwords**: the generated archive passwords and names use the cryptographic random generator of the OS (Qt >= 5.10) instead of `std::rand()`
-   **Archive password off the command line**: 7-Zip receives it on its standard input and rar through the `RAR` environment variable, so it is no longer visible in the list of processes

**Settings and tools**
-   **NZB_POST_CMD**: the placeholders can contain spaces (a path like `C:\My folder\file.nzb` stays ONE argument) and the command is started in background (ngPost doesn't wait for it anymore, so its exit code is no longer logged)
-   **RAR_EXTRA and PAR2_ARGS**: parsed respecting the double quotes. 7-Zip is detected from the program name and not from the whole path
-   **New README**: rewritten for this fork and available in 8 languages (English, French, German, Italian, Spanish, Dutch, Portuguese, Chinese)
-   **Autosave**: every change of the settings is saved automatically (the previous file is kept as `ngPost.conf.bak`)
-   **Diagnostics**: ngPost records the duration and speed of each phase (compression, par2, upload), the errors and the freezes, and the **Diagnostic report** button saves a report to analyse a problem. Passwords, usernames and file names are masked, nothing is sent anywhere

The Windows installers and portable packages and the Docker image are in the [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). On the other systems, build it from the sources (see below).<br/>

### Main features
-   **Command line and GUI Usenet poster** for binaries, developed in **C++11/Qt5**, designed to be as fast as possible
-   **compresses** (with your external rar/7zip) and **generates the par2** before posting
-   has a **posting queue** and **packs the next post while uploading the current one**
-   **automates posts**: scans folders, or monitors them, and posts each new file/folder individually (random archive name and password possible)
-   **obfuscates the Article headers**: random poster and UUID subject, the post can only be found with the nzb file
-   **generates the nzb file** and writes a csv **history of the posts** (archive name and password included)
-   can **delete files once posted**, **run a command or script** after each post (NZB_POST_CMD) and **switch off the computer**
-   translated in **Chinese, Dutch, English, French, German, Italian, Portuguese and Spanish**

### Configuration keywords changed in this fork
The configuration file works as in the original project (default: **~/.ngPost** on Linux and Mac, **ngPost.conf** in the installation folder on Windows). Only the keywords whose behaviour has changed are listed here:

- **NZB_POST_CMD**: command executed after each post. The placeholders can contain spaces and the command runs in background (its exit code is not logged)
- **RAR_EXTRA**: to customise the rar command (no need to specify 'a', '-idp' or '-r'). Not needed for 7-zip, except to change the compression level. It now respects double quotes.
- **PAR2_PATH**: to change the par2 generator and be able to use [Parpar](https://github.com/animetosho/ParPar) or [Multipar](http://hp.vector.co.jp/authors/VA021385/). (ParPar is used automatically on Windows 64bit if `parpar.exe` is in the ngPost folder; par2cmdline is still the default elsewhere)
- **PAR2_ARGS**: to customise the par2 command, especially if you choose a program different from the default one. It now respects double quotes.
- **theme**: interface theme, `AUTO` (like Windows), `LIGHT` or `DARK`
- **history**: `ON` (default) or `OFF`, history of the uploads shown in the History page. Without **POST_HISTORY** the file is `ngPost_history.csv` next to the configuration. New columns are added at the end of each line: status, number of files, duration and NZB file

### How to build
#### Linux
Dependencies (Ubuntu / Debian):
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Build the graphical version, or only the command line version (no Qt GUI libraries needed):
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
The executable **ngPost** is created in the folder where you built it: copy it somewhere in your PATH.

#### Windows
Install [Qt 5.15](https://www.qt.io/download) with MinGW, then open `src/ngPost.pro` in Qt Creator, or from a Qt command prompt: `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. The program is called `ngPostDeluxe.exe`. The installer is made with [Inno Setup](https://jrsoftware.org/isinfo.php) (`installer/ngPostDeluxe.iss`); all the steps of the official packages are in `.github/workflows/build-windows.yml`.

#### Docker
Command line version in a container (see [docker/README.md](docker/README.md)):
```
docker build -t ngpost-deluxe .
```

#### Other systems
As it is made in C++/Qt, you can build and run it on any OS (macOS, Android...): the easiest way is to [install Qt](https://www.qt.io/download) and load `src/ngPost.pro` in Qt Creator.

### Full documentation
The command line options, the complete list of configuration keywords and the installation guides are those of the original project: see the [README of ngPost](https://github.com/mbruel/ngPost#readme). Its installation guides refer to the original binaries, not to this fork.

### FAQ

**Windows says "Windows protected your PC". Is it safe?**<br/>
The installer is not digitally signed (a signature costs money), so SmartScreen warns about any unknown program. Click **More info**, then **Run anyway**. The program is open source and the packages are built automatically from this repository.

**Where are my settings and my history?**<br/>
With the installer: in `%APPDATA%\ngPost Deluxe` (`ngPost.conf` and `ngPost_history.csv`). With the portable version: `ngPost.conf` next to the program. If there is a `ngPost.conf` next to `ngPostDeluxe.exe`, it is used instead of the one in `%APPDATA%`. The history can be moved with `POST_HISTORY`.

**How do I keep the settings of the original ngPost?**<br/>
Close ngPost Deluxe, copy your old `ngPost.conf` into `%APPDATA%\ngPost Deluxe` and, if it contains a `PAR2_PATH` line pointing to an old folder, delete that line (the program then uses its own ParPar). Your old `ngPost_history.csv` is read too: copy it next to the configuration, or keep the path in `POST_HISTORY`.

**Something does not work (compression, par2, upload…). What can I do?**<br/>
Press **Diagnostic report** in the log and attach the file to an [issue](https://github.com/thenormal/ngPost-Deluxe/issues). Passwords, user names and file names are masked, nothing is sent anywhere.

**Do I have to install 7-Zip or rar?**<br/>
No: 7-Zip is included in the installer and the portable package. You can still choose another 7-Zip or WinRAR in the Settings.

**Is the history private?**<br/>
It contains the archive passwords in clear text: keep `ngPost_history.csv` (and your `ngPost.conf`, which contains your server login) private, and do not attach them to a public issue.

### Licence
[**ngPost**](https://github.com/mbruel/ngPost) is released under the **GPL v3** licence (see [LICENSE](LICENSE)). Copyright (C) 2020 Matthieu Bruel. ngPost Deluxe is a fork maintained by [thenormal](https://github.com/thenormal).

### Questions / Issues / Requests
- for questions about this fork, you can contact its maintainer on GitHub: [thenormal](https://github.com/thenormal)
- for the original program, refer to the [original project](https://github.com/mbruel/ngPost) and its author Matthieu Bruel

### Supported Languages
For now, ngPost is translated in Chinese, Dutch, English, French, German, Italian, Portuguese and Spanish (the Italian translation has been added in this fork).<br/>
If you'd like to translate ngPost in your language, it's easy to do (there is a nice GUI for that: QtLinguist), the translation files are in the **src/lang** folder.

### Thanks
- Matthieu Bruel (mbruel) for having created ngPost and published it under GPL v3
- Uukrull for his intensive testing and feedbacks and for building all the MacOS packages.
- awsms for his testing on proper server with a 10Gb/s connection that helped to improve ngPost's upload speed and the multi-threading support
- animetosho for having developped ParPar, the fasted par2 generator ever!
- demanuel for the dev of NewsUP that was my first poster
- noobcoder1983, tensai then yuppie for the German translation
- tiriclote for the Spanish translation
- hunesco for the Portuguese translation
- Peng for the Chinese translation
- all ngPost users ;)

### Support this fork
ngPost Deluxe is free. If it is useful to you, you can support its development on Ko-fi:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Support the original author
ngPost has been created by Matthieu Bruel. If you use the app and would like to contribute to the effort, please do it on the [original project](https://github.com/mbruel/ngPost).
