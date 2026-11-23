// win-arm32 (config-fix, port-owned shim). Inventory measured against the mingw the build
// ACTUALLY uses -- $LLVM_MINGW = Downloads/llvm-mingw-20260505 (win_wrappers/bin/clang-cl.exe
// line 2; -isystem 127-128), NOT the stale `toolchains/llvm-mingw` -> 20251104 compat symlink.
// 20260505 ships `wrl/` = client.h, internal.h, module.h, wrappers/corewrappers.h -- and NO
// implements.h / event.h / async.h / ftm.h / def.h. So <wrl/implements.h>, <wrl/event.h> etc.
// already resolve to xwin's real Microsoft WRL (nothing shadows them). The WinRT event path
// `Microsoft::WRL::Callback` (services/device/generic_sensor/platform_sensor_reader_winrt.cc,
// media/midi/midi_manager_winrt.cc, components/system_media_controls/win/system_media_controls_win.cc)
// pulls xwin's event.h + implements.h, whose ComPtr/HString machinery needs xwin's client.h and
// corewrappers.h -- but mingw ships those two as strict SUBSETS: mingw client.h line 12 is
// `/* #include <weakreference.h> */` (no WeakRef/AgileRef body), mingw corewrappers.h has only
// HString/HStringReference/RoInitializeWrapper (no SRWLock/Event/CriticalSection). Because the
// -isystem path is global, forwarding these two mingw-shipped subset headers to xwin is the only
// way xwin's event.h/implements.h see a consistent object model.
//
// Forwarded set is exactly TWO: client.h (this file) and wrappers/corewrappers.h -- the only
// mingw-shipped WRL headers that (a) xwin event.h/implements.h require and (b) mingw provides
// only as subsets. internal.h is NOT forwarded (a byte-identical verbatim MS copy already sits
// at win_sdk_supplement/wrl/internal.h, `diff`=0, ahead of mingw on -isystem). implements.h is
// NOT forwarded (mingw 20260505 ships none; <wrl/implements.h> already resolves to xwin). def.h
// resolves to xwin via the sibling _MSC_VER shim [[toolchains/win_mingw_fixes/wrl/def.h]].
// module.h is NOT forwarded: no event TU includes it, mingw's module.h is a 23-line stub defining
// no Microsoft::WRL::Module (`grep -c 'class Module'`=0, nothing to hybridize), and xwin's newer
// module.h uses ARM64EC-only STDMETHOD_CHPE_PATCHABLE + legacy Register/RevokeActivationFactories
// absent under mingw, so forwarding it would ADD errors; its consumers (chrome/elevation_service,
// chrome/updater) are a SEPARATE pre-existing gap, not on content_shell.exe's path.
//
// This does NOT introduce a new hybrid: BEFORE this shim, the whole build already compiled xwin
// event.h/implements.h/async.h over mingw's client.h (mingw lacks the formers) -- forwarding
// client.h/corewrappers.h to xwin makes the family MORE uniform, and xwin-WRL-over-mingw-base-SDK
// (<weakreference.h>/<unknwn.h>/<roapi.h> still from mingw) is the established, already-compiling
// port layering used by base/win's WinRT TUs and third_party/dawn. Kept under win_mingw_fixes
// rather than as a wrapper -isystem reorder, so the whole fix stays in one place.
// The include names the SDK's winrt/ directory through its sibling um/ and shared/
// directories (every Windows SDK Include/<version>/ has um, shared, winrt side by side; the build
// puts um and shared on the search path, build/config/win/BUILD.gn win_sdk_splat_dir), so it does
// not depend on where the SDK is installed: no other search-path directory has ../winrt/wrl/.
// Runtime correctness (ComPtr ref-count, WeakRef, HString lifetime, Callback + EventSource
// InvokeAll) is device-verified on Surface Pro X (WoW64 AArch32), not compile-only.
#include <../winrt/wrl/client.h>
