// win-arm32 (config-fix, port-owned toolchain shim). Make mingw's MIDL_INTERFACE
// carry the interface UUID the MSVC way, so __uuidof works for every COM/WinRT
// interface declared by the MSVC-style SDK headers (toolchains/xwin-sdk um/winrt),
// which Chromium/Dawn use via __uuidof / IID_PPV_ARGS.
//
// DEFECT (in the untouched mingw headers): mingw's rpcndr.h defines
//   #define MIDL_INTERFACE(x) struct
// i.e. a BARE struct that DROPS the uuid string, unlike MSVC's
//   #define MIDL_INTERFACE(x) struct __declspec(uuid(x)) __declspec(novtable)
// mingw carries IIDs for its OWN interfaces via a separate __CRT_UUID_DECL. But the
// MSVC-style SDK headers (xwin) declare `MIDL_INTERFACE("<uuid>") IFoo : ...` with NO
// __CRT_UUID_DECL, so under mingw those interfaces end up with NO uuid at all and
// `__uuidof(IFoo)` (guiddef.h's __mingw_uuidof primary -> clang's native operator)
// errors "cannot call operator __uuidof on a type with no GUID" (e.g.
// ISwapChainPanelNative in third_party/dawn/.../SwapChainD3D12.cpp).
//
// FIX: after mingw's rpcndr.h, redefine MIDL_INTERFACE to also attach
// __declspec(uuid(x)). Guarded by __has_declspec_attribute(uuid), which is true iff
// the compiler accepts __declspec(uuid) (clang with -fms-extensions/-fms-compatibility,
// or MSVC) -- so the redefine never turns __declspec(uuid) into a parse error on a
// plain-gnu TU. ADDITIVE and non-invasive:
//  * for mingw's own interfaces the attached uuid is byte-identical to their
//    __CRT_UUID_DECL (verified: across all mingw MIDL_INTERFACE headers every uuid
//    string matches its __CRT_UUID_DECL numeric uuid, 0 divergences), and the explicit
//    __mingw_uuidof<T> specialization still OUTRANKS the guiddef.h primary, so their
//    resolution is unchanged;
//  * every MIDL_INTERFACE type now carries its OWN uuid, which REDUCES (never widens)
//    clang's base-class __uuidof fallback vs guiddef.h alone -- a derived interface
//    resolves to its own IID, not a base's (device/object-verified: ISwapChainPanelNative
//    and its derived ISwapChainPanelNative2 each resolve to their distinct own IIDs).
// novtable is deliberately omitted so the struct/vtable/ABI shape stays byte-identical
// to mingw's bare `struct`; the only added semantic is the uuid attribute.
#include_next <rpcndr.h>

#if defined(__cplusplus) && defined(__has_declspec_attribute) && \
    !defined(_WINRT_FIX_MIDL_INTERFACE_UUID_)
#if __has_declspec_attribute(uuid)
#define _WINRT_FIX_MIDL_INTERFACE_UUID_ 1
#undef MIDL_INTERFACE
#define MIDL_INTERFACE(x) struct __declspec(uuid(x))
#endif
#endif
