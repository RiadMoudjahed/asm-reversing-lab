#!/usr/bin/env bash
# Native install for Ubuntu 24.04 users who prefer not to use Docker.
# Installs the toolchain of docker/Dockerfile except Rizin and Ghidra (install those separately).
# Usage: sudo scripts/install-native.sh
set -euo pipefail

if [[ $EUID -ne 0 ]]; then
  echo "Run as root: sudo $0" >&2
  exit 1
fi

GEF_VERSION="2024.06"
ILSPYCMD_VERSION="8.2.0.7535"
TARGET_USER="${SUDO_USER:-root}"

dpkg --add-architecture i386
apt-get update
apt-get install -y --no-install-recommends \
  ca-certificates curl git make file xxd hexyl binwalk \
  nasm gcc g++ gcc-multilib g++-multilib binutils \
  gdb strace ltrace xz-utils \
  mingw-w64 wine64 yara \
  python3 python3-pip python3-venv \
  nodejs npm

# .NET SDK for the managed-code module (packaged in Ubuntu 24.04).
if ! apt-get install -y --no-install-recommends dotnet-sdk-8.0; then
  echo "warning: dotnet-sdk-8.0 is not available here; the examples of module 22 will not build" >&2
fi

python3 -m venv /opt/arl-venv
/opt/arl-venv/bin/pip install --no-cache-dir \
  pyelftools pefile capstone unicorn yara-python codespell

# Markdown lint, as in the image (version compatible with Node 18).
npm install -g markdownlint-cli2@0.13.0

# Ubuntu ships only /usr/lib/wine/wine64; expose it as `wine` when no `wine` command exists.
if ! command -v wine >/dev/null && [[ -x /usr/lib/wine/wine64 ]]; then
  printf '#!/bin/sh\nexec /usr/lib/wine/wine64 "$@"\n' > /usr/local/bin/wine
  chmod +x /usr/local/bin/wine
fi

curl -fsSL -o /usr/local/share/gef.py \
  "https://raw.githubusercontent.com/hugsy/gef/${GEF_VERSION}/gef.py"
mkdir -p /etc/gdb
install -m 0644 "$(dirname "$0")/../docker/gdbinit" /etc/gdb/gdbinit

# ILSpy command line for the user who ran sudo.
if command -v dotnet >/dev/null; then
  sudo -u "$TARGET_USER" -H dotnet tool install --global ilspycmd --version "$ILSPYCMD_VERSION" || \
    echo "warning: ilspycmd could not be installed; module 22 needs it" >&2
fi

cat <<MSG
Install complete.
Add these lines to your shell profile (for example ~/.bashrc), then open a new terminal:
  source /opt/arl-venv/bin/activate
  export PATH="\$HOME/.dotnet/tools:\$PATH" DOTNET_ROLL_FORWARD=Major WINEARCH=win64
Run lab commands natively with:  make NATIVE=1 <target>
Install Ghidra and Rizin separately (see modules/00-setup); the Docker image bundles Rizin.
MSG
