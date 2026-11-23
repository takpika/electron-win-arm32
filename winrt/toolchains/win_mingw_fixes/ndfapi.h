// win-arm32 real-header supplement: DEPRECATED_STDAPI for the SDK's <ndfapi.h>.
// llvm-mingw's <ndfapi.h>/<ndattrib.h> are incomplete (ndattrib.h uses ShellCommandInfo,
// LIFE_TYPE, DIAG_SOCKADDR, ... it never defines; ndfapi.h lacks NDFHANDLE), so
// chrome/browser/net/net_error_diagnostics_dialog_win.cc cannot compile against them. The
// real SDK headers are therefore supplied verbatim in toolchains/win_sdk_supplement/
// (ndfapi.h, ndattrib.h). The SDK ndfapi.h declares its API with
//   DEPRECATED_STDAPI(message)
// which the SDK defines in um/winnt.h:803 as
//   EXTERN_C __declspec(deprecated(message)) HRESULT STDAPICALLTYPE
// and mingw's winnt.h does not define. It also names the protocol parameter of
// NdfCreateInboundIncident (declared only when winsock's __CSADDR_DEFINED__ is set) with the
// SDK's `IPPROTO` type -- an int-sized enum in the SDK's ws2def.h, whose values mingw spells
// as plain int macros (IPPROTO_TCP 6, ...) without the typedef. This -isystem #1 shim
// supplies ONLY those two names (DEPRECATED_STDAPI in the SDK spelling, #ifndef-guarded;
// IPPROTO as int, the type mingw gives those values and the same by-value ABI as the SDK's
// enum) and then includes the next <ndfapi.h> -- the SDK copy.
#ifndef _WRT_NDFAPI_SUPPLEMENT_
#define _WRT_NDFAPI_SUPPLEMENT_
#include <windows.h>
#ifndef DEPRECATED_STDAPI
#define DEPRECATED_STDAPI(message) \
  EXTERN_C __declspec(deprecated(message)) HRESULT STDAPICALLTYPE
#endif
#if defined(__CSADDR_DEFINED__)
typedef int IPPROTO;
#endif
#include_next <ndfapi.h>
#endif  // _WRT_NDFAPI_SUPPLEMENT_
