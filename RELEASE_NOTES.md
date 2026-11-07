## ngPost Deluxe 1.0

First stable release of **ngPost Deluxe**, a revisited and redesigned version of the Usenet poster [ngPost](https://github.com/mbruel/ngPost) by Matthieu Bruel (GPL v3).

### What's new
- **New interface**: side bar (Quick Post, Auto Posting, Servers, Settings, History), pages made of cards, advanced options hidden until needed, a new icon and an empty screen that explains what to do
- **Light and dark themes**, or automatic like Windows
- **History page**: all your uploads in one sortable and searchable table (date, name, size, speed, status, files, duration, archive, password, groups), with totals, hidden passwords, copy and export. It reads the old `ngPost_history.csv` files too
- **Servers page** with a **Test connection** button
- **Support the project** button (Ko-fi) in the About window and in the Settings
- **Faster compression on Windows** (the output of 7-Zip / rar is now read as it arrives), faster par2 with **ParPar** (64 bit), security and yEnc fixes inherited from the previous beta
- **Diagnostic report** button in the log to help to find problems
- Interface translated in 8 languages: English, French, German, Italian, Spanish, Dutch, Portuguese, Chinese

### Download
| File | For |
|---|---|
| `ngPostDeluxe-v1.0-win64-setup.exe` | **Installer** for Windows 64 bit (Windows 10 / 11), 7-Zip included |
| `ngPostDeluxe-v1.0-win32-setup.exe` | **Installer** for Windows 32 bit (par2cmdline instead of ParPar), 7-Zip included |
| `ngPostDeluxe-v1.0-win64.zip` / `ngPostDeluxe-v1.0-win32.zip` | Portable version, no installation |

Run the installer and follow the steps. Windows may show "Windows protected your PC" because the installer is not digitally signed: click **More info**, then **Run anyway**. Your settings and the history are kept in `%APPDATA%\ngPost Deluxe` (they survive updates and uninstall).

[7-Zip](https://www.7-zip.org/) (GNU LGPL, licence in the `7-Zip` folder) is included and used by default to compress the posts: there is nothing else to install. You can still choose another 7-Zip or WinRAR in the Settings.

If you already use ngPost: copy your `ngPost.conf` into `%APPDATA%\ngPost Deluxe` (the folder is created at the first start) to keep your servers and settings with the installed version. The portable version, started from the folder that contains `ngPost.conf`, keeps using it.

### Version number
ngPost Deluxe starts its own numbering at 1.0 (the fork was v5.0 beta). Copies of the 5.0 beta will not be told about this release: download it from this page.
