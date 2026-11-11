# Auto Posting

Auto Posting posts many files and folders without preparing each one by hand.

## Scan a folder

Choose a folder with **Choose a folder to scan**, then **Generate Posts**: each file or folder inside it becomes a separate post. Folders need **compression**.

## Monitor a folder

**Monitor Folder** keeps watching the folder: each new file or folder is posted when it arrives. Press **Stop Monitoring** to stop. Stopping with posts still waiting asks for confirmation.

## Tips

- Use compression, random names, random passwords and par2: this is the usual setup for automated posting.
- You can choose to delete the files once posted (command line only, `--rm_posted`). Do it only when you are sure: the files are removed for good.
- For a NAS or a server without screen, the command line and the Docker image do the same job.

See [Command line](10-command-line.md).
