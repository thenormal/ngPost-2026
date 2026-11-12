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
