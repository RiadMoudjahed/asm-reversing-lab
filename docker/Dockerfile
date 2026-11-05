# asm-reversing-lab: reproducible lab image.
# Build:  docker build -t asm-reversing-lab docker/
# Use:    make shell
FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive
ARG GEF_VERSION=2024.06
ARG RIZIN_VERSION=0.9.1
ARG ILSPYCMD_VERSION=8.2.0.7535

ENV ARL_IN_LAB=1 \
    LANG=C.UTF-8 \
    SOURCE_DATE_EPOCH=0 \
    DOTNET_CLI_TELEMETRY_OPTOUT=1 \
    DOTNET_NOLOGO=1 \
    WINEDEBUG=-all \
    WINEPREFIX=/home/lab/.wine

RUN dpkg --add-architecture i386 \
 && apt-get update \
 && apt-get install -y --no-install-recommends \
      ca-certificates curl git make file xxd hexyl binwalk \
      nasm gcc g++ gcc-multilib g++-multilib binutils \
      gdb strace ltrace xz-utils \
      mingw-w64 wine64 \
      yara \
      python3 python3-pip python3-venv \
      dotnet-sdk-8.0 \
      nodejs npm \
 && rm -rf /var/lib/apt/lists/*

# Python analysis libraries, isolated from the system interpreter.
RUN python3 -m venv /opt/venv \
 && /opt/venv/bin/pip install --no-cache-dir \
      pyelftools pefile capstone unicorn yara-python codespell
ENV PATH="/opt/venv/bin:/root/.dotnet/tools:/home/lab/.dotnet/tools:${PATH}"

# Rizin is not packaged for Ubuntu 24.04; use the project's pinned static release.
RUN curl -fsSL -o /tmp/rizin.tar.xz \
      "https://github.com/rizinorg/rizin/releases/download/v${RIZIN_VERSION}/rizin-v${RIZIN_VERSION}-static-x86_64.tar.xz" \
 && mkdir -p /opt/rizin && tar -xJf /tmp/rizin.tar.xz -C /opt/rizin \
 && ln -s "$(find /opt/rizin -type f -name rizin -perm -u+x | head -n1)" /usr/local/bin/rizin \
 && for t in rz-bin rz-asm rz-hash rz-diff; do b="$(find /opt/rizin -type f -name $t | head -n1)"; [ -n "$b" ] && ln -s "$b" /usr/local/bin/$t; done; \
    rm /tmp/rizin.tar.xz; rizin -v

# Markdown lint (version compatible with Node 18 from Ubuntu 24.04).
RUN npm install -g markdownlint-cli2@0.13.0 && npm cache clean --force

# GEF (GDB Enhanced Features), pinned to a release tag.
RUN curl -fsSL -o /usr/local/share/gef.py \
      "https://raw.githubusercontent.com/hugsy/gef/${GEF_VERSION}/gef.py"
COPY gdbinit /etc/gdb/gdbinit

# Ubuntu ships only /usr/lib/wine/wine64; expose it as `wine` and use a 64-bit-only prefix.
RUN printf '#!/bin/sh\nexec /usr/lib/wine/wine64 "$@"\n' > /usr/local/bin/wine && chmod +x /usr/local/bin/wine
ENV WINEARCH=win64
# ilspycmd 8.2 targets .NET 6; let it run on the .NET 8 runtime shipped in this image.
ENV DOTNET_ROLL_FORWARD=Major

# Unprivileged user; the repository is mounted at /lab.
# The user's uid/gid match the host user's (set by `make image`) so the mounted repository
# is writable. Ubuntu's stock `ubuntu` user already occupies uid 1000, so remove it first.
ARG LAB_UID=1000
ARG LAB_GID=1000
RUN userdel -r ubuntu 2> /dev/null; \
    groupadd -g "${LAB_GID}" lab && useradd -m -u "${LAB_UID}" -g "${LAB_GID}" -s /bin/bash lab
USER lab
# Create the Wine prefix now so the first run in the lab is quiet.
RUN wine wineboot --init > /dev/null 2>&1; wineserver -w; test -d /home/lab/.wine
RUN dotnet tool install --global ilspycmd --version "${ILSPYCMD_VERSION}"
WORKDIR /lab
CMD ["bash"]
