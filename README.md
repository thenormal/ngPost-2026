<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

**English** | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost Deluxe** is a revisited and redesigned version of the famous Usenet uploader **ngPost**.<br/>
It is a fork of [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licence GPL v3). All the credit for the original program goes to its author, Matthieu Bruel.<br/>

### What is different in this fork
-   **Security fix**: possible heap overflow in the yEnc encoding buffer (the output could be bigger than the buffer with specific data)
-   **yEnc fix**: a space or tab at the end of a line is now escaped correctly (some servers strip it, which corrupted the data)
-   **Safer passwords**: the generated archive passwords and names use the cryptographic random generator of the OS (Qt >= 5.10) instead of `std::rand()`
-   **NZB_POST_CMD**: the placeholders can contain spaces (a path like `C:\My folder\file.nzb` stays ONE argument) and the command is started in background (ngPost doesn't wait for it anymore, so its exit code is no longer logged)
-   **RAR_EXTRA and PAR2_ARGS**: parsed respecting the double quotes. 7-Zip is detected from the program name and not from the whole path
-   **ParPar** (a much faster par2 generator) is shipped for Windows 64bit (v0.4.6, public domain) and used automatically when `parpar.exe` is in the ngPost folder. The 64bit par2cmdline has been removed from the package, the 32bit one is kept for Windows 32bit
-   **New interface**: side bar (Quick Post, Auto Posting, Servers, Settings, History), pages with cards, advanced options hidden until needed, a new icon and an empty screen that explains what to do. **Light and dark themes**, or automatic like Windows (`theme` key). Set the environment variable `NGPOST_NO_THEME` to get the native look back
-   **Italian interface**: the GUI/CMD is now also translated in Italian (it was already available in Chinese, Dutch, English, French, German, Portuguese and Spanish)
-   **New README**: rewritten for this fork and available in 8 languages (English, French, German, Italian, Spanish, Dutch, Portuguese, Chinese)
-   **ngPost Deluxe v5.0 beta**: new name and version (the Windows program is now `ngPostDeluxe.exe`; the configuration file does not change), redesigned About window, the original donation buttons are replaced by a **Ko-fi** button for this fork (in the About window and in the Settings)
-   **New Servers section**: list of the servers with a status dot and an on/off box, details form (password shown on demand) and a **Test connection** button that checks the login. Editing the servers while posting can no longer crash ngPost
-   **Autosave**: every change of the settings is saved automatically (the previous file is kept as `ngPost.conf.bak`)
-   **Auto Posting tab**: new buttons **Remove completed**, **Remove All** and **Close finished Tabs**
-   **Archive password off the command line**: 7-Zip receives it on its standard input and rar through the `RAR` environment variable, so it is no longer visible in the list of processes
-   **Diagnostics**: ngPost records the duration and speed of each phase (compression, par2, upload), the errors and the freezes, and the **Diagnostic report** button saves a report to analyse a problem. Passwords, usernames and file names are masked, nothing is sent anywhere
-   **Windows**: the console window closes by itself when ngPost is started with a double click; the package ships OpenSSL 1.1.1w (latest version supported by Qt 5)
-   **History page**: every upload is recorded (`ngPost_history.csv` next to the configuration, or `POST_HISTORY`) with its status, number of files and duration. The page lists them with search, period filter, totals, passwords hidden until asked, copy buttons, export and the folder of the NZB. Be careful: the file contains the archive passwords in clear text
-   **Quick Post**: a note reminds that all the items of the list are posted as one release (one NZB, one archive)

There are no official releases of this fork yet. A ready-to-run **Windows 64bit** package is built automatically by GitHub Actions at each change (tab *Actions* > run *Build Windows (x64)* > *Artifacts*, a GitHub account is needed). On the other systems, build it from the sources (see below).<br/>

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
- **PAR2_CMD**: to change the par2 generator and be able to use [Parpar](https://github.com/animetosho/ParPar) or [Multipar](http://hp.vector.co.jp/authors/VA021385/). (ParPar is used automatically on Windows 64bit if `parpar.exe` is in the ngPost folder; par2cmdline is still the default elsewhere)
- **PAR2_ARGS**: to customise the par2 command, especially if you choose a program different from the default one. It now respects double quotes.
- **theme**: interface theme, `AUTO` (like Windows), `LIGHT` or `DARK`
- **history**: `ON` (default) or `OFF`, history of the uploads shown in the History page. Without **POST_HISTORY** the file is `ngPost_history.csv` next to the configuration. New columns are added at the end of each line: status, number of files, duration and NZB file

### How to build
#### Dependencies:
- build-essential (C++ compiler, libstdc++, make,...)
- qt5-default (Qt5 libraries and headers)
- qt5-qmake (to generate the moc files and create the Makefile)
- libssl (v1.0.2 or v1.1) but it should be already installed on your system

#### Build:
- go to the src folder
- qmake
- make

Easy! it should have generate the executable **ngPost**</br>
you can copy it somewhere in your PATH so it will be accessible from anywhere


As it is made in C++/Qt, you can build it and run it on any OS (Linux / Windows / MacOS / Android) <br/>
In order to build on other OS, the easiest way would be to [install Qt](https://www.qt.io/download) and load the project in QtCreator<br/>

### Full documentation
The command line options, the complete list of configuration keywords and the installation guides are those of the original project: see the [README of ngPost](https://github.com/mbruel/ngPost#readme). Its installation guides refer to the original binaries, not to this fork.

### Licence
**ngPost** is released under the **GPL v3** licence
<pre>
//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================
</pre>

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
