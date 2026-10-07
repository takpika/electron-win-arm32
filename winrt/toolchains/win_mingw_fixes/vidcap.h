// win-arm32 real-header supplement for <vidcap.h>: make the KSCATEGORY_SENSOR_CAMERA definition
// reach every TU that pulls <vidcap.h>, regardless of include form.
//
// media/capture/video/win/video_capture_device_factory_win.cc uses KSCATEGORY_SENSOR_CAMERA but
// reaches KS only transitively: video_capture_device_win.h:15 `#include <vidcap.h>` ->
// mingw vidcap.h:72 `#include "ks.h"`. That inner include is QUOTE-form, so it resolves next to
// vidcap.h in generic-w64-mingw32/include (mingw's <ks.h>), BYPASSING the -isystem #1
// win_mingw_fixes/ks.h shim -- and mingw's <ks.h> lacks KSCATEGORY_SENSOR_CAMERA. Because the outer
// `#include <vidcap.h>` is ANGLE-form, THIS shim (-isystem #1) is reached first: it pulls mingw's
// real <vidcap.h> (whose quoted "ks.h" sets __KS__ and defines the KS machinery) and THEN routes
// through the win_mingw_fixes/ks.h shim, which supplies the missing KSCATEGORY_SENSOR_CAMERA GUID
// DATA (selectany COMDAT). One-shot via that shim's own guard, so no double definition if a TU also
// includes <ks.h> directly (e.g. video_capture_device_mf_win.cc:8).
#ifndef _WRT_VIDCAP_SENSOR_CAMERA_SHIM_
#define _WRT_VIDCAP_SENSOR_CAMERA_SHIM_
#include_next <vidcap.h>
#include <ks.h>
#endif  // _WRT_VIDCAP_SENSOR_CAMERA_SHIM_
