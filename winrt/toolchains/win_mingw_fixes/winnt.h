// win-arm32 class fix: honor DECLSPEC_UUID under the -gnu triple so SDK interfaces keep their IIDs.
// The Windows SDK tags COM/WinRT interfaces with DECLSPEC_UUID("....") and reads them via
// __uuidof. llvm-mingw's winnt.h:220 #defines DECLSPEC_UUID(x) to NOTHING on the -gnu triple
// (it carries IIDs through __CRT_UUID_DECL instead), so every SDK header that reaches this build
// via the xwin gap-fill (243 headers under xwin-sdk/sdk/include/{um,shared}) loses its IIDs and
// any __uuidof on them fails "type has no GUID" (first hit: IDXGraphicsAnalysis in dawn's
// BackendD3D12). This -isystem #1 shim pulls mingw's real <winnt.h> and then makes DECLSPEC_UUID
// expand to the standard __declspec(uuid(x)), which the port's guiddef.h primary __mingw_uuidof
// fallback already consumes (clang honors __declspec(uuid) under -fms-extensions, enabled at
// build/config/win/BUILD.gn). ONE fix for the whole class -- no per-interface GUID transcription.
//
// Safe for mingw's own interfaces: mingw carries their IIDs via __CRT_UUID_DECL, which provides
// an EXPLICIT __mingw_uuidof<T>() specialization that always outranks the primary, so __uuidof(T)
// returns the __CRT_UUID_DECL GUID and the added __declspec(uuid) is simply unused (never a
// conflicting second source). Only active under the mingw-gnu __uuidof mechanism + clang + C++.
#ifndef _WRT_WINNT_DECLSPEC_UUID_CLASS_FIX_
#define _WRT_WINNT_DECLSPEC_UUID_CLASS_FIX_
#include_next <winnt.h>
#if defined(__cplusplus) && defined(__clang__) && defined(USE___UUIDOF) && (USE___UUIDOF == 0)
#undef DECLSPEC_UUID
#define DECLSPEC_UUID(x) __declspec(uuid(x))
#endif
#endif  // _WRT_WINNT_DECLSPEC_UUID_CLASS_FIX_
