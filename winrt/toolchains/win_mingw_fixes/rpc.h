// win-arm32 real-header fix for <rpc.h>: reproduce the MS SDK's ZERO-TOUCH handling of the COM
// `interface` convenience macro under WIN32_LEAN_AND_MEAN.
//
// mingw's <rpc.h>:10-13 does `#undef interface / #define interface struct` for every non-ObjC TU.
// The MS SDK's <rpc.h> never mentions `interface` at all; that macro is defined (to the COM
// interface / `struct`) only via um/windows.h -> ole2.h -> objbase.h -> basetyps.h, which windows.h
// pulls ONLY when WIN32_LEAN_AND_MEAN is NOT set. Chromium builds -DWIN32_LEAN_AND_MEAN
// (build/config/win/BUILD.gn:595-596), so on stock MSVC `interface` is NOT a macro after <rpc.h> and
// Chromium sources use it as an ordinary identifier (device/fido/aoa/android_accessory_discovery.h:43
// `uint8_t interface;`, reached via device/bluetooth/public/cpp/bluetooth_uuid.h -> <rpc.h>).
//
// FIX: reproduce MSVC's "rpc.h leaves the `interface` macro exactly as it was" property WITHOUT
// editing Chromium source, by bracketing mingw's real <rpc.h> with push_macro/pop_macro -- the same
// idiom mingw's OWN windows.h:15-20 uses around its `interface` redefinition. push before / pop after
// restores the PRIOR macro state whether or not `interface` was defined on entry. A blind `#undef`
// (the earlier formulation) is wrong: in an ordering like `hidclass.h -> rpc.h -> punknown.h` mingw
// basetyps.h defines `interface` WITHOUT consuming <rpc.h>, and MSVC (whose rpc.h is inert) keeps it
// defined there; a blind `#undef` would destroy that definition, diverging from MSVC. push/pop cannot
// drop a definition MSVC keeps, by construction. Only under WIN32_LEAN_AND_MEAN: without it the COM
// chain legitimately defines `interface` and mingw's rpc.h agrees, so no neutralization is wanted.
// No include guard of its own: mingw's <rpc.h> is re-entered by design (it includes <windows.h>
// before setting __RPC_H__, and without WIN32_LEAN_AND_MEAN windows.h -> ole2.h -> ... ->
// <rpcndr.h> includes <rpc.h> again, which must reach mingw's body before rpcndr.h uses
// __RPC_USER); a guard here would turn that nested include into nothing.
#if defined(WIN32_LEAN_AND_MEAN)
#pragma push_macro("interface")
#include_next <rpc.h>
#pragma pop_macro("interface")
#else
#include_next <rpc.h>
#endif
