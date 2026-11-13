# ngPost Deluxe in Docker (Unraid, NAS, servers)

The image contains the **command line** version of ngPost Deluxe (no graphical interface), 7-Zip and ParPar.
It **watches a folder**: every file or folder you put in `/watch` is compressed (random name and password),
protected with par2 and posted. The NZB file is written in `/nzb`, the history (with the archive passwords) in `/config`.

Image: `ghcr.io/thenormal/ngpost-deluxe` (tags `latest` and the version, for example `1.2`).

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
- To be told when the posts are done (Discord, Telegram or a webhook) add the options after the image name, for example `--notify_discord https://discord.com/api/webhooks/...`, or the `notify_*` keys in your `ngPost.conf` (see the Help page *Progress and log*). The address is a secret: keep the config folder private.
- The image is built for `amd64` (x86-64).

## Unraid

The template is in [`unraid/ngpost-deluxe.xml`](../unraid/ngpost-deluxe.xml). Until the application is in the *Community Apps* catalog:

1. Docker tab > **Add Container** > in **Template** choose *Add template URL* (or copy the XML file in `/boot/config/plugins/dockerMan/templates-user/`)
   with `https://raw.githubusercontent.com/thenormal/ngPost-Deluxe/master/unraid/ngpost-deluxe.xml`
2. Choose the folders (watch, nzb, config, work), fill in your Usenet server, user, password and newsgroup(s), then **Apply**.

## Behind a VPN (gluetun)

With a VPN your Usenet provider sees the address of the VPN and not yours. The usual way is to run a VPN container
([gluetun](https://github.com/qdm12/gluetun)) and make ngPost Deluxe use **its network**: all the traffic of ngPost goes
through the VPN, and if the VPN stops, ngPost has no network (it does not fall back on your normal connection, gluetun blocks it).
ngPost only makes outgoing connections, so you do not need to open any port.

Before you start, check that your Usenet provider accepts connections from a VPN (some of them refuse some VPN addresses).

### Docker Compose

Both containers are in the same `docker-compose.yml`. Replace the values between `<` and `>` and read the page of your VPN
service in the [gluetun wiki](https://github.com/qdm12/gluetun-wiki/tree/main/setup/providers): it lists the variables that service needs
(for example a WireGuard key, or a user name and password for OpenVPN).

```yaml
services:
  gluetun:
    image: qmcgaw/gluetun
    container_name: gluetun
    cap_add:
      - NET_ADMIN
    devices:
      - /dev/net/tun:/dev/net/tun
    environment:
      - VPN_SERVICE_PROVIDER=<your VPN service>
      - VPN_TYPE=<wireguard or openvpn>
      # ... the other variables of your VPN service (see the gluetun wiki)
    restart: unless-stopped

  ngpost-deluxe:
    image: ghcr.io/thenormal/ngpost-deluxe:latest
    container_name: ngpost-deluxe
    network_mode: "service:gluetun"      # <- uses the network (and the VPN) of gluetun
    depends_on:
      - gluetun
    environment:
      - NG_HOST=news.example.com
      - NG_PORT=563
      - NG_SSL=true
      - NG_USER=myUser
      - NG_PASS=myPassword
      - NG_GROUPS=alt.binaries.test
    volumes:
      - /path/config:/config
      - /path/watch:/watch
      - /path/nzb:/nzb
      - /path/work:/work
    restart: unless-stopped
```

Start it with `docker compose up -d`. The other settings of ngPost (folders, variables) are the ones described above.

If gluetun is in another Compose file (or started with `docker run`), give it `container_name: gluetun` and use
`network_mode: "container:gluetun"` (or `--network=container:gluetun` with `docker run`) in the ngPost container.
The gluetun wiki explains it in [Connect a container to gluetun](https://github.com/qdm12/gluetun-wiki/blob/main/setup/connect-a-container-to-gluetun.md).

### Unraid

1. Install and start gluetun first (it is in the Community Apps catalog) and give it the settings of your VPN service.
2. In the ngPost Deluxe container, switch on **Advanced View** (top right) and set **Network Type** to `None`.
3. In **Extra Parameters** write `--network=container:gluetun`, then **Apply**.
4. Make sure gluetun starts **before** ngPost Deluxe (the order of the containers in the Docker tab, or a delay on ngPost).
   After a restart of gluetun, restart ngPost Deluxe too: it must join the network again.

### Check that it works

- `docker logs gluetun` must show that the VPN is connected.
- `docker logs -f ngpost-deluxe` shows the connections of ngPost: if they fail, look at the messages about the connection and the login.
- To see the address ngPost uses, run `docker exec gluetun wget -qO- https://ifconfig.me` (it is the address of the VPN, not yours).

### Good to know

- The speed depends on the VPN: the choice of the VPN server and the number of connections (`NG_CONNECTIONS`) matter.
- If the connection works without the VPN but fails with it, try another VPN server (some are blocked by Usenet providers) or another port of your provider.
- This is a standard gluetun setup, but it has not been tested here with every VPN service: if you find something to correct, please open an issue.

## Build it yourself

```
docker build -t ngpost-deluxe .
```
