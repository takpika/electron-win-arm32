// win-arm32 real-header supplement: the two Bluetooth-LE device-interface GUIDs the SDK
// <bthledef.h> declares and llvm-mingw omits STORAGE for.
//
// mingw's <bthledef.h>:17-18 DECLARES both via DEFINE_GUID -- which, without INITGUID, expands to
// a bare `EXTERN_C const GUID <name>;` DECLARATION -- but NO mingw arm32 lib defines their storage
// (llvm-nm across all arm32 libs: 0 each). device/bluetooth/bluetooth_low_energy_win.cc odr-uses
// &GUID_BLUETOOTHLE_DEVICE_INTERFACE / &GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE, so the bare
// declarations are unresolved LINK symbols. Emit real storage definitions in the selectany COMDAT
// form (== the <initguid.h> expansion), so every including TU carries a folded definition and
// resolution never depends on which unrelated TU happened to #define INITGUID.
//
// Values byte-for-byte from mingw's own <bthledef.h> DEFINE_GUID lines, cross-checked against the
// in-tree Rust winapi (third_party/rust/winapi/v0_3/crate/src/um/bthledef.rs:11,13):
//   GUID_BLUETOOTHLE_DEVICE_INTERFACE            = {781AEE18-7733-4CE4-ADD0-91F41C67B592}
//   GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE = {6E3BB679-4372-40C8-9EAA-4509DF260CD8}
#ifndef _WRT_BTHLEDEF_GUID_SUPPLEMENT_
#define _WRT_BTHLEDEF_GUID_SUPPLEMENT_
#include_next <bthledef.h>

EXTERN_C const GUID DECLSPEC_SELECTANY GUID_BLUETOOTHLE_DEVICE_INTERFACE =
    { 0x781aee18, 0x7733, 0x4ce4, { 0xad, 0xd0, 0x91, 0xf4, 0x1c, 0x67, 0xb5, 0x92 } };
EXTERN_C const GUID DECLSPEC_SELECTANY GUID_BLUETOOTH_GATT_SERVICE_DEVICE_INTERFACE =
    { 0x6e3bb679, 0x4372, 0x40c8, { 0x9e, 0xaa, 0x45, 0x09, 0xdf, 0x26, 0x0c, 0xd8 } };
#endif  // _WRT_BTHLEDEF_GUID_SUPPLEMENT_
