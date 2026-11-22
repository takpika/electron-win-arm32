// win-arm32 real-header wiring: llvm-mingw's <windows.system.h> is a reduced WinRT header that
// ships NO Windows.System.Launcher family at all (grep ILauncherStatics / ILauncher /
// RuntimeClass_Windows_System_Launcher = 0; it does ship User). content/browser/installedapp/
// installed_app_provider_impl_win.cc uses ABI::Windows::System::ILauncherStatics4 +
// RuntimeClass_Windows_System_Launcher (GetActivationFactory). This -isystem #1 shim pulls
// mingw's real <windows.system.h> UNCHANGED via include_next (keeping every mingw interface,
// User, IPropertySet instantiations, enums and #defines), then the port-owned extension
// (win_sdk_supplement/windows.system_ext.h, widl-generated from the pinned xwin
// windows.system.idl) supplying ONLY the missing Launcher family. Additive form (same as the
// bluetooth *_ext shims) -- no full shadow, so mingw's own Windows.System declarations are
// neither re-emitted nor altered. verify_vs_xwin: the 2 emitted interfaces (ILauncherStatics4,
// ILaunchUriResult) have IID + STDMETHOD count identical to xwin, mine-only=0.
#ifndef _WRT_WINDOWS_SYSTEM_LAUNCHER_EXT_SHIM_
#define _WRT_WINDOWS_SYSTEM_LAUNCHER_EXT_SHIM_

#include_next <windows.system.h>
#include <windows.system_ext.h>

#endif  // _WRT_WINDOWS_SYSTEM_LAUNCHER_EXT_SHIM_
