# Roadmap

This is a working plan, not a promise: there are no release dates, and what is listed under *Ideas* may change, move or never happen. Releases are numbered with two numbers (1.2, 1.3...).

Your opinion counts: if an idea matters to you (or something is missing), say it in the [issues](https://github.com/thenormal/ngPost-Deluxe/issues).

## Version 1.2 - Reliability (released)

Goal: a post should not be lost because of a bad connection, a crash or a restart, and it should be easy to see what is going on.

| | |
|---|---|
| Done | Automated tests on every change (command line build and a fake Usenet server) |
| Done | Articles that fail are tried again by another connection or another server, with a second round at the end of the post |
| Done | **Post the missing Articles again** with one click (the nzb file is fixed, nothing is posted twice) |
| Done | **Persistent queue**: the posts that are not finished come back after a restart (resume, discard or decide later) |
| Done | Colored log and duration of each phase (compression, par2, upload) |
| Done | Progress bar with the phase, the remaining time and colors |
| Done | Newsgroups box with "chips" (suggestions and checks) and a menu for where the posts go when there are several groups |
| Done | Help section inside the program (offline, in English; other languages can be added later) |
| Done | Guide for Docker / Unraid behind a VPN container (gluetun) |
| Done | Latency measures in the diagnostic report |

## Version 1.3 - Easier to use

Goal: the program should be simple for someone who opens it for the first time, especially the Auto Posting page, and it should tell you what happens.

**Auto Posting, redesigned** (the biggest part of this version)

| | |
|---|---|
| Done | Three clear choices at the top, in plain words: post what I choose, scan a folder once, watch a folder and post what arrives |
| Done | A ready profile, "Recommended for automatic posting" (compression, random name and password, par2 10 %), with the single settings under "Advanced options" |
| Done | Checks in plain words before starting (for example: folders need compression), fixed with one click instead of an error message |
| Done | Preview of what will be posted (sizes), with a box to leave an item out |
| Planned | After posting, the user chooses: do nothing (default), **move to a "Posted" folder**, or delete the files |
| Planned | Items already in the history found by a one-time scan: one window lists them (name, date of the previous post) with a "Post again" box, off by default, instead of skipping them silently. The folder watching keeps skipping them with a line in the log |
| Planned | State of the folder watching always visible (folder, how many posts today), with a pause button |
| Planned | One list of the posts (state, progress, size, NZB, password) instead of one tab for each; if it turns out too big for this version it moves to 1.4 |

**Everything else**

| | |
|---|---|
| Done | **Profiles** of options (menu in Quick Post and Auto Posting: Recommended, Custom and the profiles saved by the user), one **NFO file** card shared by Quick Post and Auto Posting (moved from Settings), par2 starts at 10 % and the password is in the Archive card of Quick Post |
| Done | **Quick Post**: checks before posting (a window in plain words with a one-click fix: no server or group, folders without compression, rar / 7z or par2 not found, empty password, NZB folder not writable; warnings with "Post anyway": temporary folder, MediaInfo) and a summary line (items, size, size to upload, upload time from the real speed of the last upload). Auto Posting has the same checks, once for the whole group and also before watching a folder |
| Done | Fix: the "One release" note of Quick Post must not appear during or after the upload (the list then shows the volumes of the archive) |
| Planned | Fix: in a Quick Post tab whose post is finished, adding new files should show the "One release" note again (the finished state is not reset when the user adds files) |
| Planned | Uninstaller: ask whether to also delete the settings and the history kept in `%APPDATA%\ngPost Deluxe` (default: keep them) |
| Done | **Feedback** entry in the About window: report a bug or suggest an idea. It opens the new-issue page of GitHub already filled in (no access key in the program, nothing is sent without the user seeing it), with the diagnostic report to attach and issue forms in the repository |
| Done | Warning when a post is already in the history (same name and size), before it is posted again (Quick Post asks, auto posting and command line skip it; `--allow_repost` and a Settings option turn it off) |
| Done | NFO file generator for a post: written next to the nzb, optionally posted with the files, optional password and free text (Settings, card NFO file; `--gen_nfo`). The options will move into the shared options card of Quick Post and Auto Posting |
| Done | Media information in the NFO file with MediaInfo (duration, video, audio, subtitles of up to 20 files; on by default, `--no_nfo_media`, `--mediainfo_path`). MediaInfo is shipped in the Windows installer and the Docker image |
| Done | **Notifications** when the posts are done or fail: system notification (GUI), Discord, Telegram and a generic webhook; one message when all the posts are done (default) or after each post; the addresses and tokens are treated like passwords |

## Version 1.4 - Verification and control (first thoughts)

These are directions, not a list of promises. The points will be decided when 1.3 is done.

- **A post is finished when it is verified**: check that the Articles are on the server (button, optional automatic), show the result in the history, and post again by itself what is missing
- A post that stops **continues from the last Article sent** instead of starting again
- **Bandwidth limit** and time slots for the upload
- Posting profiles saved by the user, newsgroups per post and favorites, maybe the list of groups from the server

## Ideas, not committed

No order, no version. Some of them will probably be done, some will be dropped.

**Control**
- VPN guard: pause the posts when the VPN connection drops (the real kill-switch stays in the firewall or in the VPN application)
- Check for updates and update from inside the program (with a checksum, and asking before anything is installed)
- Temporary files in RAM: easier on Windows, and chosen automatically when there is room
- Web panel for Docker and NAS: queue, progress, history and pause from a browser (access with password, local network only by default)
- Automatic tuning of the number of connections and of the Article size, from the measures of the latency

**Check and comfort**
- Log: filters by category, search, one log per post
- A progress bar for each post in the lists, a graph of the speed
- Contextual help and search in the Help section

**Maintenance**
- Move to Qt 6 and split the biggest source file
- Digitally signed Windows installer (Windows would stop warning about an unknown program)
