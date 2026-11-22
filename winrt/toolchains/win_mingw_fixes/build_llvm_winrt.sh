#!/bin/sh
# Builds the win-arm32 toolchain: llvm-mingw 20260922 (/opt/llvm-mingw, clang/lld 23.1.2 at
# llvm-project 85ac5602) with clang and lld rebuilt from the SAME llvm-project revision plus
# exactly ten patches, and with Chromium's clang
# plugins (tools/clang/plugins, tools/clang/blink_gc_plugin) compiled in:
#   clang_mingw_cl_arg_translation.patch  clang-cl command lines for a *-windows-gnu triple get
#                                         /O, -D name#value and /permissive[-] desugared, and
#                                         an explicit /std:c++NN honored, like the MSVC
#                                         toolchain does (unpatched: silently ignored, so /O2
#                                         compiles at -O0 and /std:c++20 has no effect).
#   llvm_mingw_crt_init_array.patch       a *-windows-gnu object compiled with -fuse-init-array
#                                         puts its static constructors in the Microsoft CRT's
#                                         .CRT$XCU table (clang's MinGW driver honors the flag;
#                                         the COFF backend uses UseInitArray for -gnu), so
#                                         images using Microsoft's static CRT run them
#                                         (unpatched: always .ctors, which only MinGW's CRT runs),
#                                         and x86 main no longer calls MinGW's __main (the .ctors
#                                         runner) for such objects.
#   lld_arm_delayload_tailmerge.patch     lld's ARM delay-load tail-merge reaches
#                                         __delayLoadHelper2 with movw/movt+blx instead of a
#                                         +-16MB bl (chrome.dll is larger).
#   llvm_arm_windows_stack_guard_pic.patch the Thumb-2 stack-protector guard load on Windows
#                                         uses absolute movw/movt like every other global
#                                         access there (unpatched, under the PIC relocation
#                                         model lld uses for LTO codegen on ARM it emits a
#                                         pc-relative movw/movt that COFF cannot represent:
#                                         every ThinLTO module with a /GS-protected function
#                                         fails "Cannot represent this expression").
#   lld_coff_gfids_relocations.patch      lld-link puts the functions a /guard:cf object lists
#                                         in .gfids$x (ADDR32NB relocations, as the Windows SDK's
#                                         x64 UCRT objects do) into the CFG function table, as it
#                                         does for .gfids$y symbol indices (unpatched: the UCRT's
#                                         initializers are left out and every x64 image
#                                         fast-fails at startup, FAST_FAIL_GUARD_ICALL_CHECK_FAILURE).
#   llvm_lib_thumb_bitcode_machine.patch  lld-link /lib (llvm-lib) gives bitcode with a thumb
#                                         triple -- all Windows ARM32 LTO bitcode, the triple is
#                                         always thumbv7 -- the ARMNT machine type, as lld-link's
#                                         own linker does (unpatched: "unknown arch in target
#                                         triple: thumbv7-w64-windows-gnu" for every ThinLTO
#                                         static library).
#   llvm_arm_codeview_ignored_regs.patch  ARM's CodeView writer skips a variable location in a
#                                         register CodeView has no number for (a NEON tuple such
#                                         as d25_d26, a GPR pair), as AArch64's does for SVE
#                                         registers (unpatched: "LLVM ERROR: unknown codeview
#                                         register D25_D26" -- libaom's intrapred_neon.c with
#                                         -gcodeview and -ftrivial-auto-var-init=pattern, i.e.
#                                         chrome.dll).
#   lld_arm_forwarder_rva.patch           lld-link writes an ARM32 forwarded export's RVA (the
#                                         address of its "dll.name" string) without the Thumb bit
#                                         it ORs into code exports (unpatched: "KERNEL32.CreateFileA"
#                                         is exported as "ERNEL32.CreateFileA").
#   llvm_arm_coff_feat00.patch            ARM's AsmPrinter emits the COFF @feat.00 symbol (with the
#                                         GuardCF bit under /guard:cf), as AArch64 and X86 do, so
#                                         lld-link builds the CFG tables from each object's
#                                         .gfids$y/.gljmp$y (unpatched: no ARM32 object is
#                                         CFG-aware; lld admits every function-typed relocation
#                                         target instead -- 683249 entries in chrome.dll, no
#                                         longjmp table -- and leaves out untyped assembler
#                                         functions that C calls through pointers, e.g. FFmpeg's
#                                         ff_prefetch_arm: renderer fast-fail 0xC0000409/0xa).
#   clang_gnu_mscompat_exceptions_macro.patch  clang defines __EXCEPTIONS for C++ exceptions under
#                                         MSVC compatibility when GNU compatibility is declared
#                                         (-fgnuc-version: clang-cl for *-windows-gnu), as it does
#                                         __GXX_RTTI (unpatched: neither __EXCEPTIONS nor the MSVC
#                                         environment's _CPPUNWIND is defined although /EHsc
#                                         enables exceptions; node-addon-api's napi.h fails
#                                         "Exception support not detected").
# Only bin/clang-23 (clang, clang++, clang-cl) and bin/lld (ld.lld, lld-link) are replaced;
# everything else (headers, CRT, libraries, driver config files, and llvm-mingw's other tools
# such as llvm-ar/llvm-objcopy, which the build also uses, or clangd/clang-tidy, which it does
# not) is llvm-mingw's, copied unchanged -- those other tools are built from the same
# revision without the patches, none of which touches them. Output:
# /home/winrt/sdk/llvm-mingw-winrt (clang_base_path of the build). The replaced binaries
# report the pinned revision (LLVM_FORCE_VC_REVISION), and every name the build invokes is
# checked to.
#
# Runs on a linux build host (aarch64 or x86_64). The host binaries are compiled with
# llvm-mingw's own clang for the host's *-linux-gnu triple against the DEPS-pinned
# Chromium debian_bullseye sysroot of the host's arch (arm64 or amd64; installed in
# /home/winrt/sdk/sysroots), with libstdc++,
# zlib and libxml2 linked statically. libxml2 (lld-link merges the images' manifests with it
# instead of calling mt.exe) is built exactly as Chromium's own clang build does it
# (tools/clang/scripts/build.py BuildLibXml2: libxml2-v2.9.12 from the chromium-browser-clang
# bucket, only TREE+OUTPUT+THREADS enabled). Its only user here is lld (manifest merging);
# CLANG_ENABLE_LIBXML2=OFF keeps libclang's optional libxml2 use (c-index-test's RelaxNG comment
# schema validation, which this minimal libxml2 does not provide) out; the clang binary itself
# does not use libxml2.
set -eu
PIN=85ac560262434c9ccfc0c183ec22d4138ed647fb
HERE=$(cd "$(dirname "$0")" && pwd)
SDK=/home/winrt/sdk
LLVM_MINGW=/opt/llvm-mingw
case $(uname -m) in
  aarch64) HOST_ARCH=aarch64 HOST_SYSROOT=arm64 ;;
  x86_64) HOST_ARCH=x86_64 HOST_SYSROOT=amd64 ;;
  *) echo "unsupported build host $(uname -m)" >&2; exit 1 ;;
