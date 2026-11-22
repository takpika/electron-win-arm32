#!/bin/sh
# Builds the runtime pieces that Windows on ARM32 images link next to Microsoft's static CRT
# (libcmt/libvcruntime/libucrt, the configuration /MT gives upstream). The port compiles with
# llvm-mingw's clang for armv7-w64-windows-gnu: Itanium C++ ABI and mingw-w64's headers. What a
# MinGW link would take from llvm-mingw's DLL-UCRT builds is built here for the static CRT:
#   libunwind.a         the unwinder under Chromium's in-tree libc++abi (which GN builds, as it
#                       does for every Itanium-ABI platform; buildtools/third_party/libc++).
#   libssp_crt.a        mingw-w64's ssp/ (__stack_chk_guard, __stack_chk_fail) for
#                       -fstack-protector code.
#   libdso_handle.a     __dso_handle, the per-image handle Itanium-ABI code passes to
#                       __cxa_thread_atexit (clang references it for thread_local objects with
#                       destructors). MinGW links take it from mingw-w64-crt (crt/tls_atexit.c),
#                       ELF links from crtbegin; the Microsoft CRT has none. Defined as crtbegin
#                       does for a shared object (its own address): unique per image.
#                       __cxa_thread_atexit itself is libc++abi's (buildtools/third_party/libc++abi).
#   crt_aliases.obj     the symbol aliases mingw-w64's UCRT import library defines
#                       (def-include/crt-aliases.def.in with UCRTBASE: fseeko == fseek,
#                       fseeko64 == _fseeki64, strcasecmp == _stricmp, ...) that Microsoft's
#                       static CRT and OLDNAMES.lib lack, as /alternatename directives (they only
#                       take effect for a symbol nothing defines). Also the long double math
#                       functions that Microsoft's <math.h> defines as __inline forwards to the
#                       double function (frexpl -> frexp, ldexpl -> ldexp, ...; the Microsoft CRT
#                       exports no symbol for them): mingw-w64's <math.h> declares them extern,
#                       so they are aliases of the double function the SDK header forwards to,
#                       exact because long double is double here (arm; -mlong-double-64 on x64).
#                       And mingw-w64's __mingw_<f> printf/scanf entry points (libmingwex) as the
#                       UCRT <f> that __USE_MINGW_ANSI_STDIO=0 gives every plain call. And the
#                       two names that join this build's Itanium-ABI code to Microsoft's atls.lib
#                       (ATL::_AtlBaseModule; atls.lib's _AtlInitializeCriticalSectionEx).
#   libsdk_inlines.a    functions the Windows SDK defines only inline in its headers, which no
#                       Microsoft library exports and mingw-w64's headers declare extern (a MinGW
#                       link takes them from mingw-w64's own libraries), each compiled from the SDK
#                       header's own body: the <math.h> float forwards the static CRT lacks
#                       (frexpf; x64 also fabsf; unlike long double they cannot be aliases, a float
#                       is passed and returned differently from a double) and <ws2tcpip.h>'s
#                       gai_strerrorA/W (FormatMessage into a static buffer; ws2_32.lib is an
#                       import library only).
#   dinput8.lib (arm)   the ARM32 DirectInput 8 import library the Windows SDK lacks for arm,
#                       llvm-mingw's libdinput8.a under the name the build links.
# llvm-mingw's copies of these are built for its DLL UCRT: their CRT calls reference __imp_<fn>
# (dllimport), which a static CRT does not satisfy, and their constructors are in .ctors, which
# only MinGW's startup code runs. Here they are compiled with
#   -D_CRTIMP= -D_SECIMP=  mingw-w64's headers declare CRT functions without dllimport (the
#                          switch mingw-w64 uses when building its CRT; MSVC's /MT headers do the
#                          same with _ACRTIMP);
#   -fuse-init-array       static constructors in .CRT$XCU, run by the Microsoft CRT startup
#                          (build_llvm_winrt.sh's llvm_mingw_crt_init_array.patch);
#   -mguard=cf             as llvm-mingw builds them.
# Sources: libunwind from llvm-project at the toolchain's pin, configured like llvm-mingw's
# build-libcxx.sh (static only); mingw-w64 at llvm-mingw 20260922's pinned commit.
# Output: $OUT/<cpu>/{libunwind.a,libssp_crt.a,libdso_handle.a,crt_aliases.obj,
#         libsdk_inlines.a}, and $OUT/arm/dinput8.lib.
set -eu
PIN=85ac560262434c9ccfc0c183ec22d4138ed647fb
MINGW_W64=57b595039040eaa15bece85b7cc71d952281b269
MINGW_W64_SHA256=a68816e314290facd5da1ac96c7eead7e6de6ea4f709a1fbdcf8185497c57eb2
SDK=/home/winrt/sdk
ROOT=${WIN_MINGW_ROOT:-$SDK/llvm-mingw-winrt}
OUT=${OUT:-$SDK/winrt-crt}
XWIN=${WIN_SDK_SPLAT:-$SDK/xwin-10.0.22621}
CMAKE="python3 -c 'import sys; sys.path.insert(0, \"$SDK/tools/cmake\"); import cmake; sys.exit(cmake.cmake())'"
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT

