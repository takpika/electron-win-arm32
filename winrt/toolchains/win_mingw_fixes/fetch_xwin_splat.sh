#!/bin/sh
# Fetches the Windows SDK / Microsoft C runtime splat the win-arm32 build uses (win_sdk_splat_dir):
# Windows SDK 10.0.22621 and the MSVC CRT 14.44.17.14 (both pinned) for arm and x64, from Microsoft's manifests via
# xwin 0.10.0, with ATL and with the PDBs Microsoft ships for the static CRT libraries
# (libcmt.arm.pdb, libvcruntime.arm.pdb, ...). The PDBs are needed because images are linked with
# /DEBUG under /WX: the CRT objects reference them, and lld-link reports a missing one as
# LNK4099 "Cannot use debug info for 'libcmt.lib(...)'" (an error under /WX), as link.exe would
# with an MSVC installation that lacked them. Directory names use Microsoft's arch notation
# (arm, x64), as in an MSVC installation. --copy leaves the unpacked cache intact (splat otherwise
# moves files out of it, so a second or interrupted run would find it incomplete).
# ATL's headers are also installed in their own directory, $OUT/atlmfc/include, as an MSVC
# installation keeps them (VC/Tools/MSVC/<version>/atlmfc/include): splat merges them into
# crt/include with the C runtime's and the C++ standard library's headers, and the win-arm32 build
# (mingw-w64 headers, libc++) must be able to use ATL without MSVC's runtime headers on its path.
# Output: $OUT (default /home/winrt/sdk/xwin-10.0.22621); a sorted sha256 manifest of every
# regular file, printed and written to $OUT.sha256.
set -eu
OUT=${OUT:-/home/winrt/sdk/xwin-10.0.22621}
CACHE=${CACHE:-/home/winrt/sdk/staging/xwin-cache}
xwin --version | grep -qx "xwin 0.10.0"
xwin --accept-license --include-atl --arch aarch,x86_64 --manifest-version 17 --sdk-version 10.0.22621 --crt-version 14.44.17.14 --cache-dir "$CACHE" \
  splat --copy --preserve-ms-arch-notation --include-debug-symbols --output "$OUT"
# ATL: exactly the ATL headers package's include directory (the same files splat copied into
# crt/include; each is checked to be identical there).
ATL=$CACHE/unpack/Microsoft.VC.14.44.17.14.ATL.Headers.base.vsix/include
rm -rf "$OUT/atlmfc"; mkdir -p "$OUT/atlmfc"
cp -a "$ATL" "$OUT/atlmfc/include"
n=0
for f in "$OUT/atlmfc/include"/*; do
  [ -f "$f" ] || continue
  cmp "$f" "$OUT/crt/include/$(basename "$f")"; n=$((n + 1))
done
[ "$n" -gt 50 ] || { echo "ERROR: only $n ATL headers" >&2; exit 1; }
echo "atlmfc/include: $n ATL headers, identical to crt/include's copies"
# The manifest is also written to $OUT.sha256 (next to the splat, not inside it): a file whose
# content changes whenever any splat file or symbolic link does: the record of exactly what this
# run installed.
# Regular files by content, symbolic links by target: any change to either changes the manifest.
( cd "$OUT" && find . \( -type f -o -type l \) | LC_ALL=C sort | while IFS= read -r f; do
    if [ -L "$f" ]; then printf 'symlink %s -> %s\n' "$f" "$(readlink "$f")"; else sha256sum "$f"; fi
  done ) > "$OUT.sha256.tmp"
# Replaced only when the content changed, so an identical re-splat leaves its timestamp alone.
if cmp -s "$OUT.sha256.tmp" "$OUT.sha256"; then rm "$OUT.sha256.tmp"; else mv "$OUT.sha256.tmp" "$OUT.sha256"; fi
cat "$OUT.sha256"
