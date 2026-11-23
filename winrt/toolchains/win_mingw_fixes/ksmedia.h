// win-arm32 (config-fix). mingw's <ksmedia.h> predates two families of SDK additions:
//  (1) the KSAUDIO_SPEAKER_* channel-mask constants for 2.1/3.0/3.1/5.0/7.0 layouts (it has
//      QUAD/5POINT1/7POINT1 but not these); media/audio/win/core_audio_util_win.cc uses
//      KSAUDIO_SPEAKER_2POINT1 / _5POINT0 / _7POINT0.
//  (2) the Win11 extended-camera-control BACKGROUNDSEGMENTATION property id + flags;
//      media/capture/video/win/video_capture_device_mf_win.cc:1413,1609 uses
//      KSPROPERTY_CAMERACONTROL_EXTENDED_BACKGROUNDSEGMENTATION and the OFF/BLUR flags.
// Both -> "use of undeclared identifier". #include_next mingw's real header, then append the
// missing names VERBATIM from the SDK ksmedia.h (each #ifndef-guarded, so a future mingw update
// or the real xwin <ksmedia.h> never double-defines).
#include_next <ksmedia.h>

// (1) channel-mask constants -- expressed with the SPEAKER_* bits mingw already defines.
#ifndef KSAUDIO_SPEAKER_2POINT1
#define KSAUDIO_SPEAKER_2POINT1 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
#endif
#ifndef KSAUDIO_SPEAKER_3POINT0
#define KSAUDIO_SPEAKER_3POINT0 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER)
#endif
#ifndef KSAUDIO_SPEAKER_3POINT1
#define KSAUDIO_SPEAKER_3POINT1 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | \
                                 SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY)
#endif
#ifndef KSAUDIO_SPEAKER_5POINT0
#define KSAUDIO_SPEAKER_5POINT0 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | \
                                 SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
#endif
#ifndef KSAUDIO_SPEAKER_7POINT0
#define KSAUDIO_SPEAKER_7POINT0 (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | \
                                 SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | \
                                 SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
#endif

// (2) Win11 extended-camera-control BACKGROUNDSEGMENTATION. mingw's <ksmedia.h> has NONE of the
// KSPROPERTY_CAMERACONTROL_EXTENDED_* family (grep=0). The property id is enumerator 41 of
// KSPROPERTY_CAMERACONTROL_EXTENDED_PROPERTY (xwin shared/ksmedia.h:6036 PHOTOMODE=0 .. :6097
// BACKGROUNDSEGMENTATION=41, contiguous +1), reproduced as a value since mingw lacks the enum; the
// two flags are the SDK's own object-like macros (xwin ksmedia.h:6716-6717).
#ifndef KSPROPERTY_CAMERACONTROL_EXTENDED_BACKGROUNDSEGMENTATION
#define KSPROPERTY_CAMERACONTROL_EXTENDED_BACKGROUNDSEGMENTATION 41
#endif
#ifndef KSCAMERA_EXTENDEDPROP_BACKGROUNDSEGMENTATION_OFF
#define KSCAMERA_EXTENDEDPROP_BACKGROUNDSEGMENTATION_OFF                    0x0000000000000000
#endif
#ifndef KSCAMERA_EXTENDEDPROP_BACKGROUNDSEGMENTATION_BLUR
#define KSCAMERA_EXTENDEDPROP_BACKGROUNDSEGMENTATION_BLUR                   0x0000000000000001
#endif

// (3) KSNODETYPE_VIDEO_PROCESSING / _VIDEO_CAMERA_TERMINAL: mingw's <ksmedia.h> DECLARES both
// (DEFINE_GUIDSTRUCT/DEFINE_GUIDNAMED idiom -> `EXTERN_C const GUID <name>`, a declaration only)
// but NO mingw arm32 lib carries their STORAGE (llvm-nm across all libs: 0), unlike KSCATEGORY_*/
// KSDATAFORMAT_* which live in libksuser.a. media/capture/video/win/video_capture_device_win.cc
// odr-uses both, so the bare declarations are unresolved LINK symbols. Emit real storage in the
// selectany COMDAT form (== the <initguid.h> expansion). Values byte-for-byte from mingw's own
// <ksmedia.h> DEFINE_GUIDSTRUCT strings (and identical in xwin shared/ksmedia.h): E5/E6-F70F-11D0-
// B917-00A0C9223196. The self-referential DEFINE_GUIDNAMED macro expands each name to itself.
#if defined(__cplusplus)
#ifndef _WRT_KSMEDIA_VIDEO_NODETYPE_GUIDS_
#define _WRT_KSMEDIA_VIDEO_NODETYPE_GUIDS_
EXTERN_C const GUID DECLSPEC_SELECTANY KSNODETYPE_VIDEO_PROCESSING =
    { 0xdff229e5, 0xf70f, 0x11d0, { 0xb9, 0x17, 0x00, 0xa0, 0xc9, 0x22, 0x31, 0x96 } };
EXTERN_C const GUID DECLSPEC_SELECTANY KSNODETYPE_VIDEO_CAMERA_TERMINAL =
    { 0xdff229e6, 0xf70f, 0x11d0, { 0xb9, 0x17, 0x00, 0xa0, 0xc9, 0x22, 0x31, 0x96 } };
#endif
#endif
