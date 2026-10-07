// win-arm32 real-header wiring for <windows.devices.bluetooth.h> (two provable needs):
//
// (1) mingw's windows.devices.bluetooth.idl FORWARD-DECLARES IBluetoothLEDevice2/3/4 and the
//     BluetoothLEAppearance family but ships NO bodies, so As<IBluetoothLEDevice4>()
//     (bluetooth_device_winrt.cc) hits an incomplete type. The port-owned ledevice_ext header
//     (win_sdk_supplement, widl-generated from the pinned xwin idl) supplies
//     ONLY those real bodies; it re-#includes bluetooth.h for shared enums (guarded no-op).
//
// (2) widl/mingw ns_prefix headers emit no namespace-scoped IID_IXxx; the SDK does
//     (xwin windows.devices.bluetooth.h:2390 `MIDL_CONST_ID IID& IID_IBluetoothAdapterStatics
//     = __uuidof(...)`). Chromium's `using ABI::Windows::Devices::Bluetooth::
//     IID_IBluetoothAdapterStatics;` (bluetooth_adapter_winrt.cc, 4 uses) needs the member.
//
// NOTE: the Advertisement sub-namespace that bluetooth_adapter_winrt.h names is NOT wired here.
// The SDK's bluetooth.h does not expose it (verified: xwin bluetooth.h has 0 Advertisement
// decls, its idl does not import Advertisement), so injecting it into every <..bluetooth.h>
// includer would be wrong-by-construction; the fix is the IWYU include added to the one
// consuming header, device/bluetooth/bluetooth_adapter_winrt.h.
#ifndef _WRT_BLUETOOTH_WIRING_SHIM_
#define _WRT_BLUETOOTH_WIRING_SHIM_

#include_next <windows.devices.bluetooth.h>

// (1) real IBluetoothLEDevice2/3/4 + BluetoothLEAppearance bodies mingw forward-declares but
//     never defines:
#include <windows.devices.bluetooth.ledevice_ext.h>

// (2) SDK-form namespace-scoped IID_IBluetoothAdapterStatics (equals mingw's own __CRT_UUID_DECL
//     IID 8B02FB6A-... via the guiddef __uuidof bridge). Sole in-tree user is
//     bluetooth_adapter_winrt.cc.
#if defined(__cplusplus)
namespace ABI { namespace Windows { namespace Devices { namespace Bluetooth {
inline const GUID IID_IBluetoothAdapterStatics = __uuidof(IBluetoothAdapterStatics);
}}}}
#endif  // __cplusplus

#endif  // _WRT_BLUETOOTH_WIRING_SHIM_
