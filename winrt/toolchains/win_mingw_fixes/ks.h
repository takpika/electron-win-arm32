// win-arm32 real-header supplement for <ks.h>: DEFINE the KSCATEGORY_SENSOR_CAMERA class GUID,
// which the SDK <ks.h> declares and llvm-mingw omits. mingw ships KSCATEGORY_VIDEO_CAMERA (ks.h:649)
// -- but only as a DECLARATION (DEFINE_GUIDSTRUCT->EXTERN_C const GUID name); its DATA lives in
// mingw's libksguid.a/libksuser.a. KSCATEGORY_SENSOR_CAMERA is declared by neither header NOR any
// mingw arm32 lib (llvm-nm --defined-only over armv7-w64-mingw32/lib/*.a: 0 hits), and this port has
// no arm32 ksuser.lib to supply it. So a bare DEFINE_GUIDSTRUCT/DEFINE_GUIDNAMED (a declaration)
// would compile but leave an UNRESOLVED symbol at link (media/capture/video/win/
// video_capture_device_factory_win.cc:155 odr-uses it). Instead emit the real DATA as a selectany
// COMDAT definition (the INITGUID form), byte-identical to the SDK GUID {24E552D7-...} -- so every
// including TU carries a folded definition and the reference resolves without any KS import lib.
#ifndef _WRT_KS_SENSOR_CAMERA_SHIM_
#define _WRT_KS_SENSOR_CAMERA_SHIM_
#include_next <ks.h>
#ifndef KSCATEGORY_SENSOR_CAMERA
EXTERN_C const GUID DECLSPEC_SELECTANY KSCATEGORY_SENSOR_CAMERA =
    { 0x24e552d7, 0x6523, 0x47f7, { 0xa6, 0x47, 0xd3, 0x46, 0x5b, 0xf1, 0xf5, 0xca } };
#endif  // KSCATEGORY_SENSOR_CAMERA
#endif  // _WRT_KS_SENSOR_CAMERA_SHIM_
