# Command line

The program can also run without a window, for a server, a NAS or a script. The command line version is the same program built without the window (or the Docker image).

## Examples

Post two items with compression, random name and password, and par2:

    ngPost -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2

Post every item of a folder, one post each:

    ngPost --auto /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rar_size 42

Watch a folder and delete the files once posted:

    ngPost --monitor /data/folder1 --compress --gen_par2 --gen_name --gen_pass --rm_posted

## Useful options

- `-c` configuration file (otherwise `$HOME/.ngPost`)
- `-g` newsgroups, separated by commas
- `-o` NZB output file
- `-S` server in one string
- `--disp_progress` BAR or FILES
- `--resume_queue` post again the posts not finished
- `--allow_repost` post again the files that are already in the history (by default they are skipped)
- `--help` the full list

See [Auto Posting](08-auto-posting.md).
