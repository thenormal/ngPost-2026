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

## Profiles

The **Profile** menu above the cards sets all the options at once.

- **Recommended**: compression, random name and password, par2 with 10 % redundancy. It is the choice of a new installation.
- **Custom**: shown when you change an option by hand.
- **Your own profiles**: set the options, press **Save as profile...** and give it a name. **Delete** removes the chosen profile. The profiles are saved in the configuration file (key `profile`) and are the same in Quick Post and Auto Posting.

The **NFO file** card (see [NZB files](09-nzb-files.md)) is shared too: Quick Post and Auto Posting show the same settings.

## Tabs

The **+** tab opens a new post. A tab shows its state with an icon: pending, uploading, done or error.

See also [Progress and log](06-progress-and-log.md).
