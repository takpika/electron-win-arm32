// win-arm32 mingw fix. mingw/ucrt lacks the MSVC-SDK <crtversion.h>
// that ffmpeg's libavutil/internal.h includes under HAVE_LIBC_MSVCRT. Provide it
// verbatim from the MS SDK (matches toolchains/xwin-sdk/crt/include/crtversion.h).
// Version >= 14 => ffmpeg skips its legacy _VC_CRT_MAJOR_VERSION<14 strtod shim
// (ucrt has strtod), which is correct for the llvm-mingw/ucrt runtime.
#ifndef _INC_CRTVERSION
#define _INC_CRTVERSION
#define _VC_CRT_MAJOR_VERSION 14
#define _VC_CRT_MINOR_VERSION 44
#define _VC_CRT_BUILD_VERSION 35220
#define _VC_CRT_RBUILD_VERSION 0
#endif
