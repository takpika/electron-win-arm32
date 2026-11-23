// win-arm32 real-header wiring: supply the BluetoothLEAdvertisementPublisher family that
// llvm-mingw's windows.devices.bluetooth.advertisement.h omits (Watcher-only; grep
// IBluetoothLEAdvertisementPublisher = 0). device/bluetooth/bluetooth_advertisement_winrt.{h,cc}
// use IBluetoothLEAdvertisementPublisher / ...Factory / ...StatusChangedEventArgs. This
// -isystem #1 shim pulls mingw's real header UNCHANGED (include_next -> keeps every mingw
// Watcher interface, the __FIIterable_1_GUID / __FIIterator_1_GUID instantiations and every
// enum/#define) and then the port-owned extension (win_sdk_supplement, widl-generated from the
// pinned xwin idl) supplying ONLY the missing Publisher family; the extension re-#includes this
// header (guarded no-op) for the shared BluetoothLEAdvertisementPublisherStatus enum, never
// redefining it. Additive form (ledevice_ext / genericattributeprofile_ext pattern) -- no full
// shadow, so mingw's Watcher-side declarations are not re-emitted or altered.
#ifndef _WRT_BT_ADVERTISEMENT_PUBLISHER_EXT_SHIM_
#define _WRT_BT_ADVERTISEMENT_PUBLISHER_EXT_SHIM_

#include_next <windows.devices.bluetooth.advertisement.h>
#include <windows.devices.bluetooth.advertisement_ext.h>

#endif  // _WRT_BT_ADVERTISEMENT_PUBLISHER_EXT_SHIM_