fetch() {  # fetch <url> <file> <sha256>
  curl -sfL "$1" -o "$2"
  echo "$3  $2" | sha256sum -c - >/dev/null || { echo "ERROR: $1 sha256 mismatch" >&2; exit 1; }
  echo "fetched $1 sha256 $3"
}

git -C "$SDK/src/llvm-project" rev-parse HEAD | grep -qx "$PIN"
mkdir -p "$W/llvm"
git -C "$SDK/src/llvm-project" archive "$PIN" cmake llvm/cmake llvm/utils/llvm-lit runtimes \
  libunwind | tar -x -C "$W/llvm"
fetch "https://github.com/mingw-w64/mingw-w64/archive/$MINGW_W64.tar.gz" "$W/mingw.tar.gz" "$MINGW_W64_SHA256"
tar -xzf "$W/mingw.tar.gz" -C "$W"
MS=$W/mingw-w64-$MINGW_W64/mingw-w64-crt
CRTFLAGS="-mguard=cf -D_CRTIMP= -D_SECIMP= -fuse-init-array"
rm -rf "$OUT"
for cpu in arm x64; do
  case $cpu in
    arm) arch=armv7; ldflag= ;;
    # the x64 helper toolchain compiles with 64-bit long double (build/config/win/BUILD.gn)
    x64) arch=x86_64; ldflag=-mlong-double-64 ;;
  esac
  mkdir -p "$OUT/$cpu"

  # --- libunwind
  B=$W/unwind-$cpu; mkdir -p "$B"
  eval $CMAKE -G Ninja -S "$W/llvm/runtimes" -B "$B" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER="$ROOT/bin/$arch-w64-mingw32-clang" \
    -DCMAKE_CXX_COMPILER="$ROOT/bin/$arch-w64-mingw32-clang++" \
    -DCMAKE_CXX_COMPILER_TARGET=$arch-w64-windows-gnu -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER_WORKS=TRUE -DCMAKE_CXX_COMPILER_WORKS=TRUE \
    -DCMAKE_AR="$ROOT/bin/llvm-ar" -DCMAKE_RANLIB="$ROOT/bin/llvm-ranlib" \
    -DLLVM_ENABLE_RUNTIMES=libunwind \
    -DLIBUNWIND_USE_COMPILER_RT=TRUE -DLIBUNWIND_ENABLE_SHARED=OFF -DLIBUNWIND_ENABLE_STATIC=ON \
    -DCMAKE_C_FLAGS_INIT="'$CRTFLAGS $ldflag'" -DCMAKE_CXX_FLAGS_INIT="'$CRTFLAGS $ldflag'" > "$W/unwind-$cpu.log" 2>&1 ||
    { tail -n 30 "$W/unwind-$cpu.log" >&2; exit 1; }
  ninja -C "$B" unwind_static
  cp "$B/lib/libunwind.a" "$OUT/$cpu/"

  # --- ssp
  for f in stack_chk_guard stack_chk_fail; do
    "$ROOT/bin/$arch-w64-mingw32-clang" -O2 $CRTFLAGS $ldflag -mno-incremental-linker-compatible \
      -c "$MS/ssp/$f.c" -o "$W/$cpu-$f.o"
  done
  "$ROOT/bin/llvm-ar" --format=gnu crsD "$OUT/$cpu/libssp_crt.a" "$W/$cpu-stack_chk_guard.o" "$W/$cpu-stack_chk_fail.o"

  # --- __dso_handle
  echo 'void *__dso_handle = &__dso_handle;' > "$W/dso_handle.c"
  "$ROOT/bin/$arch-w64-mingw32-clang" -O2 $CRTFLAGS $ldflag -mno-incremental-linker-compatible \
    -c "$W/dso_handle.c" -o "$W/$cpu-dso_handle.o"
  "$ROOT/bin/llvm-ar" --format=gnu crsD "$OUT/$cpu/libdso_handle.a" "$W/$cpu-dso_handle.o"

  # --- crt_aliases.obj
  # The long double aliases below are exact only where long double is double.
  ldsize=$(echo __SIZEOF_LONG_DOUBLE__ | "$ROOT/bin/$arch-w64-mingw32-clang" $ldflag -E -P -x c -)
  [ "$ldsize" = 8 ] || { echo "ERROR: $cpu long double is $ldsize bytes, not 8" >&2; exit 1; }
  # Preprocessed by the target's compiler with the build's long double, as mingw-w64-crt's
  # Makefile does ($(CPP)): the list depends on the architecture (def-include/func.def.in tests
  # __arm__/__x86_64__/...) and on __SIZEOF_LONG_DOUBLE__ (F_LD64: acosl == acos, ...).
  "$ROOT/bin/$arch-w64-mingw32-clang" $ldflag -E -P -x c -DUCRTBASE -I"$MS/def-include" "$MS/def-include/crt-aliases.def.in" > "$W/aliases-$cpu.txt"
  "$ROOT/bin/llvm-nm" --defined-only "$XWIN/sdk/lib/ucrt/$cpu/libucrt.lib" "$XWIN/crt/lib/$cpu/libvcruntime.lib" \
    "$XWIN/crt/lib/$cpu/libcmt.lib" "$XWIN/crt/lib/$cpu/OLDNAMES.lib" "$XWIN/crt/lib/$cpu/legacy_stdio_definitions.lib" \
    > "$W/nm-$cpu.txt"
  awk 'NF>=2{print $NF}' "$W/nm-$cpu.txt" | sort -u > "$W/defined-$cpu.txt"
  # ATL: Microsoft's atls.lib (splat: crt/lib/<cpu>, linked through <atlbase.h>'s
  # `#pragma comment(lib, "atls.lib")`) is compiled for the MSVC C++ ABI. Its atlbase.obj defines
  # the module object ATL::_AtlBaseModule (and runs its constructor from .CRT$XCU) and the three
  # CAtlBaseModule members <atlcore.h> declares without a body (AddResourceInstance,
  # RemoveResourceInstance, GetHInstanceAt; bool/HINSTANCE results, `this` and the argument in
  # the first argument registers -- the same convention in both C++ ABIs on arm and x64 Windows)
  # under their MSVC names; this build's code refers to them by their Itanium names. The only
  # name atls.lib in turn
  # expects from the caller is ATL::_AtlInitializeCriticalSectionEx, an `extern inline` function
  # <atlwinverapi.h> defines for the user's translation unit to emit; with this build's
  # NTDDI_VERSION its body is exactly `return ::InitializeCriticalSectionEx(...)` (same
  # parameters; __cdecl and WINAPI are the same convention on arm and x64). Both are aliases:
  # the Itanium name -> the MSVC-named object, and the MSVC-named function -> kernel32's
  # InitializeCriticalSectionEx. The recipe fails if atls.lib's external needs or the header
  # body are anything else.
  ATLS=$XWIN/crt/lib/$cpu/atls.lib
  ATLBASE=$("$ROOT/bin/llvm-nm" --defined-only "$ATLS" | awk '$2=="B"||$2=="D"{print $3}' | grep '^?_AtlBaseModule@ATL@@3V' | sort -u)
  [ "$(echo "$ATLBASE" | wc -l)" = 1 ] && [ -n "$ATLBASE" ] || { echo "ERROR: $cpu atls.lib _AtlBaseModule: '$ATLBASE'" >&2; exit 1; }
  for m in AddResourceInstance RemoveResourceInstance GetHInstanceAt; do
    v=$("$ROOT/bin/llvm-nm" --defined-only "$ATLS" | awk '$2=="T"{print $3}' | grep "^?$m@CAtlBaseModule@ATL@@" | sort -u)
    [ "$(echo "$v" | wc -l)" = 1 ] && [ -n "$v" ] || { echo "ERROR: $cpu atls.lib CAtlBaseModule::$m: '$v'" >&2; exit 1; }
    eval "ATL_$m=\$v"
  done
  "$ROOT/bin/llvm-nm" --defined-only "$ATLS" "$XWIN/crt/lib/$cpu/libvcruntime.lib" "$XWIN/crt/lib/$cpu/libcmt.lib" \
    "$XWIN/sdk/lib/ucrt/$cpu/libucrt.lib" | awk 'NF>=3{print $3}' | sort -u > "$W/atls-def-$cpu.txt"
  "$ROOT/bin/llvm-nm" -u "$ATLS" | awk '{print $NF}' | grep '^?' | sort -u > "$W/atls-und-$cpu.txt"
  ATLINIT=$(comm -23 "$W/atls-und-$cpu.txt" "$W/atls-def-$cpu.txt")
  case "$ATLINIT" in
    '?_AtlInitializeCriticalSectionEx@ATL@@'*) [ "$(echo "$ATLINIT" | wc -l)" = 1 ] ;;
    *) false ;;
  esac || { echo "ERROR: $cpu atls.lib expects from its caller: $ATLINIT" >&2; exit 1; }
  tr -d '\r' < "$XWIN/atlmfc/include/atlwinverapi.h" |
    grep -A4 '^extern inline BOOL __cdecl _AtlInitializeCriticalSectionEx(_Out_ LPCRITICAL_SECTION lpCriticalSection, _In_ DWORD dwSpinCount, _In_ DWORD Flags)$' |
    grep -qx '	return ::InitializeCriticalSectionEx(lpCriticalSection, dwSpinCount, Flags);' ||
    { echo "ERROR: <atlwinverapi.h> _AtlInitializeCriticalSectionEx is not a plain forward" >&2; exit 1; }
  "$ROOT/bin/llvm-nm" --defined-only "$XWIN/sdk/lib/um/$cpu/kernel32.lib" | grep -q ' T InitializeCriticalSectionEx$' ||
    { echo "ERROR: $cpu kernel32.lib has no InitializeCriticalSectionEx" >&2; exit 1; }
  export ATLBASE ATLINIT ATL_AddResourceInstance ATL_RemoveResourceInstance ATL_GetHInstanceAt
  python3 - "$W/aliases-$cpu.txt" "$W/defined-$cpu.txt" "$W/aliases-$cpu.s" "$XWIN/sdk/include/ucrt/math.h" \
    "$ROOT/generic-w64-mingw32/include/math.h" "$ROOT/generic-w64-mingw32/include/stdio.h" <<'PY'
