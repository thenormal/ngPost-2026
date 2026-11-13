# ngPost Deluxe, command line version (no graphical interface) to run as a service on a NAS or a server
# (Unraid, Synology, a Linux box...): every file or folder dropped in /watch is compressed,
# protected with par2, posted to Usenet, and its NZB is written in /nzb.
#
#   docker build -t ngpost-deluxe .
#   docker run -d --name ngpost-deluxe \
#       -e NG_HOST=news.example.com -e NG_USER=myUser -e NG_PASS=myPass -e NG_GROUPS=alt.binaries.test \
#       -v /my/config:/config -v /my/watch:/watch -v /my/nzb:/nzb -v /my/work:/work ngpost-deluxe
#
# See docker/README.md for all the options.

# ---------- build ----------
FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential qtbase5-dev qt5-qmake libssl-dev \
    && rm -rf /var/lib/apt/lists/*
COPY src /src
WORKDIR /build
RUN qmake /src/ngPost_cmd.pro CONFIG+=release && make -j"$(nproc)" && strip ngPost

# ---------- ParPar (fast par2 generator, public domain) ----------
# the file is in the repository (release/libs): no download that can fail (GitHub answered 504 for hours)
FROM ubuntu:22.04 AS parpar
RUN apt-get update && apt-get install -y --no-install-recommends xz-utils \
    && rm -rf /var/lib/apt/lists/*
COPY release/libs/parpar-v0.4.6-linux-static-amd64.xz /parpar.xz
RUN echo "25da379e76f968608b6f24d332b93ca75e0c8a88b92a6ebbb21cbbb0d00c1f18  /parpar.xz" | sha256sum -c - \
    && xz -d /parpar.xz && chmod +x /parpar && /parpar --help >/dev/null

# ---------- runtime ----------
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        libqt5core5a libqt5network5 libssl3 ca-certificates p7zip-full mediainfo \
    && rm -rf /var/lib/apt/lists/*

# parpar must be next to ngPost: it is then used automatically
COPY --from=build  /build/ngPost /usr/local/bin/ngPost
COPY --from=parpar /parpar       /usr/local/bin/parpar
COPY docker/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

LABEL org.opencontainers.image.title="ngPost Deluxe" \
      org.opencontainers.image.description="Usenet poster: watches a folder, compresses, adds par2 and posts" \
      org.opencontainers.image.source="https://github.com/thenormal/ngPost-Deluxe" \
      org.opencontainers.image.licenses="GPL-3.0-or-later"

# 99:100 (nobody:users) is the user of Unraid
# /work: temporary folder for the archives and the par2 (as big as the biggest post: not in the container layer)
ENV PUID=99 PGID=100 UMASK=022 HOME=/config NG_WORK_DIR=/work
VOLUME ["/config", "/watch", "/nzb", "/work"]
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
