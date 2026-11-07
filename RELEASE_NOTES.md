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
| `ngPostDeluxe-v1.0-win64.zip` | Windows 64 bit (Windows 10 / 11) |
| `ngPostDeluxe-v1.0-win32.zip` | Windows 32 bit (par2cmdline instead of ParPar) |

Unzip the folder anywhere and start `ngPostDeluxe.exe`. To compress the posts you also need [7-Zip](https://www.7-zip.org/) or WinRAR: set its path in the Settings (or `RAR_PATH` in the configuration file).

Your existing `ngPost.conf` / `.ngPost` configuration file keeps working.

### Version number
ngPost Deluxe starts its own numbering at 1.0 (the fork was v5.0 beta). Copies of the 5.0 beta will not be told about this release: download it from this page.