import re, sys
defined = set(open(sys.argv[2]).read().split())
pairs = []
for line in open(sys.argv[1]):
    m = re.match(r'\s*(\w+)\s+(?:DATA\s+)?==\s*(\w+)\s*$', line)
    if not m or m.group(1) in defined: continue
    if m.group(2) not in defined:
        sys.exit('ERROR: alias target %s of %s is in no static library' % (m.group(2), m.group(1)))
    pairs.append(m.groups())
# The SDK's __inline long double forwards: `long double __CRTDECL f(<params>) { return g(<args>); }`
# (modfl: `double _F, _I; _F = modf((double)_X, &_I); *_Y = _I; return _F;`). Each body must be
# exactly a forward: the arguments are the parameters in order, a `long double` parameter passed
# as `(double)<name>` and any other parameter passed unchanged; anything else fails the recipe.
# Names mingw-w64's <math.h> #defines to another name (e.g. `#define _hypotl hypotl`) can never
# be referenced and get no alias.
sdk = open(sys.argv[4]).read()
mingw = open(sys.argv[5]).read()
macros = set(re.findall(r'^\s*#\s*define\s+(\w+)\s+\w+\s*$', mingw, re.M))
parsed = 0
for m in re.finditer(r'__inline long double __CRTDECL (\w+)\(([^)]*)\)\s*\{(.*?)\}', sdk, re.S):
    name, params, body = m.group(1), m.group(2), ' '.join(m.group(3).split())
    plist = []
    for prm in params.split(','):
        prm = re.sub(r'\b_(In|Out|Inout)_\b', '', prm).strip()
        pm = re.fullmatch(r'(.*?)\s*(\w+)', prm)
        plist.append((' '.join(pm.group(1).split()), pm.group(2)))
    r = re.fullmatch(r'return (\w+)\((.*)\);', body)
    if r:
        target = r.group(1)
        args = [a.strip() for a in r.group(2).split(',')]
        want = [('(double)' + n) if t == 'long double' else n for t, n in plist]
        ok = args == want
    elif name == 'modfl':
        target = 'modf'
        ok = (plist == [('long double', '_X'), ('long double*', '_Y')] and
              body == 'double _F, _I; _F = modf((double)_X, &_I); *_Y = _I; return _F;')
    else:
        ok = False
    if not ok:
        sys.exit('ERROR: SDK long double inline %s is not a plain forward: %s' % (name, body))
    parsed += 1
    if name in defined or name in macros or any(a == name for a, _ in pairs): continue
    if target not in defined:
        sys.exit('ERROR: forward target %s of %s is in no static library' % (target, name))
    pairs.append((name, target))
