// win-arm32 real-header wiring: SDK-correct namespace placement of IID_IDeviceInformationStatics.
//
// Same mingw-vs-SDK convention gap as win_mingw_fixes/windows.devices.radios.h: the SDK puts
// `IID_IDeviceInformationStatics` as a namespace-scoped const IID& (= __uuidof(...)) inside
// ABI::Windows::Devices::Enumeration, while llvm-mingw only #defines it as a global macro.
// Chromium 109's bluetooth_adapter_winrt.cc does
// `using ABI::Windows::Devices::Enumeration::IID_IDeviceInformationStatics;` and qualified
// references, both of which need the namespace member. bluetooth_adapter_winrt.cc is the sole
// user of this IID_ macro in the tree, so #undef is build-wide safe. See the radios shim for
// the full rationale.
#ifndef _WRT_ENUMERATION_IID_NS_SHIM_
#define _WRT_ENUMERATION_IID_NS_SHIM_

#include_next <windows.devices.enumeration.h>

#if defined(__cplusplus)
#ifdef IID_IDeviceInformationStatics
#undef IID_IDeviceInformationStatics
#endif
namespace ABI { namespace Windows { namespace Devices { namespace Enumeration {
inline const GUID IID_IDeviceInformationStatics = __uuidof(IDeviceInformationStatics);
}}}}
#endif  // __cplusplus

#endif  // _WRT_ENUMERATION_IID_NS_SHIM_
