# Quick Post

Quick Post sends the files and folders you choose. Each tab is one post, so you can prepare several posts and run them in a queue.

## Steps

1. Add files and folders with **Select Files** or by dragging them in.
2. Choose the options (below).
3. Press **Post Files**.

## Options

- **Compress**: puts the files in a RAR or 7z archive before posting. You can split it in volumes.
- **Password**: the archive can have a password, fixed or generated at random. The password is saved with the post (for example in the NZB meta data and in the History).
- **Random name**: gives the archive a random name.
- **par2**: creates recovery files. Set the redundancy in percent: more redundancy lets the downloader repair more missing articles, but uploads more data.
- **Obfuscation**: hides the subjects of the articles. Without the NZB file nobody can find your post, not even you.

## Checks and summary

Under the cards a line tells you what is going to be posted: the number of items, their size and, with compression or par2, about how much is uploaded. The upload time appears after the first post of the session, from the real speed of that upload (before it, ngPost does not guess).

When you press **Post Files**, ngPost checks the post first. If something is wrong a window explains it in plain words, with a button that fixes it:

- **Errors** (the post can't start): no enabled server, no valid newsgroup, folders without compression, rar / 7z or par2 program not found, a password turned on but empty, an NZB folder that is not writable.
- **Warnings** (you can choose **Post anyway**): the temporary folder is missing or may be too small, MediaInfo is not found for the media information of the NFO.

After a fix the checks run again, so a new problem shows up at once instead of in the middle of the post. Auto Posting does not have this window yet.

## Profiles

The **Profile** menu above the cards sets all the options at once.

- **Recommended**: compression, random name and password, par2 with 10 % redundancy. It is the choice of a new installation.
- **Custom**: shown when you change an option by hand.
- **Your own profiles**: set the options, press **Save as profile...** and give it a name. **Delete** removes the chosen profile. The profiles are saved in the configuration file (key `profile`) and are the same in Quick Post and Auto Posting.

The **NFO file** card (see [NZB files](09-nzb-files.md)) is shared too: Quick Post and Auto Posting show the same settings.

## Tabs

The **+** tab opens a new post. A tab shows its state with an icon: pending, uploading, done or error.

See also [Progress and log](06-progress-and-log.md).
