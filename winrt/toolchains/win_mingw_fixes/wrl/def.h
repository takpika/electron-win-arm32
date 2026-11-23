// win-arm32 fix: the MSVC-SDK WRL <wrl/def.h> has `#if _MSC_VER < 1600 #error WRL
// requires compiler version 16.00 or greater`, but clang on --target=armv7-w64-windows-gnu does
// NOT define _MSC_VER (verified: -dM shows __GNUC__/__clang__/__MINGW32__/_WIN32, no _MSC_VER;
// -fmsc-version needs -fms-compatibility, unusable on -gnu). The WRL headers include def.h as
// `<wrl\def.h>` (backslash); clang-cl mode normalizes '\'->'/', so this forward-slash shim on
// the FIRST -isystem dir intercepts it, defines _MSC_VER ONLY across the real def.h include,
// then restores. A GLOBAL -D_MSC_VER is proven catastrophic (flips Chromium's pinned libc++ from
// Itanium to Microsoft ABI -> _LIBCPP_ABI_VCRUNTIME -> pulls MSVC-CRT-only vcruntime_exception.h,
// absent here). Across the WRL tree _MSC_VER is used only for this gate + a benign _PREFAST_ check.
#ifndef _WINRT_WRL_DEF_H_SHIM_
#define _WINRT_WRL_DEF_H_SHIM_
#  ifndef _MSC_VER
#    define _WINRT_WRL_SHIM_DEFINED_MSC_VER_ 1916
#    define _MSC_VER _WINRT_WRL_SHIM_DEFINED_MSC_VER_
#  endif
#  include_next <wrl/def.h>
#  ifdef _WINRT_WRL_SHIM_DEFINED_MSC_VER_
#    undef _MSC_VER
#    undef _WINRT_WRL_SHIM_DEFINED_MSC_VER_
#  endif
#endif  // _WINRT_WRL_DEF_H_SHIM_
