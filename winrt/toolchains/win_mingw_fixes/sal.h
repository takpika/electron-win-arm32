// win-arm32 real-header fix for <sal.h> (two independent, additive gaps; NOTHING removed).
//
// llvm-mingw's generic-w64-mingw32/include/sal.h implements a PARTIAL SAL (578 lines vs the
// MSVC-SDK's 1032). Two distinct real needs, both additive (#include_next pulls mingw's header
// byte-unchanged, every define below is #ifndef-guarded so it can never clobber mingw's own):
//
// (A) TEN SAL-1 deref/return annotations the MSVC-SDK ATL headers (toolchains/msvc-atl/include,
//     on the -isystem path; no mingw ATL exists) use but mingw's sal.h never defines. In the
//     MSVC SDK each is object-like with an EMPTY arg list -- _SAL1_1_Source_(Name,(),...) -- and
//     in a non-/analyze build every terminal *_impl_ expands to nothing, so the SDK's own
//     expansion IS the empty token sequence (xwin shared/sal.h:999/1028/1102/1103/1107/1121/
//     1194/1195/1198/1423). Empty object-like defines are mechanically equivalent, matching
//     mingw's own convention of bare empty SAL macros. Consumed via base/win/atl.h ->
//     <atlbase.h>/<atlcom.h>/<atlctl.h>/<atlwin.h> (e.g. ui/gfx/win/msg_util.h).
//
// (B) The SAL-1 __in annotation, which mingw's sal.h:526-529 DISABLES for C++ (only __in and
//     __out are gated; __inout/__deref_out at :556/:571 stay defined) with FIXME "conflicts with
//     argument names in libstdc++". This build is -nostdinc++ + Chromium's pinned libc++
//     (win_wrappers/bin/clang-cl.exe:84-90) -- mingw's libstdc++ is NEVER on the include path, so
//     the stated conflict cannot occur. Identifier-position sweep of the ACTUAL closure (Chromium
//     pinned libc++ + libc++abi, and base/media/content/services/ui/device) finds 0 uses of __in
//     as an identifier. media/renderers/win/media_engine_extension.cc:73,79 uses __in as a SAL
//     annotation on IMFMediaEngineExtension method overrides (its partner __deref_out is already
//     defined by mingw), so only __in is restored. __out is NOT added: no reproduced failure
//     reaches it (media_engine_extension uses __in + __deref_out only), and minimality forbids the
//     speculative define.
//
// (C) _Interlocked_operand_ : SAL-2 annotation used BARE in winrt/wrl/implements.h's
//     `#if defined(_ARM_)` Interlocked helpers (the exact win-arm32 path) and undefined by mingw;
//     the MSVC SDK defines it empty. Kept here (the SAL header) rather than intrin.h.
#ifndef _WRT_SAL_GAPFILL_SHIM_
#define _WRT_SAL_GAPFILL_SHIM_

#include_next <sal.h>

// (A) deref/return SAL-1 annotations the ATL + WinRT/ETW SDK headers need; empty == the SDK's own
//     non-/analyze expansion. Object-like, unconditional, individually guarded (cannot clobber
//     mingw's). `_Ret_` (bare) is used by the SDK shared/TraceLoggingProvider.h (TLG helper
//     `_Ret_ void* Fill(...)`, consumed by v8's recorder-win.cc); xwin sal.h:998 defines it
//     _SAL1_1_Source_(_Ret_,(),_Ret_valid_) -> empty in a non-/analyze build.
#ifndef _Ret_
#define _Ret_
#endif
#ifndef _Ret_opt_
#define _Ret_opt_
#endif
#ifndef _Deref_pre_z_
#define _Deref_pre_z_
#endif
#ifndef _Deref_pre_valid_
#define _Deref_pre_valid_
#endif
#ifndef _Deref_pre_opt_valid_
#define _Deref_pre_opt_valid_
#endif
#ifndef _Deref_pre_maybenull_
#define _Deref_pre_maybenull_
#endif
#ifndef _Deref_post_valid_
#define _Deref_post_valid_
#endif
#ifndef _Deref_post_opt_valid_
#define _Deref_post_opt_valid_
#endif
#ifndef _Deref_post_maybenull_
#define _Deref_post_maybenull_
#endif
#ifndef _Deref_post_opt_z_
#define _Deref_post_opt_z_
#endif
#ifndef _Deref_prepost_opt_z_
#define _Deref_prepost_opt_z_
#endif

#if defined(__cplusplus)
// (B) restore the SAL-1 __in annotation mingw gates off for C++ (safe here: -nostdinc++ + libc++).
#ifndef __in
#define __in
#endif
// (C) SAL-2 annotation used bare by the win-arm32 WRL Interlocked helpers.
#ifndef _Interlocked_operand_
#define _Interlocked_operand_
#endif
#endif  // __cplusplus

#endif  // _WRT_SAL_GAPFILL_SHIM_
