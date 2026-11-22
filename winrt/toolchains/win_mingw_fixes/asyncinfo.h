// win-arm32 real-header bridge: SDK-correct ABI placement of AsyncStatus / IAsyncInfo.
//
// The Windows SDK's <asyncinfo.h> (see the pinned xwin winrt/asyncinfo.h:72) defines
//     namespace ABI { namespace Windows { namespace Foundation {
//         enum class AsyncStatus { ... };
//         MIDL_INTERFACE(...) IAsyncInfo : public IInspectable { ... };
//     }}}
// i.e. AsyncStatus and IAsyncInfo live in ABI::Windows::Foundation. llvm-mingw's real
// <asyncinfo.h> instead defines the SAME two types at GLOBAL scope (::AsyncStatus,
// ::IAsyncInfo) -- mingw's whole WinRT stack refers to them unqualified, resolving to
// global, so it is self-consistent. But WRL's <wrl/async.h> (SDK-spelled, pulled from the
// xwin gap-fill because mingw ships no wrl/async.h) refers to them as
// ABI::Windows::Foundation::AsyncStatus and ABI::Windows::Foundation::IAsyncInfo, and
// then cannot find them against mingw's global placement ("no member named 'AsyncStatus'
// in namespace 'ABI::Windows::Foundation'"; IAsyncInfo overrides fail to bind).
//
// This -isystem #1 shim pulls mingw's REAL <asyncinfo.h> and re-exports its two types
// into ABI::Windows::Foundation via using-declarations, so an SDK-spelled name and a
// mingw-spelled name denote the IDENTICAL type (no second definition, no fabricated
// declaration -- pure aliasing of the real mingw types into the SDK-correct namespace).
#ifndef _WRT_ASYNCINFO_ABI_BRIDGE_
#define _WRT_ASYNCINFO_ABI_BRIDGE_

#include_next <asyncinfo.h>

#if defined(__cplusplus)
namespace ABI { namespace Windows { namespace Foundation {
using ::AsyncStatus;
using ::IAsyncInfo;
}}}
#endif  // __cplusplus

#endif  // _WRT_ASYNCINFO_ABI_BRIDGE_