if parsed < 20:
    sys.exit('ERROR: only %d SDK long double forwards found' % parsed)
# mingw-w64's <stdio.h> declares its own printf/scanf family as __mingw_<f> (libmingwex, not
# linked here) and, with __USE_MINGW_ANSI_STDIO=0 (build/config/win/BUILD.gn), routes the plain
# <f> to the Microsoft UCRT. Code that names __mingw_<f> directly (flac's win_utf8_io.c:
# __mingw_vsnprintf) gets that same UCRT <f>: an alias for every __mingw_<f> the header declares
# whose <f> the static libraries define (the UCRT printf/scanf family is C99). Names with no
# such <f> (__mingw_asprintf, the _FORTIFY_SOURCE helpers, ...) get none.
stdio = open(sys.argv[6]).read()
nmingw = 0
for base in sorted(set(re.findall(r'\b__mingw_(\w+)\s*\(', stdio))):
    name = '__mingw_' + base
    if base not in defined or name in defined or any(a == name for a, _ in pairs): continue
    pairs.append((name, base))
    nmingw += 1
if nmingw < 10:
    sys.exit('ERROR: only %d __mingw_ stdio aliases' % nmingw)
import os
pairs.append(('_ZN3ATL14_AtlBaseModuleE', os.environ['ATLBASE']))  # Itanium: ATL::_AtlBaseModule
# Itanium names of ATL::CAtlBaseModule::AddResourceInstance(HINSTANCE),
# RemoveResourceInstance(HINSTANCE) and GetHInstanceAt(int).
pairs.append(('_ZN3ATL14CAtlBaseModule19AddResourceInstanceEP11HINSTANCE__', os.environ['ATL_AddResourceInstance']))
pairs.append(('_ZN3ATL14CAtlBaseModule22RemoveResourceInstanceEP11HINSTANCE__', os.environ['ATL_RemoveResourceInstance']))
pairs.append(('_ZN3ATL14CAtlBaseModule14GetHInstanceAtEi', os.environ['ATL_GetHInstanceAt']))
pairs.append((os.environ['ATLINIT'], 'InitializeCriticalSectionEx'))
with open(sys.argv[3], 'w') as f:
    f.write('\t.section .drectve,"yn"\n')
    for a, b in pairs:
        f.write('\t.ascii " /alternatename:%s=%s"\n' % (a, b))
