// win-arm32 mingw-defect fix: the Windows SDK name of IDWriteFontFace4::GetGlyphImageFormats.
// The SDK declares two C++ overloads: GetGlyphImageFormats() (face-level formats) and
// GetGlyphImageFormats(glyph, ppemFirst, ppemLast, formats) (per glyph). mingw-w64
// generates dwrite_3.h with widl, which cannot express overloads, so the per-glyph one is
// named GetGlyphImageFormats_. Code written to the SDK API (skia's
// SkScalerContext_win_dw.cpp) then fails "too many arguments, expected 0, have 4".
//
// Vtable: MSVC lays out an overload set in REVERSE declaration order, so although the SDK
// declares the 0-arg member first, the per-glyph member occupies the lower slot. mingw
// declares them in slot order (per-glyph first -> C vtable slot 49, face-level -> 50) and
// clang's -gnu target lays out in declaration order. This wrapper only renames the
// identifier, so the slots stay exactly mingw's. Verified on device:
// raw slot 50 called with four arguments returns the face
// formats and leaves the out-argument untouched (the 0-arg getter), while the C++ 4-arg
// call through this header returns S_OK and writes it. C/CINTERFACE users keep widl's
// names. Other widl-renamed dwrite_3.h overloads have no consumer and are left alone.
#ifndef _WRT_DWRITE_3_SDK_OVERLOAD_NAMES_
#define _WRT_DWRITE_3_SDK_OVERLOAD_NAMES_
#if defined(__cplusplus) && !defined(CINTERFACE)
#define GetGlyphImageFormats_ GetGlyphImageFormats
#include_next <dwrite_3.h>
#undef GetGlyphImageFormats_
#else
#include_next <dwrite_3.h>
#endif
#endif  // _WRT_DWRITE_3_SDK_OVERLOAD_NAMES_
