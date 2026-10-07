#!/bin/bash
# build.sh <minutes>: builds Electron into out/electron for at most <minutes>.
# Exit 0 = everything built, 3 = time is up (continue in the next job), anything else = failure.
# args.gn: Electron's release arguments + the port's win-arm32 arguments (winrt_arm_args.gni).
set -euo pipefail
MINUTES=$1
W=/home/winrt
S=$W/gc/src
O=out/electron
ARGS=$W/toolchains/win_mingw_fixes/winrt_arm_args.gni
cd "$S"

# Case-insensitive views (as Chromium's Linux->Windows builds see the SDK) of the Windows SDK
# splat, the port toolchain and the toolchain supplement dirs: a lowercase backing copy filled
# through ciopfs, mounted read-only over the original path. The supplement dirs need it too:
# the tree includes their headers in either case (e.g. <DelayIMP.h> and <delayimp.h>), which
# the original build host's case-insensitive file system resolved to one file.
for D in sdk/xwin-10.0.22621 sdk/llvm-mingw-winrt toolchains/win_mingw_fixes toolchains/win_sdk_supplement; do
  C=$W/ci/$(basename "$D")
  if [ ! -d "$C" ]; then
    mkdir -p "$C.new" /mnt/ci
    ciopfs -o use_ino "$C.new" /mnt/ci
    python3 "$S/electron/winrt/ci/ciopfs-fill" "$W/$D" /mnt/ci
    fusermount -u /mnt/ci
    mv "$C.new" "$C"
  fi
  mountpoint -q "$W/$D" ||
    ciopfs -o ro,use_ino,allow_other,nonempty,default_permissions "$C" "$W/$D"
done

mkdir -p "$O"
printf 'import("//electron/build/args/release.gn")\nimport("%s")\n' "$ARGS" > "$O/args.gn.new"
if ! cmp -s "$O/args.gn.new" "$O/args.gn" || [ ! -f "$O/build.ninja" ]; then
  mv "$O/args.gn.new" "$O/args.gn"
  gn gen "$O"
else
  rm "$O/args.gn.new"
fi

# The release: Electron's dist zip (electron.exe's link also writes its import library
# electron.lib, node.lib for native addons) and the node headers for addon builds.
T="electron:electron_dist_zip third_party/electron_node:tar_headers"
# Headers chrome's sources include without a GN dependency on their generators (a build of
# `all` happens to generate them first); generated before everything else.
FIRST=
for t in gen/chrome/browser/password_manager/password_manager_buildflags.h \
         gen/components/password_manager/services/csv_password/public/mojom/csv_password_parser.mojom.h; do
  ninja -C "$O" -t query "$t" >/dev/null 2>&1 && FIRST="$FIRST $t"
done
# Electron's node steps (config.gypi, the npm-built bundles) run node and a host C compiler.
export CC=/opt/llvm-mingw/bin/clang CXX=/opt/llvm-mingw/bin/clang++
END=$(( $(date +%s) + MINUTES * 60 ))
[ -z "$FIRST" ] || ninja -C "$O" $FIRST
set +e
left=$(( (END - $(date +%s)) / 60 ))
[ "$left" -gt 0 ] || { echo "time is up; continuing in the next job"; exit 3; }
timeout --signal=INT --kill-after=10m "${left}m" ninja -k 0 -C "$O" $T
rc=$?
set -e
[ $rc -eq 124 ] && { echo "time is up; continuing in the next job"; exit 3; }
exit $rc