esac
SYSROOT=$SDK/sysroots/debian_bullseye_$HOST_SYSROOT-sysroot
# Chromium's clang plugins (find-bad-constructs, blink-gc-plugin) are compiled into clang from
# this Chromium tree's tools/clang, as tools/clang/scripts/build.py does for Chromium's own
# clang package (LLVM_EXTERNAL_PROJECTS=chrometools); build/config/clang/BUILD.gn and
# third_party/blink/renderer/BUILD.gn then enable them with -Xclang -add-plugin (no -load).
CHROMIUM_SRC=/home/winrt/chromium
SRC=${SRC:-$SDK/build/llvm-winrt-src}
BUILD=${BUILD:-$SDK/build/llvm-winrt}
OUT=${OUT:-$SDK/llvm-mingw-winrt}
# One run at a time: a second run would delete this run's $SRC/$BUILD and staged toolchain.
exec 9>"$SDK/llvm-winrt.lock"
flock -n 9 || { echo "another build_llvm_winrt.sh run holds $SDK/llvm-winrt.lock" >&2; exit 1; }
# The toolchain is assembled and fully tested in $STAGE; $OUT is replaced only after every test
# below passed, so a failed run leaves the previously installed toolchain in place.
STAGE=$OUT.new
CMAKE_VERSION=3.31.6

# The pinned clang really is llvm-mingw's.
"$LLVM_MINGW/bin/clang" --version | grep -q "$PIN" || { echo "llvm-mingw clang is not $PIN" >&2; exit 1; }

# cmake (not part of the host image): the PyPI wheel, pinned by version and sha256.
printf 'cmake==%s --hash=sha256:%s --hash=sha256:%s\n' "$CMAKE_VERSION" \
  42d9883b8958da285d53d5f69d40d9650c2d1bcf922d82b3ebdceb2b3a7d4521 \
  1c8b05df0602365da91ee6a3336fe57525b137706c4ab5675498f662ae1dbcec > "$SDK/tools/cmake-requirements.txt"
python3 -m pip install --quiet --require-hashes --upgrade --target "$SDK/tools/cmake" \
  -r "$SDK/tools/cmake-requirements.txt"
CMAKE="python3 -c 'import sys; sys.path.insert(0, \"$SDK/tools/cmake\"); import cmake; sys.exit(cmake.cmake())'"

# Fresh source at the pin + the ten patches (no other modification can leak in).
rm -rf "$SRC"; mkdir -p "$SRC"
git -C "$SDK/src/llvm-project" rev-parse HEAD | grep -qx "$PIN"
git -C "$SDK/src/llvm-project" archive "$PIN" cmake llvm clang lld libc libunwind third-party | tar -x -C "$SRC"
for p in clang_mingw_cl_arg_translation llvm_mingw_crt_init_array lld_arm_delayload_tailmerge \
         llvm_arm_windows_stack_guard_pic lld_coff_gfids_relocations \
         llvm_lib_thumb_bitcode_machine llvm_arm_codeview_ignored_regs lld_arm_forwarder_rva \
         llvm_arm_coff_feat00 clang_gnu_mscompat_exceptions_macro; do
  patch -d "$SRC" -p1 --no-backup-if-mismatch < "$HERE/$p.patch"
done

HOSTFLAGS="--target=$HOST_ARCH-linux-gnu --sysroot=$SYSROOT"
[ -n "${KEEP_BUILD:-}" ] || rm -rf "$BUILD"; mkdir -p "$BUILD"

# libxml2, as tools/clang/scripts/build.py BuildLibXml2 builds it.
XML=$BUILD/libxml2
mkdir -p "$XML"
curl -sfL https://commondatastorage.googleapis.com/chromium-browser-clang/tools/libxml2-v2.9.12.tar.gz \
  -o "$XML/libxml2.tar.gz"