print('crt_aliases: %d aliases: %s' % (len(pairs), ' '.join('%s=%s' % p for p in pairs)))
PY
  "$ROOT/bin/$arch-w64-mingw32-clang" -mno-incremental-linker-compatible -c "$W/aliases-$cpu.s" -o "$OUT/$cpu/crt_aliases.obj"

  # --- libsdk_inlines.a
  # (1) <math.h>: the SDK's `__inline float __CRTDECL f(<float/pointer params>) { return (float)g(<params>); }`
  # forwards that no static library defines, with exactly the SDK's body; any other body shape
  # fails the recipe. -fno-builtin keeps clang from turning (float)fabs(x) back into a call to
  # fabsf (the function being defined).
  python3 - "$W/defined-$cpu.txt" "$XWIN/sdk/include/ucrt/math.h" "$W/float_inlines-$cpu.c" <<'PY'
import re, sys
defined = set(open(sys.argv[1]).read().split())
sdk = open(sys.argv[2]).read()
out, names, parsed = ['#include <math.h>'], [], 0
for m in re.finditer(r'__inline float __CRTDECL (\w+)\(([^)]*)\)\s*\{(.*?)\}', sdk, re.S):
    name, params, body = m.group(1), m.group(2), ' '.join(m.group(3).split())
    parsed += 1
    if name in defined: continue
    plist = [re.sub(r'\b_(In|Out|Inout)_\b', '', p).strip() for p in params.split(',')]
    pnames = [re.fullmatch(r'.*?(\w+)', p).group(1) for p in plist]
    r = re.fullmatch(r'return \(float\)(\w+)\((.*)\);', body)
    if not r or [a.strip() for a in r.group(2).split(',')] != pnames:
        sys.exit('ERROR: SDK float inline %s is not a plain forward: %s' % (name, body))
    if r.group(1) not in defined:
        sys.exit('ERROR: forward target %s of %s is in no static library' % (r.group(1), name))
    out.append('float __cdecl %s(%s) { %s }' % (name, ', '.join(plist), body))
    names.append(name)
