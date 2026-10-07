#!/bin/sh
# Installs third_party/node/linux/node-linux-arm64: the official Node.js build for linux-arm64
# of the version Chromium pins for every other host (DEPS hook node_linux64 bucket
# chromium-nodejs/16.13.0; mac arm64/x64 use the same version). Upstream Chromium 109 ships
# node only as linux-x64 for Linux hosts; third_party/node/node.py selects this directory on
# linux aarch64 hosts, as it selects node-darwin-arm64 on arm64 Macs.
# The archive is verified against its pinned sha256 (= nodejs.org's SHASUMS256.txt for v16.13.0).
set -eu
V=16.13.0
SHA256=93a0d03f9f802353cb7052bc97a02cd9642b49fa985671cdc16c99936c86d7d2
CHROMIUM_SRC=${CHROMIUM_SRC:-/home/winrt/chromium}
D=$CHROMIUM_SRC/third_party/node/linux
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
curl -sfL "https://nodejs.org/dist/v$V/node-v$V-linux-arm64.tar.xz" -o "$W/node.tar.xz"
curl -sfL "https://nodejs.org/dist/v$V/SHASUMS256.txt" | grep " node-v$V-linux-arm64.tar.xz\$"
echo "$SHA256  $W/node.tar.xz" | sha256sum -c -
tar -xJf "$W/node.tar.xz" -C "$W"
rm -rf "$D/node-linux-arm64"
mv "$W/node-v$V-linux-arm64" "$D/node-linux-arm64"
"$D/node-linux-arm64/bin/node" -e 'console.log(process.version, process.arch)'
