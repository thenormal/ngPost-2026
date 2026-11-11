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

## Version 1.3 (in preparation)

The points are added one by one, when they are approved.

| | |
|---|---|
| Planned | Fix: the "One release" note of Quick Post must not appear during or after the upload (the list then shows the volumes of the archive) |

## Ideas, not committed

No order, no version. Some of them will probably be done, some will be dropped.

**Control**
- Notifications (system tray, webhook for Telegram / Discord) when a post is done or fails
- Bandwidth limit and time slots
- VPN guard: pause the posts when the VPN connection drops (the real kill-switch stays in the firewall or in the VPN application)
- Check for updates and update from inside the program (with a checksum, and asking before anything is installed)
- Temporary files in RAM: easier on Windows, and chosen automatically when there is room

**Check and comfort**
- Check the post afterwards and show the result in the history
- Posting profiles, newsgroups per post and favorites, maybe the list of groups from the server
- Log: filters by category, search, one log per post
- A progress bar for each post in the lists, a graph of the speed
- Contextual help and search in the Help section

**Maintenance**
- Move to Qt 6 and split the biggest source file
- Digitally signed Windows installer (Windows would stop warning about an unknown program)