if parsed < 20:
    sys.exit('ERROR: only %d SDK float forwards found' % parsed)
open(sys.argv[3], 'w').write('\n'.join(out) + '\n')
print('float inlines: %s' % ' '.join(names))
PY
  cat "$W/float_inlines-$cpu.c"
  "$ROOT/bin/$arch-w64-mingw32-clang" -O2 -fno-builtin $CRTFLAGS $ldflag -mno-incremental-linker-compatible \
    -c "$W/float_inlines-$cpu.c" -o "$W/$cpu-float_inlines.o"
  # Each function defined, and none calls itself.
  for f in $(sed -n 's/^float __cdecl \([a-z0-9_]*\)(.*/\1/p' "$W/float_inlines-$cpu.c"); do
    "$ROOT/bin/llvm-nm" --defined-only "$W/$cpu-float_inlines.o" | grep -q " T $f\$" ||
      { echo "ERROR: $f not defined" >&2; exit 1; }
    ! "$ROOT/bin/llvm-nm" -u "$W/$cpu-float_inlines.o" | grep -q " $f\$" ||
      { echo "ERROR: $f calls itself" >&2; exit 1; }
  done
  # (2) <ws2tcpip.h>: gai_strerrorA/W, `WS2TCPIP_INLINE char *gai_strerrorA(_In_ int ecode) {...}`,
  # copied with exactly the SDK's body when ws2_32.lib (import-only) does not define them.
  "$ROOT/bin/llvm-nm" --defined-only "$XWIN/sdk/lib/um/$cpu/WS2_32.Lib" | awk 'NF>=2{print $NF}' | sort -u > "$W/ws2-defined-$cpu.txt"
  python3 - "$W/ws2-defined-$cpu.txt" "$XWIN/sdk/include/um/WS2tcpip.h" "$W/ws2_inlines-$cpu.c" <<'PY'
import re, sys
defined = set(open(sys.argv[1]).read().split())
sdk = open(sys.argv[2]).read()
out = ['#include <winsock2.h>', '#include <ws2tcpip.h>']
for name, ret in (('gai_strerrorA', 'char'), ('gai_strerrorW', 'WCHAR')):
    m = re.search(r'WS2TCPIP_INLINE\s+%s\s*\*\s*%s\(\s*_In_ int ecode\s*\)\s*\{(.*?)\n\}' % (ret, name), sdk, re.S)
    if not m:
        sys.exit('ERROR: SDK inline %s not found in the expected form' % name)
    if name in defined:
        sys.exit('ERROR: %s is defined by ws2_32.lib; nothing to supply' % name)
    out.append('%s *%s(int ecode)\n{%s\n}' % (ret, name, m.group(1)))