echo "98bfa7a9a5e2a75638422050740448ee9f02bf4dc2075c9822d7747d5ff9e617  $XML/libxml2.tar.gz" | sha256sum -c -
tar -xzf "$XML/libxml2.tar.gz" -C "$XML"
eval $CMAKE -G Ninja -S "$XML/libxml2-v2.9.12" -B "$XML/build" \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$XML/install" -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_C_COMPILER="$LLVM_MINGW/bin/clang" -DCMAKE_C_FLAGS="'$HOSTFLAGS'" \
  -DCMAKE_EXE_LINKER_FLAGS="'$HOSTFLAGS -fuse-ld=lld'" -DCMAKE_SYSROOT="$SYSROOT" \
  -DLIBXML2_WITH_C14N=OFF -DLIBXML2_WITH_CATALOG=OFF -DLIBXML2_WITH_DEBUG=OFF \
  -DLIBXML2_WITH_DOCB=OFF -DLIBXML2_WITH_FTP=OFF -DLIBXML2_WITH_HTML=OFF -DLIBXML2_WITH_HTTP=OFF \
  -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_ICU=OFF -DLIBXML2_WITH_ISO8859X=OFF \
  -DLIBXML2_WITH_LEGACY=OFF -DLIBXML2_WITH_LZMA=OFF -DLIBXML2_WITH_MEM_DEBUG=OFF \
  -DLIBXML2_WITH_MODULES=OFF -DLIBXML2_WITH_OUTPUT=ON -DLIBXML2_WITH_PATTERN=OFF \
  -DLIBXML2_WITH_PROGRAMS=OFF -DLIBXML2_WITH_PUSH=OFF -DLIBXML2_WITH_PYTHON=OFF \
  -DLIBXML2_WITH_READER=OFF -DLIBXML2_WITH_REGEXPS=OFF -DLIBXML2_WITH_RUN_DEBUG=OFF \
  -DLIBXML2_WITH_SAX1=OFF -DLIBXML2_WITH_SCHEMAS=OFF -DLIBXML2_WITH_SCHEMATRON=OFF \
  -DLIBXML2_WITH_TESTS=OFF -DLIBXML2_WITH_THREADS=ON -DLIBXML2_WITH_THREAD_ALLOC=OFF \
  -DLIBXML2_WITH_TREE=ON -DLIBXML2_WITH_VALID=OFF -DLIBXML2_WITH_WRITER=OFF \
  -DLIBXML2_WITH_XINCLUDE=OFF -DLIBXML2_WITH_XPATH=OFF -DLIBXML2_WITH_XPTR=OFF \
  -DLIBXML2_WITH_ZLIB=OFF
ninja -C "$XML/build" install

eval $CMAKE -G Ninja -S "$SRC/llvm" -B "$BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_ENABLE_PROJECTS="'clang;lld'" \
  -DLLVM_FORCE_VC_REVISION=$PIN -DLLVM_FORCE_VC_REPOSITORY=https://github.com/llvm/llvm-project.git \
  -DLLVM_TARGETS_TO_BUILD="'ARM;AArch64;X86;NVPTX'" \
  -DLLVM_ENABLE_BINDINGS=OFF \
  -DLLVM_INCLUDE_TESTS=ON -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DCMAKE_C_COMPILER="$LLVM_MINGW/bin/clang" \
  -DCMAKE_CXX_COMPILER="$LLVM_MINGW/bin/clang++" \
  -DCMAKE_ASM_COMPILER="$LLVM_MINGW/bin/clang" \
  -DCMAKE_C_FLAGS="'$HOSTFLAGS'" -DCMAKE_CXX_FLAGS="'$HOSTFLAGS'" -DCMAKE_ASM_FLAGS="'$HOSTFLAGS'" \
  -DCMAKE_EXE_LINKER_FLAGS="'$HOSTFLAGS -fuse-ld=lld -static-libstdc++ -static-libgcc'" \
  -DCMAKE_SHARED_LINKER_FLAGS="'$HOSTFLAGS -fuse-ld=lld'" \
  -DCMAKE_MODULE_LINKER_FLAGS="'$HOSTFLAGS -fuse-ld=lld'" \
  -DCMAKE_SYSROOT="$SYSROOT" \
  -DCMAKE_FIND_ROOT_PATH="$SYSROOT" \
  -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
  -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
  -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
  -DLLVM_ENABLE_LIBXML2=FORCE_ON \
  -DCLANG_ENABLE_LIBXML2=OFF \
  -DLIBXML2_LIBRARY="$XML/install/lib/libxml2.a" -DLIBXML2_LIBRARIES="$XML/install/lib/libxml2.a" \
  -DLIBXML2_INCLUDE_DIR="$XML/install/include/libxml2" \
  -DLLVM_ENABLE_ZLIB=FORCE_ON \
  -DZLIB_LIBRARY="$SYSROOT/usr/lib/$HOST_ARCH-linux-gnu/libz.a" \
  -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_TERMINFO=OFF -DLLVM_ENABLE_LIBEDIT=OFF \
  -DLLVM_HOST_TRIPLE=$HOST_ARCH-unknown-linux-gnu \
  -DLLVM_TABLEGEN_FLAGS= \
  -DLLVM_TOOL_LLVM_DRIVER_BUILD=OFF \
  -DLLVM_EXTERNAL_PROJECTS=chrometools \
  -DLLVM_EXTERNAL_CHROMETOOLS_SOURCE_DIR="$CHROMIUM_SRC/tools/clang" \
  -DCHROMIUM_TOOLS="'plugins;blink_gc_plugin'"
