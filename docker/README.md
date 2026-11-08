# ngPost Deluxe in Docker (Unraid, NAS, servers)

The image contains the **command line** version of ngPost Deluxe (no graphical interface), 7-Zip and ParPar.
It **watches a folder**: every file or folder you put in `/watch` is compressed (random name and password),
protected with par2 and posted. The NZB file is written in `/nzb`, the history (with the archive passwords) in `/config`.

Image: `ghcr.io/thenormal/ngpost-deluxe` (tags `latest` and the version, for example `1.1`).

## Quick start

```
docker run -d --name ngpost-deluxe \
  -e NG_HOST=news.example.com -e NG_PORT=563 -e NG_SSL=true \
  -e NG_USER=myUser -e NG_PASS=myPassword \
  -e NG_GROUPS=alt.binaries.test \
  -v /path/config:/config \
  -v /path/watch:/watch \
  -v /path/nzb:/nzb \
  -v /path/work:/work \
  ghcr.io/thenormal/ngpost-deluxe:latest
```

The first time, `/config/ngPost.conf` is created from the `NG_*` variables. After that the file is used as it is
(edit it, or put your own `ngPost.conf` in the config folder before the first start and the variables are not needed).
Check the log with `docker logs -f ngpost-deluxe`.

## Folders

| Folder | Content |
|---|---|
| `/watch` | folder to monitor: put here what you want to post (must be writable if you use `NG_DELETE_AFTER_POST`) |
| `/nzb` | the NZB files |
| `/config` | `ngPost.conf` and `ngPost_history.csv` (**it contains the archive passwords in clear text: keep it private**) |
| `/work` | temporary files (archives and par2): it needs as much space as your biggest post |

## Variables (used only to create `ngPost.conf` the first time)

| Variable | Default | |
|---|---|---|
| `NG_HOST`, `NG_USER`, `NG_PASS`, `NG_GROUPS` | | **required** if there is no `ngPost.conf` |
| `NG_PORT` | `563` | |
| `NG_SSL` | `true` | `false` for port 119 |
| `NG_CONNECTIONS` | `20` | check what your provider allows |
| `NG_PAR2_PCT` | `10` | par2 redundancy in % |
| `NG_ARCHIVE_SIZE_MB` | `0` | split the archive in volumes of this size (0 = no split) |
| `NG_SCAN_DELAY_SEC` | `10` | seconds a new file must stay unchanged before it is posted |
| `NG_GROUP_POLICY` | `ALL` | `ALL`, `EACH_POST` or `EACH_FILE` when there are several groups |

These ones are read every time the container starts:

| Variable | Default | |
|---|---|---|
| `NG_DELETE_AFTER_POST` | `false` | `true` deletes the files from `/watch` once posted |
| `PUID`, `PGID`, `UMASK` | `99`, `100`, `022` | user / group / permissions (nobody:users on Unraid) |

## Notes

- Your password is visible in the container settings (`docker inspect`) when you use `NG_PASS`. If you prefer, put your own
  `ngPost.conf` in the config folder.
- New files are detected with the file system notifications of Linux (inotify). If you copy files to the watch folder
  from another computer through the network, wait for the copy to end: a file is posted when its size stops changing.
- Extra options of `ngPost` can be added after the image name (for example `--debug`). The list: `docker run --rm --entrypoint ngPost ghcr.io/thenormal/ngpost-deluxe --help`.
- The image is built for `amd64` (x86-64).

## Unraid

The template is in [`unraid/ngpost-deluxe.xml`](../unraid/ngpost-deluxe.xml). Until the application is in the *Community Apps* catalog:

1. Docker tab > **Add Container** > in **Template** choose *Add template URL* (or copy the XML file in `/boot/config/plugins/dockerMan/templates-user/`)
   with `https://raw.githubusercontent.com/thenormal/ngPost-Deluxe/master/unraid/ngpost-deluxe.xml`
2. Choose the folders (watch, nzb, config, work), fill in your Usenet server, user, password and newsgroup(s), then **Apply**.

## Build it yourself

```
docker build -t ngpost-deluxe .
```
