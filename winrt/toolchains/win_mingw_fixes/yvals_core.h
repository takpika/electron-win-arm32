// win-arm32 (config-fix). ATL's cstringt.h:43 does `#include <yvals_core.h>` -- MSVC STL's internal
// "core" header -- but only to obtain STL feature-test macros; the sole one cstringt.h references is
// __cpp_lib_three_way_comparison (cstringt.h:44). llvm-mingw + Chromium's pinned libc++ have no
// <yvals_core.h>, and MSVC's own 2025-line copy (xwin crt/include/yvals_core.h) would drag in MSVC
// STL declarations that clash with libc++ (the port compiles with -nostdinc++ + Chromium libc++).
// Supply the SAME feature-test macros from the canonical libc++ source, <version>, instead. That is
// the standard, header-only feature-test-macro header, so cstringt.h's `#ifdef
// __cpp_lib_three_way_comparison` gets the correct answer for this toolchain without any MSVC STL.
#ifndef _WRT_YVALS_CORE_SHIM_
#define _WRT_YVALS_CORE_SHIM_
#include <version>
#endif  // _WRT_YVALS_CORE_SHIM_