ninja -C "$BUILD" clang lld

# Plugin checks, on the freshly built clang before anything is installed: a failure here stops
# the recipe with the previously installed toolchain untouched.
T=$BUILD/selftest; rm -rf "$T"; mkdir -p "$T"
# (4) the Chromium plugins are in clang and run, invoked with the flags the build passes
#     (build/config/clang/BUILD.gn find_bad_constructs; third_party/blink/renderer/BUILD.gn):
#     a class in a header with four non-trivial members and no declared constructor must get
#     find-bad-constructs' "[chromium-style]" error; blink-gc-plugin must be registered.
printf 'struct NonTrivial {\n  NonTrivial();\n  ~NonTrivial();\n};\nclass Foo {\n public:\n  NonTrivial a, b, c, d;\n};\n' > "$T/style.h"
printf '#include "style.h"\nFoo* make() { return new Foo; }\n' > "$T/style.cc"
if "$BUILD/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /Fo"$T/style.obj" \
     -Xclang -add-plugin -Xclang find-bad-constructs \
     -Xclang -plugin-arg-find-bad-constructs -Xclang raw-ref-template-as-trivial-member \
     "$T/style.cc" > "$T/style.log" 2>&1; then
  cat "$T/style.log"; echo "SELFTEST FAIL: find-bad-constructs reported nothing" >&2; exit 1
fi
grep -q "\[chromium-style\]" "$T/style.log" || { cat "$T/style.log"; echo "SELFTEST FAIL: no [chromium-style] diagnostic" >&2; exit 1; }
grep "\[chromium-style\]" "$T/style.log"
printf 'int x;\n' > "$T/gc.cc"
"$BUILD/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /Fo"$T/gc.obj" \
  -Xclang -add-plugin -Xclang blink-gc-plugin \
  -Xclang -plugin-arg-blink-gc-plugin -Xclang fix-bugs-of-is-considered-abstract "$T/gc.cc" ||
  { echo "SELFTEST FAIL: blink-gc-plugin not registered" >&2; exit 1; }
# (5) the plugins' own regression suites on the built clang, driven by their
#     own harness classes (pylib/clang/plugin_testing.py; plugins/tests/test.py for
#     find-bad-constructs incl. layout-object-methods and blink-data-member-type;
#     blink_gc_plugin/tests/test.py's BlinkGcPluginTest, once without and once with
#     USE_V8_OILPAN, each in its own copy so neither run's .actual files can overwrite the
#     other's). The harness writes <test>.txt.actual next to each test whose output differs from
#     its golden. The goldens are clang 16's output and clang 23 renders some diagnostics
#     differently, so each differing test is compared as follows (anything else fails the recipe,
#     the installed toolchain is left untouched, and every differing test is printed as a diff):
#       - a golden without compiler diagnostics (the blink-gc object-graph tests, whose expected
#         output is process-graph.py's) must match exactly;
#       - otherwise every diagnostic line ("<file>:<line>:<col>: <severity>: <message>" with or
#         without a plugin tag, "In file included from", "N warnings generated.") must be
#         identical in number, order and text, except a leading namespace qualifier inside
#         "aka '...'" (clang 23 prints 'base::X' where clang 16 printed 'X'); every other line of the output
#         must be a ^~~~ underline, a verbatim line of the test's own sources -- <test>.cpp/.h
#         and the files they #include from the tests directory (the snippet,
#         whose line breaking and underline width clang 23 renders differently), or a line the
#         golden also has (fix-it hints), compared without surrounding whitespace;
#       - crash output, a missing golden, or a harness failure other than an output mismatch.
#     The suites run with -Wno-unused-command-line-argument and -fno-diagnostics-show-line-numbers
#     (a one-line wrapper around the staged clang): clang 23's driver otherwise adds a line for the
#     harness's -c ignored under -fsyntax-only, and a line-number gutter to every snippet.
# Each suite runs from a copy laid out like the source tree (<copy>/tools/clang/...): some checks
# select files by path (CheckLayoutObjectMethodsVisitor only examines files under
# "tools/clang/plugins/tests" or the Blink layout directory), so a differently named copy would
# silently skip them.
CT=$T/ct; rm -rf "$CT"
for d in plugins blink blink_oilpan; do
  mkdir -p "$CT/$d/tools/clang"
  cp -a "$CHROMIUM_SRC/tools/clang/pylib" "$CT/$d/tools/clang/"
done
mkdir -p "$CT/plugins/tools/clang/plugins"
cp -a "$CHROMIUM_SRC/tools/clang/plugins/tests" "$CT/plugins/tools/clang/plugins/"
for d in blink blink_oilpan; do
  mkdir -p "$CT/$d/tools/clang/blink_gc_plugin"
  cp -a "$CHROMIUM_SRC/tools/clang/blink_gc_plugin/tests" "$CHROMIUM_SRC/tools/clang/blink_gc_plugin/process-graph.py" "$CT/$d/tools/clang/blink_gc_plugin/"
