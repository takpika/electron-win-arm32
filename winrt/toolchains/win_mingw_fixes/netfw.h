// win-arm32 real-header supplement: definitions for two Windows Firewall CLSIDs.
// <netfw.h> (SDK and mingw alike) only DECLARES the coclass ids
//   EXTERN_C const CLSID CLSID_NetFwPolicy2;   // E2B3C97F-6AE1-41AC-817A-F6F92166D7DD
//   EXTERN_C const CLSID CLSID_NetFwRule;      // 2C5BC43E-3369-4C33-AB0C-BE9469677AF4
// and an MSVC build links their definitions from the SDK's uuid.lib. llvm-mingw's
// libuuid.a (what uuid.lib resolves to here) has no NetFw entries, so
// chrome/installer/util/advanced_firewall_manager_win.cc (CoCreateInstance of both)
// fails to link: "undefined symbol: CLSID_NetFwPolicy2 / CLSID_NetFwRule".
// This -isystem #1 shim pulls mingw's real <netfw.h> and supplies ONLY those two
// definitions, as selectany data -- exactly what mingw's own DEFINE_GUID emits under
// INITGUID -- so any number of TUs may include it and the linker keeps one copy.
#ifndef _WRT_NETFW_SUPPLEMENT_
#define _WRT_NETFW_SUPPLEMENT_
#include_next <netfw.h>
EXTERN_C const GUID DECLSPEC_SELECTANY CLSID_NetFwPolicy2 = {
    0xe2b3c97f, 0x6ae1, 0x41ac, {0x81, 0x7a, 0xf6, 0xf9, 0x21, 0x66, 0xd7, 0xdd}};
EXTERN_C const GUID DECLSPEC_SELECTANY CLSID_NetFwRule = {
    0x2c5bc43e, 0x3369, 0x4c33, {0xab, 0x0c, 0xbe, 0x94, 0x69, 0x67, 0x7a, 0xf4}};
#endif  // _WRT_NETFW_SUPPLEMENT_
