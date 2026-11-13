# Auto Posting

Auto Posting posts many files and folders without preparing each one by hand. Choose one of the three buttons at the top of the page; only the settings you need are shown.

## Post what I choose

Add the files and folders you want (drag them into the list, or use **Select Files** / **Select a Folder**), then press **Post the list**: each file or folder becomes a separate post.

## Scan a folder once

Choose a folder and press **Scan**: what is inside is listed. Folders need **compression**. Use **latest files first** to sort by date.

## Watch a folder and post what arrives

Choose the folder and press **Start watching**: each new file or folder is posted when it arrives. Press **Stop watching** to stop. Stopping with posts still waiting asks for confirmation.

## The list

Each item has a box and its size. Uncheck an item to leave it out; only the checked items are posted. To remove items from the list, select them and press DEL. The line under the list shows how many items are selected, their size, the size to upload and, after a first post, the upload time.

## Items already posted

If some checked items are already in the history (same name and size), one window lists them with the date of the previous post. Check **Post again** for the ones you want to post once more; the others are not posted and stay in the list, unchecked and grey. If an NZB file with the same name already exists, the new one is named `name_1.nzb` and so on. When watching a folder, items already in the history are skipped with a line in the log. You can turn the warning off in **Settings** (History).

## Before it starts

When you press **Post the list** or **Start watching**, the program checks the settings once for the whole group (a server and a newsgroup, compression for folders, the rar / 7z and par2 programs, the folder of the NZB, the temporary folder). Problems are shown in plain words with a button that fixes them; warnings can be ignored with **Post anyway**. It is the same window as in [Quick Post](04-quick-post.md).

## Tips

- Use compression, random names, random passwords and par2: this is the usual setup for automated posting. The **Recommended** profile (menu **Profile**) sets it with one click.
- **After posting** (the row above the Post button): do nothing (default), move the files to a **"Posted" folder**, or delete them. The default name `Posted` creates that folder next to each file or folder; you can also choose another folder. Nothing is moved or deleted unless the post finished without error. A name already used in the folder is never overwritten: the file becomes `name (2)`. The scan and the folder watching ignore the "Posted" folder. Deleting is for good: use it only when you are sure.
- On the command line the same choice is `--move_posted` or `--rm_posted` (see [Command line](10-command-line.md)).
- The program remembers the last button you used.
- For a NAS or a server without screen, the command line and the Docker image do the same job.

See [Command line](10-command-line.md).