done
find "$CT" -name '*.actual' -delete
printf '#!/bin/sh\nexec "%s" "$@" -Wno-unused-command-line-argument -fno-diagnostics-show-line-numbers\n' "$BUILD/bin/clang" > "$T/clang-suite"
chmod +x "$T/clang-suite"
echo "### find-bad-constructs suite"
python3 "$CT/plugins/tools/clang/plugins/tests/test.py" "$T/clang-suite" > "$CT/plugins.log" 2>&1 || true
for d in blink blink_oilpan; do
  echo "### blink-gc-plugin suite ($d)"
  oilpan=False; [ $d = blink_oilpan ] && oilpan=True
  ( cd "$CT/$d/tools/clang/blink_gc_plugin/tests" && python3 -c "import importlib.util, os, sys; spec = importlib.util.spec_from_file_location('gc_test', os.path.abspath('test.py')); m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m); m.BlinkGcPluginTest($oilpan, os.getcwd(), sys.argv[1], 'blink-gc-plugin', False).Run()" "$T/clang-suite" ) > "$CT/$d.log" 2>&1 || true
done
grep -hE "^Ran " "$CT/plugins.log" "$CT/blink.log" "$CT/blink_oilpan.log"
[ "$(cat "$CT/plugins.log" "$CT/blink.log" "$CT/blink_oilpan.log" | grep -c '^Ran ')" = 3 ] ||
  { tail -n 20 "$CT"/*.log; echo "SELFTEST FAIL: a plugin suite did not run" >&2; exit 1; }
python3 - "$CT" <<'PY'
import difflib, glob, os, re, sys
ct = sys.argv[1]
DIAG = re.compile(r"^(In file included from |\S+:\d+:\d+: (warning|error|note|fatal error): |\d+ (warning|error)s? generated\.$)")
UNDERLINE = re.compile(r"^[ \t]*\^[~ ]*$|^[ \t]*~[~ ^]*$")
CRASH = re.compile(r"PLEASE submit|Stack dump|LLVM ERROR|Segmentation fault")
AKA = re.compile(r"aka '([^']*)'")
def norm(line):
    return re.sub(r"aka '(?:[A-Za-z_]\w*::)+", "aka '", line)
bad, relaxed = [], 0
for suite in ('plugins', 'blink', 'blink_oilpan'):
    current = None
    for line in open(os.path.join(ct, suite + '.log')):
        m = re.match(r"Testing (\S+)\.\.\. ", line)
        if m:
            current = m.group(1)
        if line.startswith('failed: ') and not line.startswith('failed: expected and actual differed'):
            bad.append('%s/%s: %s' % (suite, current, line.strip()))
for a in sorted(glob.glob(os.path.join(ct, '**', '*.actual'), recursive=True)):
    g, name = a[:-len('.actual')], os.path.relpath(a, ct)
    actual = open(a).read()
    golden = open(g).read() if os.path.exists(g) else None
    sys.stdout.writelines(difflib.unified_diff((golden or '').splitlines(True), actual.splitlines(True), g, a, n=0))
    if golden is None:
        bad.append('%s: no golden' % name); continue
    if CRASH.search(actual):
        bad.append('%s: crash output' % name); continue
    gl, al = golden.splitlines(), actual.splitlines()
    if not any(DIAG.match(l) for l in gl):
        bad.append('%s: output without compiler diagnostics differs' % name); continue
    if [norm(l) for l in gl if DIAG.match(l)] != [norm(l) for l in al if DIAG.match(l)]:
        bad.append('%s: diagnostic lines differ' % name); continue
    tests_dir = os.path.dirname(a)
    base = os.path.basename(g)
    base = base[:-len('.cppgc.txt')] if base.endswith('.cppgc.txt') else base[:-len('.txt')]
    files = [f for f in glob.glob(os.path.join(tests_dir, base + '.*')) if f.endswith(('.cpp', '.h'))]
    for f in list(files):
        for inc in re.findall(r'^#include "([^"]+)"', open(f, errors='replace').read(), re.M):
            if os.path.exists(os.path.join(tests_dir, inc)):
                files.append(os.path.join(tests_dir, inc))
    source = set()
    for f in files:
        source.update(l.rstrip() for l in open(f, errors='replace'))
    golden_other = set(l.strip() for l in gl if not DIAG.match(l))
    stray = [l for l in al if not DIAG.match(l) and not UNDERLINE.match(l) and l.rstrip() not in source
             and l.strip() not in golden_other and l.strip()]
    if stray:
        bad.append('%s: lines that are neither diagnostics, underlines nor source: %r' % (name, stray[:3])); continue
    relaxed += 1
print('plugin suites: %d tests differ from their clang-16 goldens only in snippet/underline rendering or aka qualifiers, %d otherwise' % (relaxed, len(bad)))
if bad:
    print('\n'.join(bad)); sys.exit('SELFTEST FAIL: plugin regression suites')
PY

# Assemble (in $STAGE): llvm-mingw unchanged, with the two rebuilt driver binaries swapped in. The build
# drives clang-cl (the CL-mode name of clang-23) and lld-link (a name of lld); the recipe sets
# both names itself rather than relying on what the input tree carries.
rm -rf "$STAGE"
cp -a "$LLVM_MINGW" "$STAGE"
cp "$BUILD/bin/clang-23" "$STAGE/bin/clang-23"
cp "$BUILD/bin/lld" "$STAGE/bin/lld"
ln -sfn clang-23 "$STAGE/bin/clang-cl"
ln -sfn lld "$STAGE/bin/lld-link"
for b in clang clang++ clang-cl ld.lld lld-link; do
  "$STAGE/bin/$b" --version | grep -q "$PIN" ||
    { echo "$STAGE/bin/$b does not report llvm-project $PIN" >&2; exit 1; }
done
"$STAGE/bin/clang" --version
"$STAGE/bin/ld.lld" --version

# Upstream regression tests touching the patched code: lld's COFF suite (includes
# delayimports-armnt.yaml as updated by the tail-merge patch, gfids-x.s, the .gfids$x test, and
# export-armnt.yaml as extended by the forwarder patch) and clang's whole driver suite (clang/test/Driver: the clang
# patch's mingw-cl-translate-args.c, the clang-cl option tests, every mingw-*.c test).
ninja -C "$BUILD" check-lld-coff
ninja -C "$BUILD" check-clang-driver
# The predefined-macro patch: clang's preprocessor predefine tests, including its own
# predefined-exceptions-ms-compat.cpp.
"$BUILD/bin/llvm-lit" -sv "$SRC/clang/test/Preprocessor"
# The backend patches: the COFF structor tests the init-array patch extends, the Windows ARM
# codegen tests (Windows/pic.ll gains the stack-guard case), and every x86 codegen test for a
# MinGW or Cygwin triple (the __main change; its own test is X86/mingw-main-init-array.ll).
ninja -C "$BUILD" llc llvm-mc llvm-objdump FileCheck not count llvm-as llvm-lib
X86_MINGW_TESTS=$(cd "$SRC/llvm/test/CodeGen/X86" &&
  grep -l "mingw\|cygwin\|windows-gnu" *.ll *.mir | sed "s#^#$SRC/llvm/test/CodeGen/X86/#")
"$BUILD/bin/llvm-lit" -sv $X86_MINGW_TESTS \
  "$SRC/llvm/test/CodeGen/ARM/Windows" "$SRC/llvm/test/CodeGen/ARM/cfguard-module-flag.ll" \
  "$SRC/llvm/test/CodeGen/ARM/cfguard-checks.ll" "$SRC/llvm/test/MC/COFF" \
  "$SRC/llvm/test/tools/llvm-lib" "$SRC/llvm/test/DebugInfo/COFF/ARMNT"

# Self-tests of the changes against the assembled toolchain.
mkdir -p "$T"
# (1) clang-cl /O2 for a -gnu triple optimizes (stock: ignored with an "argument unused" warning,
#     an error under /WX, and the same code as /Od), invoked the way the build invokes it.
printf 'int f(int x){int s=0;for(int i=0;i<x;i++)s+=i*i;return s;}\n' > "$T/o.c"
for o in O2 Od; do
  "$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /$o /Fo"$T/$o.obj" "$T/o.c"
done
n2=$("$STAGE/bin/llvm-objdump" -d "$T/O2.obj" | grep -cE '^ +[0-9a-f]+:')
nd=$("$STAGE/bin/llvm-objdump" -d "$T/Od.obj" | grep -cE '^ +[0-9a-f]+:')
[ "$n2" -lt "$nd" ] || { echo "SELFTEST FAIL: /O2 ($n2 insns) not smaller than /Od ($nd)" >&2; exit 1; }
# (2) delay-load tail-merge more than 16MB from __delayLoadHelper2 (lld_delayload_test/: a 20MB
#     pad between them; unpatched lld: "relocation out of range").
CC="$STAGE/bin/armv7-w64-mingw32-clang"
"$STAGE/bin/llvm-ar" x --output "$T" "$STAGE/armv7-w64-mingw32/lib/libmingwex.a" libarm32_libmingwex_a-delayimp.o
"$CC" -O1 -c "$HERE/lld_delayload_test/main.c" -o "$T/main.o"
"$CC" -c "$HERE/lld_delayload_test/pad.s" -o "$T/pad.o"
"$CC" "$T/libarm32_libmingwex_a-delayimp.o" "$T/pad.o" "$T/main.o" \
      -Wl,--delayload=shlwapi.dll -lshlwapi -o "$T/dl_test.exe"
"$STAGE/bin/llvm-objdump" -d --no-show-raw-insn "$T/dl_test.exe" > "$T/dl_test.dis"
[ "$(grep -c "mov	r1, r12" "$T/dl_test.dis")" -eq 1 ] || { echo "SELFTEST FAIL: expected one tail-merge chunk" >&2; exit 1; }
grep -A10 "mov	r1, r12" "$T/dl_test.dis" | grep -B4 -A6 "movw	r0" > "$T/tailmerge.dis"
cat "$T/tailmerge.dis"
HELPER=$(awk '/<__delayLoadHelper2>:/{print $1; exit}' "$T/dl_test.dis")
LO=$(awk '/movw	r12, #/{sub(/.*#/,""); print; exit}' "$T/tailmerge.dis")
HI=$(awk '/movt	r12, #/{sub(/.*#/,""); print; exit}' "$T/tailmerge.dis")
grep -q "blx	r12" "$T/tailmerge.dis" || { echo "SELFTEST FAIL: no blx r12 in tail-merge" >&2; exit 1; }
[ $(( (HI << 16) | LO )) -eq $(( 0x$HELPER | 1 )) ] || { echo "SELFTEST FAIL: tail-merge target != __delayLoadHelper2|1" >&2; exit 1; }
IB=$("$STAGE/bin/llvm-readobj" --file-headers "$T/dl_test.exe" | awk '/ImageBase:/{print $2; exit}')
"$STAGE/bin/llvm-readobj" --coff-basereloc "$T/dl_test.exe" | awk '/Type:/{t=$2} /Address:/{print t, $2}' > "$T/basereloc.txt"
RELOCS=0
for site in $(awk '/movw	(r0|r12), #/{sub(":","",$1); print $1}' "$T/tailmerge.dis"); do
  rva=$(printf '0x%X' $(( 0x$site - IB )))
  grep -qix "ARM_MOV32(T) $rva" "$T/basereloc.txt" || { echo "SELFTEST FAIL: no ARM_MOV32T base relocation at $rva" >&2; exit 1; }
  RELOCS=$((RELOCS + 1))
done
[ "$RELOCS" -eq 2 ] || { echo "SELFTEST FAIL: expected 2 movw sites, found $RELOCS" >&2; exit 1; }
# (3) a /GS-protected function through ThinLTO: lld-link codegens it with the PIC relocation
#     model (lld/COFF/LTO.cpp createConfig); unpatched: "Cannot represent this expression" and the
#     module's object is dropped ("undefined symbol: main").
printf 'void use(char *);\nint main(void){char b[64];use(b);return b[0];}\n' > "$T/sg.c"
printf 'unsigned __stack_chk_guard = 0x2b992ddf;\nvoid __stack_chk_fail(void){for(;;);}\nvoid use(char *p){p[0]=0;}\n' > "$T/guard.c"
"$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /O2 /GS "$T/sg.c" /Fo"$T/sg_native.obj"
"$STAGE/bin/llvm-nm" -u "$T/sg_native.obj" | grep -q "__stack_chk_guard" || { echo "SELFTEST FAIL: /GS emitted no stack protector" >&2; exit 1; }
"$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /O2 /GS -flto=thin "$T/sg.c" /Fo"$T/sg.obj"
"$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /O2 /GS- "$T/guard.c" /Fo"$T/guard.obj"
"$STAGE/bin/lld-link" /lldmingw /entry:main /subsystem:console /nodefaultlib /out:"$T/sg.exe" "$T/sg.obj" "$T/guard.obj" \
  > "$T/sg.log" 2>&1 || { cat "$T/sg.log"; echo "SELFTEST FAIL: ThinLTO /GS link" >&2; exit 1; }
# (7) the installed lld-link /lib accepts ThinLTO bitcode (thumbv7 triple) as the build archives
#     it: a thin archive of a ThinLTO object and a native object, both ARMNT.
"$STAGE/bin/lld-link" /lib /nologo /WX /llvmlibthin /out:"$T/thin.lib" "$T/sg.obj" "$T/guard.obj" \
  > "$T/lib.log" 2>&1 || { cat "$T/lib.log"; echo "SELFTEST FAIL: lld-link /lib on thumbv7 bitcode" >&2; exit 1; }
# (8) CodeView for a variable held in a NEON register tuple: libaom's intrapred_neon.c (the
#     chrome.dll module that failed) compiled as the build compiles it -- -gcodeview (what emits
#     locals on this -gnu triple) and -ftrivial-auto-var-init=pattern (the product flag that
#     reaches the d25_d26 location in av1_dr_prediction_z3_neon). Unpatched: "unknown codeview
#     register D25_D26". Patched, the check is that only unencodable locations are dropped:
#     inside that function's proc record (S_GPROC32 ... S_PROC_ID_END) the register locations
#     that CodeView can encode must still be there -- at least 100 register def-ranges, among them
#     q-, d- and r-register ones (a predicate that ignored every register would leave none; the
#     same function compiled without the trigger flag has ~1400, on ARM_NQ*, ARM_ND*, ARM_R*,
#     ARM_SP, ARM_LR).
"$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config -mfpu=neon /c /O2 /Z7 -gcodeview \
  -ftrivial-auto-var-init=pattern -I"$CHROMIUM_SRC/third_party/libaom/source/config" \
  -I"$CHROMIUM_SRC/third_party/libaom/source/config/win/arm-neon" -I"$CHROMIUM_SRC/third_party/libaom/source/libaom" \
  "$CHROMIUM_SRC/third_party/libaom/source/libaom/aom_dsp/arm/intrapred_neon.c" /Fo"$T/intrapred_neon.obj" \
  > "$T/cv.log" 2>&1 || { cat "$T/cv.log"; echo "SELFTEST FAIL: CodeView for a NEON register tuple" >&2; exit 1; }
"$STAGE/bin/llvm-readobj" --codeview "$T/intrapred_neon.obj" |
  awk '/DisplayName: av1_dr_prediction_z3_neon$/{p=1} p&&/Kind: S_PROC_ID_END/{exit} p' > "$T/cv_z3.txt"
CVR=$(grep -c 'DefRangeRegisterSym\|DefRangeSubfieldRegisterSym' "$T/cv_z3.txt" || true)
[ "$CVR" -ge 100 ] && grep -q 'Register: ARM_NQ[0-9]' "$T/cv_z3.txt" && grep -q 'Register: ARM_ND[0-9]' "$T/cv_z3.txt" &&
  grep -q 'Register: ARM_R[0-9]' "$T/cv_z3.txt" ||
  { echo "SELFTEST FAIL: av1_dr_prediction_z3_neon keeps $CVR register def-ranges (need >= 100 incl. q/d/r)" >&2; exit 1; }
# (9) Control Flow Guard tables on ARM32 (llvm_arm_coff_feat00.patch), linked the way the product
#     links (mingw driver, -mguard=cf / --guard-cf), once with native codegen and once through
#     ThinLTO (lld-link's own codegen, which is how chrome.dll's C/C++ is compiled): an assembler
#     function without a COFF function type (asm_fn, as FFmpeg's/dav1d's `function` macros
#     define them) whose address C takes, and a C function only ever called directly (direct_fn).
#     The C object must carry @feat.00 with GuardCF (0x800); the image's GuardFidTable must hold
#     asm_fn (named by the C object's .gfids$y; unpatched: missing -> indirect call fast-fails)
#     and must NOT hold direct_fn (unpatched: every function-typed relocation target is admitted).
printf '.text\n.syntax unified\n.thumb\n.globl asm_fn\n.p2align 2\n.thumb_func\nasm_fn:\n  bx lr\n' > "$T/cfg_asm.s"
printf '__attribute__((noinline)) int direct_fn(int x) { return x * 3 + 1; }\nvoid asm_fn(void);\nvoid (*volatile fp)(void) = asm_fn;\nint main(int argc, char **argv) { fp(); return direct_fn(argc) - 4; }\n' > "$T/cfg_c.c"
"$CC" -c "$T/cfg_asm.s" -o "$T/cfg_asm.o"
for mode in native thinlto; do
  lto=; [ $mode = thinlto ] && lto=-flto=thin
  "$CC" -O1 -mguard=cf $lto -c "$T/cfg_c.c" -o "$T/cfg_c_$mode.o"
  if [ $mode = native ]; then
    FEAT=$("$STAGE/bin/llvm-objdump" -t "$T/cfg_c_native.o" | awk '/@feat.00/{sub(/^0x/, "", $(NF-1)); print $(NF-1); exit}')
    [ $(( 0x${FEAT:-0} & 0x800 )) -ne 0 ] || { echo "SELFTEST FAIL: no @feat.00 GuardCF bit (got '$FEAT')" >&2; exit 1; }
  fi
  "$CC" -mguard=cf -Wl,--guard-cf $lto "$T/cfg_c_$mode.o" "$T/cfg_asm.o" -o "$T/cfg_$mode.exe"
  "$STAGE/bin/llvm-readobj" --coff-load-config "$T/cfg_$mode.exe" > "$T/cfg_$mode.lc"
  for f in asm_fn direct_fn; do
    a=$("$STAGE/bin/llvm-nm" "$T/cfg_$mode.exe" | awk -v f=$f '$3==f{print $1; exit}')
    [ -n "$a" ] || { echo "SELFTEST FAIL: $f not in the $mode image's symbol table" >&2; exit 1; }
    va=$(printf '0x%X' $(( 0x$a & ~1 )))  # GuardFidTable lists virtual addresses, Thumb bit clear
    if awk '/GuardFidTable \[/{p=1} p&&/\]/{exit} p' "$T/cfg_$mode.lc" | grep -qix "  *$va"; then eval in_$f=yes; else eval in_$f=no; fi
  done
  [ "$in_asm_fn" = yes ] || { cat "$T/cfg_$mode.lc"; echo "SELFTEST FAIL: $mode: asm_fn not in GuardFidTable" >&2; exit 1; }
  [ "$in_direct_fn" = no ] || { cat "$T/cfg_$mode.lc"; echo "SELFTEST FAIL: $mode: directly-called direct_fn in GuardFidTable" >&2; exit 1; }
done
# (10) clang-cl for a -gnu triple with the build's MS-compatibility flags: /EHsc defines
#     __EXCEPTIONS (and no _CPPUNWIND), no /EH defines neither.
printf 'int x;\n' > "$T/eh.cc"
for eh in /EHsc ""; do
  "$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config -fms-extensions \
    -fms-compatibility -fmsc-version=1916 -fgnuc-version=4.2.1 $eh /E /clang:-dM "$T/eh.cc" > "$T/eh${eh#/}.txt"
done
grep -qx "#define __EXCEPTIONS 1" "$T/ehEHsc.txt" || { echo "SELFTEST FAIL: /EHsc did not define __EXCEPTIONS" >&2; exit 1; }
! grep -q "_CPPUNWIND" "$T/ehEHsc.txt" || { echo "SELFTEST FAIL: _CPPUNWIND defined for -gnu" >&2; exit 1; }
! grep -q "__EXCEPTIONS" "$T/eh.txt" || { echo "SELFTEST FAIL: __EXCEPTIONS without /EH" >&2; exit 1; }
# (6) the installed driver answers the plugin names the build passes.
"$STAGE/bin/clang-cl" --target=armv7-w64-windows-gnu --no-default-config /WX /c /Fo"$T/gc2.obj" \
  -Xclang -add-plugin -Xclang find-bad-constructs -Xclang -add-plugin -Xclang blink-gc-plugin "$T/gc.cc"
echo "SELFTEST PASS: /O2 $n2 < /Od $nd insns; tail-merge -> 0x$HELPER|1 via movw/movt+blx r12, $RELOCS ARM_MOV32T relocs; ThinLTO /GS link; /lib of thumbv7 bitcode; CodeView with NEON tuples ($CVR register def-ranges kept); CFG @feat.00=0x$FEAT, GuardFidTable from .gfids\$y (native+ThinLTO); __EXCEPTIONS for -gnu /EHsc; chromium plugins"
# Every test passed: install the tested tree as $OUT (the old one is removed only now).
rm -rf "$OUT.old"
[ ! -e "$OUT" ] || mv "$OUT" "$OUT.old"
mv "$STAGE" "$OUT"
rm -rf "$OUT.old"
sha256sum "$OUT/bin/clang-23" "$OUT/bin/lld"
