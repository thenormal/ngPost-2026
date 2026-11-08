#!/bin/sh
# ngPost Deluxe container: watch a folder and post everything that arrives in it.
#
# - /config/ngPost.conf is used if it exists (put your own one there);
#   otherwise it is generated from the NG_* environment variables.
# - /watch is the folder to monitor, /nzb receives the NZB files.
set -e

CONFIG_DIR="${NG_CONFIG_DIR:-/config}"
WATCH_DIR="${NG_WATCH_DIR:-/watch}"
NZB_DIR="${NG_NZB_DIR:-/nzb}"
WORK_DIR="${NG_WORK_DIR:-/work}"
CONF="$CONFIG_DIR/ngPost.conf"
PUID="${PUID:-99}"
PGID="${PGID:-100}"

mkdir -p "$CONFIG_DIR" "$NZB_DIR" "$WORK_DIR"

if [ ! -f "$CONF" ]; then
    missing=""
    for v in NG_HOST NG_USER NG_PASS NG_GROUPS; do
        eval "val=\${$v:-}"
        [ -n "$val" ] || missing="$missing $v"
    done
    if [ -n "$missing" ]; then
        echo "ngPost: no $CONF and these variables are not set:$missing" >&2
        echo "ngPost: either put your ngPost.conf in $CONFIG_DIR or set the variables (see docker/README.md)" >&2
        exit 1
    fi
    echo "ngPost: generating $CONF from the environment variables"
    {
        echo "# generated at the first start by the Docker image: edit it freely, it will not be overwritten"
        echo "lang = EN"
        echo "nzbPath = $NZB_DIR"
        echo "TMP_DIR = $WORK_DIR"
        echo "POST_HISTORY = $CONFIG_DIR/ngPost_history.csv"
        echo "GROUPS = $NG_GROUPS"
        echo "GROUP_POLICY = ${NG_GROUP_POLICY:-ALL}"
        echo "GEN_FROM = true"
        echo "NZB_RM_ACCENTS = true"
        echo "DISP_Progress = NONE"
        echo "RAR_PATH = /usr/bin/7z"
        echo "RAR_SIZE = ${NG_ARCHIVE_SIZE_MB:-0}"
        echo "PAR2_PCT = ${NG_PAR2_PCT:-10}"
        echo "PACK = COMPRESS, GEN_NAME, GEN_PASS, GEN_PAR2"
        echo "MONITOR_SEC_DELAY_SCAN = ${NG_SCAN_DELAY_SEC:-10}"
        echo ""
        echo "[server]"
        echo "host = $NG_HOST"
        echo "port = ${NG_PORT:-563}"
        echo "ssl = ${NG_SSL:-true}"
        echo "user = $NG_USER"
        echo "pass = $NG_PASS"
        echo "connection = ${NG_CONNECTIONS:-20}"
        echo "enabled = true"
        echo "nzbCheck = false"
    } > "$CONF"
    chmod 600 "$CONF"
fi

# --pack: compress, random name and password, par2 (PACK line of the config file)
args="--conf $CONF --pack --monitor $WATCH_DIR"
if [ "${NG_DELETE_AFTER_POST:-false}" = "true" ]; then
    args="$args --rm_posted"
fi

if [ ! -d "$WATCH_DIR" ]; then
    echo "ngPost: the folder to watch $WATCH_DIR does not exist (mount it with -v /your/folder:$WATCH_DIR)" >&2
    exit 1
fi

umask "${UMASK:-022}"

# run as PUID:PGID (99:100 is nobody:users on Unraid) when started as root
if [ "$(id -u)" = "0" ]; then
    chown "$PUID:$PGID" "$CONFIG_DIR" "$NZB_DIR" "$WORK_DIR"
    [ -f "$CONF" ] && chown "$PUID:$PGID" "$CONF"
    [ -f "$CONFIG_DIR/ngPost_history.csv" ] && chown "$PUID:$PGID" "$CONFIG_DIR/ngPost_history.csv"
    echo "ngPost: watching $WATCH_DIR as $PUID:$PGID, NZB in $NZB_DIR"
    # shellcheck disable=SC2086
    exec setpriv --reuid="$PUID" --regid="$PGID" --clear-groups ngPost $args "$@"
fi

echo "ngPost: watching $WATCH_DIR, NZB in $NZB_DIR"
# shellcheck disable=SC2086
exec ngPost $args "$@"
