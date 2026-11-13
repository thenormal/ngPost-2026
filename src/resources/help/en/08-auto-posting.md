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

## Before it starts

When you press **Post the list** or **Start watching**, the program checks the settings once for the whole group (a server and a newsgroup, compression for folders, the rar / 7z and par2 programs, the folder of the NZB, the temporary folder). Problems are shown in plain words with a button that fixes them; warnings can be ignored with **Post anyway**. It is the same window as in [Quick Post](04-quick-post.md).

## Tips

- Use compression, random names, random passwords and par2: this is the usual setup for automated posting. The **Recommended** profile (menu **Profile**) sets it with one click.
- You can choose to delete the files once posted (command line only, `--rm_posted`). Do it only when you are sure: the files are removed for good.
- The program remembers the last button you used.
- For a NAS or a server without screen, the command line and the Docker image do the same job.

See [Command line](10-command-line.md).
