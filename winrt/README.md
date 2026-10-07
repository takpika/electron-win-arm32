# Electron for Windows on ARM32

Electron 22.3.27 for 32-bit ARM Windows: Surface 2 (Windows RT 8.1) and ARM64 Windows devices
running ARM32 code (tested on Surface Pro X). It is built on the Windows on ARM32 port of
Chromium 108.0.5359.215 ([takpika/chromium-win-arm32](https://github.com/takpika/chromium-win-arm32),
branch `winarm32-108.0.5359.215`).

## Downloads

Releases (tags `v<version>-win-arm32`) carry:

| file | contents |
|---|---|
| `electron-v<version>-win32-armv7l.zip` | Electron; unzip and run `electron.exe` |
| `node-v<version>-headers.tar.gz` | node headers for building native addons |
| `win-armv7l-node.lib` | `node.lib` for native addons (`electron.lib`, the import library of `electron.exe`) |

`armv7l` is the name Electron's tools (`@electron/get`, `electron-packager`) use for 32-bit ARM.
`v8_context_snapshot.bin` is not included: the context snapshot is off for this cross build
(`use_v8_context_snapshot` is false when the build host is not Windows), and Electron starts
from `snapshot_blob.bin`.

## Layout of this branch

- Electron's own sources carry the port's changes directly.
- `DEPS` takes Chromium from the port's branch (`chromium_win_arm32_commit`); that branch's
  `winrt/patches/` (its changes to Chromium's DEPS-pinned dependencies) are applied first, then
  Electron's `patches/` as usual. Beyond Electron's own patches:
  - `patches/chromium/build_win_arm32_*.patch`: the Chromium-side changes Electron needs on top of
    the Chromium port,
  - `patches/node/build_win_arm32_*.patch`: the port's changes to node,
  - `patches/perfetto/define_ssize_t_to_be_intptr_t_to_match_libuv.patch` is rebased onto the
    port's `sys_types.h`.
- `winrt/toolchains/` holds the toolchain recipes and supplements this build uses: llvm-mingw
  with patched clang/lld (`build_llvm_winrt.sh`), the Windows SDK/CRT splat
  (`fetch_xwin_splat.sh`), CRT runtimes, header supplements, and the GN arguments
  (`winrt_arm_args.gni`).
- `winrt/ci/` builds everything on a linux host (x86_64 or aarch64): `checkout.sh`, `toolchain.sh`,
  `build.sh <minutes>`, `package.py`.

The build directory's `args.gn` is Electron's release arguments plus the port's:

```
import("//electron/build/args/release.gn")
import("/home/winrt/toolchains/win_mingw_fixes/winrt_arm_args.gni")
```

## Building

`.github/workflows/build.yml` runs on every push (GitHub-hosted `ubuntu-24.04` runners).
The toolchain is built once and cached by the hash of its recipes; the build is split into
stages that hand `out/` to each other, because a job is limited to six hours. Pushing a tag
`v*-win-arm32` publishes the release files as a release.
