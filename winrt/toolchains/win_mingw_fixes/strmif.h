// win-arm32 real-header fix for <strmif.h>: give ICodecAPI its __uuidof association.
//
// mingw ships ICodecAPI in BOTH <icodecapi.h> and <strmif.h>, guarded by the shared
// __ICodecAPI_INTERFACE_DEFINED__. Only <icodecapi.h>'s copy carries the
// `__CRT_UUID_DECL(ICodecAPI, 901db4c7-...)` (inside that guard); <strmif.h>'s copy declares
// `EXTERN_C const IID IID_ICodecAPI` but NO __CRT_UUID_DECL. So a TU that reaches <strmif.h>'s copy
// FIRST (media/gpu/windows/media_foundation_video_encode_accelerator_win.h:11 includes <strmif.h>)
// gets a uuid-less ICodecAPI, and `__uuidof(ICodecAPI)` (WRL ComPtr<ICodecAPI>, wrl/client.h) then
// fails "cannot call operator __uuidof on a type with no GUID".
//
// Fix: after mingw's <strmif.h>, attach the missing __CRT_UUID_DECL -- but ONLY when ICodecAPI was
// not already defined (with its uuid) before this include. If __ICodecAPI_INTERFACE_DEFINED__ is
// already set on entry, ICodecAPI came from <icodecapi.h> (or a prior pass of this shim), which
// already provided the __CRT_UUID_DECL, and re-declaring the __mingw_uuidof<ICodecAPI> specialization
// would be a redefinition. The IID is icodecapi.h's own value; nothing is invented, and we do NOT
// pull <icodecapi.h> (it re-defines strmif.h's unguarded CodecAPIEventData).
#ifndef _WRT_STRMIF_ICODECAPI_UUID_SHIM_
#define _WRT_STRMIF_ICODECAPI_UUID_SHIM_

#ifdef __ICodecAPI_INTERFACE_DEFINED__
#define _WRT_STRMIF_ICODECAPI_PREDEFINED_
#endif

#include_next <strmif.h>

#if defined(__cplusplus) && defined(__CRT_UUID_DECL) && \
    !defined(_WRT_STRMIF_ICODECAPI_PREDEFINED_)
__CRT_UUID_DECL(ICodecAPI, 0x901db4c7, 0x31ce, 0x41a2, 0x85, 0xdc, 0x8f, 0xa0, 0xbf, 0x41, 0xb8, 0xda)
#endif

#endif  // _WRT_STRMIF_ICODECAPI_UUID_SHIM_
