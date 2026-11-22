// win-arm32 real-header wiring: supply the GATT interface bodies llvm-mingw forward-declares
// but never defines. mingw's windows.devices.bluetooth.genericattributeprofile.idl forward-
// declares IGattReadResult2, IGattPresentationFormatStatics and IGattPresentationFormatStatics2
// with NO bodies, so Chromium's As<IGattReadResult2>() (bluetooth_remote_gatt_*_winrt.cc) hits
// an incomplete type. This -isystem #1 shim pulls mingw's real header and then the port-owned
// extension (win_sdk_supplement, widl-generated from the pinned xwin idl) that supplies ONLY
// those missing real interface bodies; the extension re-#includes the mingw header for the
// shared GATT enums rather than redefining them, so it does not collide with mingw's guarded
// copies. Nothing stubbed -- real SDK interface bodies wired at the include point.
#ifndef _WRT_GATT_PRESENTATION_READRESULT2_EXT_SHIM_
#define _WRT_GATT_PRESENTATION_READRESULT2_EXT_SHIM_

#include_next <windows.devices.bluetooth.genericattributeprofile.h>
#include <windows.devices.bluetooth.genericattributeprofile_ext.h>

#endif  // _WRT_GATT_PRESENTATION_READRESULT2_EXT_SHIM_
