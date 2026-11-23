// win-arm32 mingw-defect fix (self-contained wrapper, mingw tree left pristine).
// DEFECT (visible in the untouched mingw header): mingw-w64 20260505 REGRESSED vs 20251104
// -- 20251104 wrapped the C++ specialization `IReference<boolean>` + its __CRT_UUID_DECL in
// `#ifndef __MINGW32__`; 20260505 dropped that guard, so both `IReference<boolean>` and the
// (always-unguarded) `IReference<BYTE>` C++ specializations are emitted. `boolean` and `BYTE`
// are BOTH `unsigned char`, so they are the SAME instantiation `IReference<unsigned char>`
// declared twice with different MIDL UUIDs => redefinition error. Reproducible by compiling
// any <windows.ui.viewmanagement.h> consumer through the real wrapper (mingw is first on the
// include path; SDK winrt/ is never reached for this include).
//
// FIX: pre-set the boolean interface block's include guard before include_next, so mingw's
// header skips that whole block. NOTE ON MINIMALITY: a wrapper (include_next) CANNOT inject a
// mid-header `#ifndef __MINGW32__` around ONLY the C++ specialization the way 20251104 did --
// the sole lever a wrapper has is this block-level include guard, which also drops the block's
// C DEFINE_GUID/vtable `__FIReference_1_boolean`. That extra drop is VERIFIED HARMLESS: grep of
// the whole mingw+SDK include trees and chromium-rt/src shows NOTHING references
// `__FIReference_1_boolean` / `IID_IReference_boolean` / `IReference<boolean>` (the only near
// hit is base/win/reference_unittest.cc, which uses the DISTINCT type `IReference<bool>` with
// its own GUID). Chosen over editing the mingw download in place because that (a) is fragile
// (a re-download silently reverts it, reintroducing the bug) and (b) erases the pre-state so
// the defect becomes invisible; this wrapper keeps the mingw tree pristine and inspectable.
//
// HONEST behavior (NOT a compile error): because boolean==BYTE==unsigned char, a C++ use of
// `IReference<boolean>` still resolves -- to the surviving BYTE specialization -- and thus
// silently carries BYTE's GUID (e5198cc8), not Boolean's (3c00fd60). This is the SAME latent
// aliasing mingw's own __MINGW32__ path (20251104) produces, and is inert here (no such use in
// product code). `__MINGW32__` IS defined by this build's wrapper (verified: clang-cl.exe
// -dM -E prints `#define __MINGW32__ 1`); it is not relied on by this wrapper's mechanism.
#if defined(__cplusplus) && !defined(CINTERFACE)
#  ifndef ____FIReference_1_boolean_INTERFACE_DEFINED__
#    define ____FIReference_1_boolean_INTERFACE_DEFINED__
#  endif
#endif
#include_next <windows.foundation.h>
// NOTE: we deliberately re-emit NOTHING after include_next. The suppressed block's
// C `DEFINE_GUID(IID___FIReference_1_boolean, 3c00fd60)` is NOT re-supplied here:
// (a) it is grep-empty (no consumer in mingw/SDK/chromium-rt references
// IID___FIReference_1_boolean / a Boolean IReference), and (b) this wrapper has no
// include guard of its own, so a DEFINE_GUID here would re-execute on every
// `#include <windows.foundation.h>` in a TU and, under INITGUID (where DEFINE_GUID
// is a definition), cause `redefinition of IID___FIReference_1_boolean` — reachable
// in-tree (e.g. media/midi/midi_manager_winrt.cc defines INITGUID and pulls WinRT
// foundation headers twice). Any Boolean GUID that is ever genuinely needed belongs
// in the consuming TU (the way Chromium's base/win/reference_unittest.cc does it),
// not in this system-header wrapper.
