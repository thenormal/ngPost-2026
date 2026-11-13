# NZB files

An NZB file lists the articles of a post. Anyone who has it can download the post from Usenet.

## Where it is saved

The folder is set in **Settings**, section **NZB files**. The file takes the name of the post (the first file or folder).

## Using it

- Share it together with the password of the archive, if you set one.
- A download program (a newsreader) opens it and downloads the files.
- With obfuscation the NZB is the **only** way to find your post: keep a copy.

## Check an NZB

On the command line, `--check` verifies that the articles of an NZB are still available on the server.

## History

The **History** page lists your past uploads (date, name, size, speed, archive, password, groups). You can turn the history off in Settings. The password is saved in clear text.

### Posts already in the history

Before a post starts, ngPost looks in the history for a successful post with the same NZB name and the same total size of the files. In **Quick Post** a window asks whether to post it again (**Post anyway** or **Cancel**). In **Auto Posting** and on the command line the post is skipped and a line in the log says so. To post it again anyway, use the option `--allow_repost` or turn the warning off in **Settings** (History of the uploads). The check needs the history to be on, and it only looks at the name and the size: a renamed folder is not recognised.

## NFO file

An NFO is a small text file that describes the post: its name, the date, the files with their sizes, how it was prepared (archive and par2) and a text of yours. Turn it on in the card **NFO file** of Quick Post or Auto Posting (it is the same card: the settings are shared).

- **Write an NFO file next to the nzb**: `name.nfo` is saved in the same folder as `name.nzb`.
- **Also post the NFO file with the files**: it is uploaded with the post and appears in the NZB.
- **Include media information (MediaInfo)**: on by default. For videos and audio files the NFO lists the duration, the video (format, size, frame rate, HDR), the audio tracks (language, format, channels, bit rate) and the subtitles. At most 20 files are described (the others are counted) and the reading stops after 20 seconds. It uses the free program MediaInfo: it comes with the Windows installer and the Docker image; on Linux and macOS install the `mediainfo` package, or choose the program in Settings. Without MediaInfo the NFO simply has no media section.
- **Write the password of the archive**: off by default. In a posted NFO the password can be read by everyone who downloads the post.
- **Text added at the end**: notes, links, anything you want to say. The same text is used for all the posts.

When the names of the files are hidden (obfuscation of the articles or of the file names), the NFO that is posted does not list any file name or media information, and the file itself gets a random name. On the command line use `--gen_nfo`, `--nfo_in_post`, `--nfo_password` and `--nfo_text file.txt`, plus `--no_nfo_media` and `--mediainfo_path path`.