open(sys.argv[3], 'w').write('\n'.join(out) + '\n')
print('ws2tcpip inlines: gai_strerrorA gai_strerrorW')
PY
  cat "$W/ws2_inlines-$cpu.c"
  "$ROOT/bin/$arch-w64-mingw32-clang" -O2 $CRTFLAGS $ldflag -mno-incremental-linker-compatible \
    -c "$W/ws2_inlines-$cpu.c" -o "$W/$cpu-ws2_inlines.o"
  for f in gai_strerrorA gai_strerrorW; do
    "$ROOT/bin/llvm-nm" --defined-only "$W/$cpu-ws2_inlines.o" | grep -q " T $f\$" ||
      { echo "ERROR: $f not defined" >&2; exit 1; }
  done
  "$ROOT/bin/llvm-ar" --format=gnu crsD "$OUT/$cpu/libsdk_inlines.a" "$W/$cpu-float_inlines.o" "$W/$cpu-ws2_inlines.o"

  # --- dinput8.lib (arm only)
  # content/browser names dinput8.lib in its libs; the Windows SDK has an x64 one
  # (sdk/lib/um/x64/dinput8.lib) but none for ARM32 (nothing matching dinput8 in sdk/lib/um/arm).
  # llvm-mingw's ARM32 libdinput8.a is that library: DINPUT8.dll import stubs plus the c_dfDI*
  # DirectInput data formats, the contents of Microsoft's dinput8.lib. It is installed under the
  # name the build links; the recipe fails if the SDK ever ships its own, so it never shadows one.
  if [ "$cpu" = arm ]; then
    if ls "$XWIN/sdk/lib/um/arm" | grep -qi '^dinput8\.lib$'; then
      echo "ERROR: the SDK now ships an ARM32 dinput8.lib; drop this step" >&2; exit 1; fi
    cp "$ROOT/armv7-w64-mingw32/lib/libdinput8.a" "$OUT/arm/dinput8.lib"
    "$ROOT/bin/llvm-nm" --defined-only "$OUT/arm/dinput8.lib" | grep -q ' T __imp_DirectInput8Create$' ||
      { echo "ERROR: dinput8.lib lacks the DINPUT8.dll import of DirectInput8Create" >&2; exit 1; }
    for f in c_dfDIJoystick c_dfDIJoystick2 c_dfDIKeyboard c_dfDIMouse c_dfDIMouse2; do
      "$ROOT/bin/llvm-nm" --defined-only "$OUT/arm/dinput8.lib" | grep -q " R $f\$" ||
        { echo "ERROR: dinput8.lib lacks $f" >&2; exit 1; }
    done
    # The only references the archive does not satisfy itself are the DirectInput GUID_* objects
    # its c_dfDI* members point at (in Microsoft's SDKs they come from dxguid.lib, whose ARM32
    # build has no DirectInput GUIDs); an image links those members only if it uses the data
    # formats, which is also the only way it could need the GUIDs.
    "$ROOT/bin/llvm-nm" --defined-only "$OUT/arm/dinput8.lib" | awk 'NF==3{print $3}' | sort -u > "$W/di-def.txt"
    "$ROOT/bin/llvm-nm" -u "$OUT/arm/dinput8.lib" | awk 'NF==2{print $2}' | sort -u > "$W/di-undef.txt"
    other=$(comm -23 "$W/di-undef.txt" "$W/di-def.txt" | grep -v '^GUID_' | tr '\n' ' ')
    [ -z "$other" ] || { echo "ERROR: dinput8.lib references $other" >&2; exit 1; }
  fi

  # Checks: no CRT function referenced through __imp_ (every __imp_ reference must be a Windows
  # API import, not something Microsoft's static UCRT/vcruntime/libcmt defines); no .ctors.
  "$ROOT/bin/llvm-nm" --defined-only "$XWIN/sdk/lib/ucrt/$cpu/libucrt.lib" "$XWIN/crt/lib/$cpu/libvcruntime.lib" \
    "$XWIN/crt/lib/$cpu/libcmt.lib" > "$W/nm-crt.txt"
  awk 'NF==3{print $3}' "$W/nm-crt.txt" | sort -u > "$W/crt-defined.txt"
  [ "$(wc -l < "$W/crt-defined.txt")" -gt 10000 ] || { echo "ERROR: CRT symbol list implausibly short" >&2; exit 1; }
  for lib in "$OUT/$cpu"/*.a; do
    "$ROOT/bin/llvm-nm" -u "$lib" > "$W/undef.txt"
    awk '{print $NF}' "$W/undef.txt" | sed -n 's/^__imp_//p' | sort -u > "$W/imp.txt"
    bad=$(comm -12 "$W/imp.txt" "$W/crt-defined.txt" | tr '\n' ' ')
    [ -z "$bad" ] || { echo "ERROR: $lib imports CRT functions: $bad" >&2; exit 1; }
    if "$ROOT/bin/llvm-objdump" -h "$lib" | awk '{print $2}' | grep -qx '\.ctors\|\.dtors'; then
      echo "ERROR: $lib has .ctors/.dtors sections" >&2; exit 1; fi
    echo "OK $lib (Windows API imports: $(wc -l < "$W/imp.txt"))"
  done
done
find "$OUT" -type f | sort | xargs sha256sum
