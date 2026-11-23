#!/bin/sh
# Builds build/toolchain/win/rc/linux-arm64/rc: Chromium's resource compiler (rc.py's backend;
# build/toolchain/win/rc/README.md) for a linux-arm64 build host. Upstream ships prebuilt rc
# binaries only for linux-x64, mac and win (upload_rc_binaries.sh -> res/distrib.py of
# https://github.com/nico/hack). This builds the same program from the same source for
# aarch64, the way distrib.py builds the linux binary.
#
# Source revision: REV below is the last nico/hack commit touching res/ before Chromium 109
# branched (2022-10); res/rc.cc did not change again until 2023-08-16, so it is the rc.cc of
# the linux64/rc (sha1 1ca25446...) pinned in this tree.
#
# Compile line: res/distrib.py at REV, Linux branch (-std=c++14 -Wall -Wno-c++11-narrowing -O2
# -fno-rtti -fno-exceptions -DNDEBUG -fuse-ld=lld --sysroot), with these differences:
#  - -target aarch64-unknown-linux-gnu and the DEPS-pinned debian_bullseye_arm64 sysroot instead
#    of x86_64 / debian_sid_amd64;
#  - the compiler is clang++ of the build's toolchain (llvm-mingw-winrt, build_llvm_winrt.sh),
#    not third_party/llvm-build's, which exists only for x64 hosts;
#  - -fsigned-char: plain char is signed on x86-64 (where the pinned binary runs) but unsigned
#    on aarch64, and rc.cc computes ACCELERATORS keys from plain chars
#    (`accelerator->key = (accelerator->key << 8) + c`), so without it non-ASCII keys differ.
#
# Checks before install (all must pass):
#  1. test_rc.py at REV, all three sections: the 34 general tests (test/<name>.rc must give
#     exactly test/<name>.res, Microsoft rc.exe's output shipped with the source), the five
#     directory-search tests (/I, /cd, /fo as rc.py passes them; test/dirsearch*.res), and the
#     absolute-path test.
#  2. /utf-8 (rc.py passes it for UTF-16 inputs, which it converts to UTF-8): each UTF-16LE-BOM
#     general test, converted to UTF-8, must give the same test/<name>.res.
#  3. /showIncludes (rc.py always passes it): the bitmap test must list its four .bmp files.
#  4. char signedness: the key of `1 ACCELERATORS { "\xc3\xa9", 100 }` must be 0xC2A9, the value
#     rc.cc's expression gives with signed char (x86-64), not 0xC3A9 (unsigned char).
set -eu
REV=909d7e6dc4660ed68027b9c0da5c41fc6b2c7b62
ROOT=${WIN_MINGW_ROOT:-/home/winrt/sdk/llvm-mingw-winrt}
SYSROOT=${SYSROOT:-/home/winrt/sdk/sysroots/debian_bullseye_arm64-sysroot}
CHROMIUM_SRC=${CHROMIUM_SRC:-/home/winrt/chromium}
OUT=${OUT:-$CHROMIUM_SRC/build/toolchain/win/rc/linux-arm64/rc}

W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
git clone -q https://github.com/nico/hack "$W/hack"
git -C "$W/hack" archive "$REV" res | tar -x -C "$W"
R=$W/res; cd "$R"

"$ROOT/bin/clang++" -std=c++14 rc.cc -Wall -Wno-c++11-narrowing -O2 -fno-rtti \
  -fno-exceptions -DNDEBUG -fsigned-char -o rc -fuse-ld=lld -target aarch64-unknown-linux-gnu \
  --sysroot "$SYSROOT"

fail=0
check() { if "$@"; then echo "PASS $label"; else echo "FAIL $label"; fail=1; fi; }

# 1a. general tests
for t in language cursor bitmap icon menu menu_opts dialog_nocontrols dialog_controls \
         dialogex_nocontrols dialogex_controls stringtable stringtable_by_language \
         accelerators rcdata rcdata_inline versioninfo_fixedonly versioninfo versioninfo_slurp \
         dlginclude html html_inline custom custom_inline stringnames eval literals_int \
         literals_string unicode_simple_utf16le_bom unicode_simple_utf16le_nobom \
         unicode_utf16le_bom_custom unicode_utf16le_bom_dialog unicode_utf16le_bom_menu \
         unicode_utf16le_bom_stringtable unicode_utf16le_bom_versioninfo; do
  rm -f out.res; label=$t
  check sh -c "./rc < test/$t.rc && cmp -s out.res test/$t.res"
done

# 1b. directory search (test_rc.py: cwd = a temp dir holding cwdfile.txt, /I relative to it,
#     /cd<test dir>, /fo<res dir>/out.res)
T=$W/tmp; mkdir -p "$T"; cd "$T"; printf 'never read' > cwdfile.txt
REL=$(python3 -c "import os,sys; print(os.path.relpath(sys.argv[1], sys.argv[2]))" "$R/test" "$T")
for flag in "/Idir1" "/Idir2" "/Idir1 /Idir2" "/Idir2 /Idir1" "/Idir2/dir1"; do
  relflag=""; for f in $flag; do relflag="$relflag /I$REL/${f#/I}"; done
  suffix=$(printf '%s' "$flag" | tr -d '/' | tr ' ' '.')
  rm -f "$R/out.res"; label="dirsearch $flag"
  check sh -c "\"$R/rc\" $relflag /cd$R/test /fo$R/out.res < \"$R/test/dirsearch.rc\" && cmp -s \"$R/out.res\" \"$R/test/dirsearch$suffix.res\""
done
# 1c. absolute path reference
printf '1 RCDATA "%s"' "$T/cwdfile.txt" > abs.rc
label=abspath; check sh -c "\"$R/rc\" /cd. /fo$R/out.res < abs.rc"
cd "$R"

# 2. /utf-8
for t in unicode_simple_utf16le_bom unicode_utf16le_bom_custom unicode_utf16le_bom_dialog \
         unicode_utf16le_bom_menu unicode_utf16le_bom_stringtable unicode_utf16le_bom_versioninfo; do
  python3 -c "import sys; d=open(sys.argv[1],'rb').read(); assert d[:2]==b'\xff\xfe'; open(sys.argv[2],'wb').write(d[2:].decode('utf-16le').encode('utf-8'))" \
    "test/$t.rc" "$W/$t.utf8.rc"
  rm -f out.res; label="utf-8 $t"
  check sh -c "./rc /utf-8 < \"$W/$t.utf8.rc\" && cmp -s out.res test/$t.res"
done

# 3. /showIncludes
label="showIncludes bitmap"
check sh -c "./rc /showIncludes < test/bitmap.rc > \"$W/inc.txt\" && [ \$(grep -c '\.bmp' \"$W/inc.txt\") -eq 4 ]"

# 4. char signedness
printf '1 ACCELERATORS\nBEGIN\n  "\303\251", 100\nEND\n' > "$W/accel.rc"
rm -f out.res; ./rc /fo"$W/accel.res" < "$W/accel.rc"
label="signed char accelerator key"
check python3 -c "import sys,struct; d=open(sys.argv[1],'rb').read(); flags,key,i,pad=struct.unpack('<HHHH', d[-8:]); print('key=%#x id=%d' % (key,i)); sys.exit(0 if (key,i)==(0xc2a9,100) else 1)" "$W/accel.res"

[ "$fail" = 0 ] || { echo "rc checks failed; not installed" >&2; exit 1; }
mkdir -p "$(dirname "$OUT")"
cp rc "$OUT"
file "$OUT" 2>/dev/null || true
sha256sum "$OUT"
