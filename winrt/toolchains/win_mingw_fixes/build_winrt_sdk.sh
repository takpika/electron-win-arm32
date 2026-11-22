#!/bin/sh
# Builds the toolchain description build/toolchain/win/setup_toolchain.py reads in its
# hermetic-toolchain mode (visual_studio_path / windows_sdk_path / wdk_path) for the Windows on
# ARM32 build and its x64 helper toolchain: vs/win_sdk/bin/SetEnv.<cpu>.json and vs/bin/cl.exe (a
# link to the toolchain's clang-cl; setup_toolchain.py locates the compiler directory by cl.exe on
# PATH). The INCLUDE/LIB lists are what setup_toolchain.py requires to be non-empty; the arm and x64
# toolchains do not use them (build/config/win/BUILD.gn and build/toolchain/win/toolchain.gni set
# their include and library flags). They name only directories of the Windows SDK/CRT splat
# (fetch_xwin_splat.sh), so the output does not depend on $OUT.
# Output: $OUT (default /home/winrt/sdk/winrt-sdk).
set -eu
ROOT=${WIN_MINGW_ROOT:-/home/winrt/sdk/llvm-mingw-winrt}
XWIN=${WIN_SDK_SPLAT:-/home/winrt/sdk/xwin-10.0.22621}
OUT=${OUT:-/home/winrt/sdk/winrt-sdk}

rm -rf "$OUT"; mkdir -p "$OUT/vs/bin" "$OUT/vs/win_sdk/bin"
for cpu in arm x64; do
  cat > "$OUT/vs/win_sdk/bin/SetEnv.$cpu.json" <<EOF
{
  "env": {
    "VSINSTALLDIR": [["."]],
    "PATH": [["bin"]],
    "INCLUDE": [["$XWIN/sdk/include/um"], ["$XWIN/sdk/include/shared"], ["$XWIN/sdk/include/winrt"]],
    "LIB": [["$XWIN/crt/lib/$cpu"], ["$XWIN/sdk/lib/ucrt/$cpu"], ["$XWIN/sdk/lib/um/$cpu"]]
  }
}
EOF
done
ln -s "$ROOT/bin/clang-cl" "$OUT/vs/bin/cl.exe"
find "$OUT" -type f | sort | xargs sha256sum
