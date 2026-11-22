#!/bin/sh
# Installs third_party/lzma_sdk/bin/host_platform/{7za,7zr,7zz}: the host 7-Zip that
# chrome/tools/build/win/create_installer_archive.py (GetLZMAExec) runs on non-Windows hosts
# for //chrome/installer/mini_installer:mini_installer_archive (chrome.7z, setup.ex_,
# chrome.packed.7z -> mini_installer.exe). DEPS:1558-1567 declares it as the cipd package
# infra/3pp/tools/7z/${platform} (version:2@22.01, condition checkout_win); the provisioned
# checkout never populated it (the directory holds only .gitignore, while the sibling
# bin/win64 entry of the same DEPS pair is populated).
# For linux-arm64 the DEPS pin does not exist: infra/3pp/tools/7z/linux-arm64 has no
# version:2@22.01 tag; its only tagged version is version:3@26.03 (instance sha256 below,
# registered 2026-09-17). Using it deviates from the pinned 7-Zip version (26.03 vs 22.01);
# the package is verified against that digest.
#
# Like fetch_node_linux_arm64.sh, this records the provenance of a src-tree host tool that
# is installed separately.
set -eu
PKG=infra/3pp/tools/7z/linux-arm64
TAG=version:3@26.03
SHA256=7a50842305efbb38f4a1cd4763dac120bcd255915465ab5d6c82396befde7fbd
CHROMIUM_SRC=${CHROMIUM_SRC:-/home/winrt/chromium}
D=$CHROMIUM_SRC/third_party/lzma_sdk/bin/host_platform
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
curl -sfL "https://chrome-infra-packages.appspot.com/dl/$PKG/+/$TAG" -o "$W/7z.zip"
echo "$SHA256  $W/7z.zip" | sha256sum -c -
python3 -c "import sys, zipfile; zipfile.ZipFile(sys.argv[1]).extractall(sys.argv[2])" "$W/7z.zip" "$W/x"
for b in 7za 7zr 7zz; do
  test -f "$W/x/$b"
  install -m 0755 "$W/x/$b" "$D/$b"
done
"$D/7za" i | head -n 3
