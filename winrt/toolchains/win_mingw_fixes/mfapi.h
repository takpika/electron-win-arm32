// win-arm32 real-header supplement: MF attribute/event GUIDs + protection-scheme enum the SDK
// <mfapi.h> declares and llvm-mingw omits (media_foundation cdm/renderer/stream).
//
// These 7 GUIDs are absent BOTH from mingw's <mfapi.h> (grep=0) AND from mingw's arm32
// libmfuuid.a (llvm-nm: 0 hits each) -- so a bare DEFINE_GUID (which, without INITGUID, is only a
// DECLARATION `EXTERN_C const GUID name;`) would leave every odr-use as an unresolved LINK symbol
// (the two consumers media_foundation_cdm.cc / media_foundation_stream_wrapper.cc do NOT include
// <initguid.h>). We therefore emit a real STORAGE definition here, in the selectany COMDAT form --
// byte-identical to mingw guiddef.h's own INITGUID+C++ expansion (guiddef.h:61) -- so every TU
// that includes this header carries a folded definition and resolution never depends on which
// unrelated TU happened to set INITGUID. Values byte-for-byte from the SDK DEFINE_GUID lines.
#ifndef _WRT_MFAPI_GUID_SUPPLEMENT_
#define _WRT_MFAPI_GUID_SUPPLEMENT_
#include_next <mfapi.h>
// selectany COMDAT definition (the <initguid.h> expansion), independent of INITGUID in the TU:
#define _WRT_MFAPI_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1,b2,b3,b4,b5,b6,b7,b8 } }

_WRT_MFAPI_GUID(MF_EVENT_STREAM_METADATA_CONTENT_KEYIDS, 0x5063449d, 0xcc29, 0x4fc6, 0xa7, 0x5a, 0xd2, 0x47, 0xb3, 0x5a, 0xf8, 0x5c);
_WRT_MFAPI_GUID(MF_EVENT_STREAM_METADATA_SYSTEMID, 0x1ea2ef64, 0xba16, 0x4a36, 0x87, 0x19, 0xfe, 0x75, 0x60, 0xba, 0x32, 0xad);

typedef enum _MFSampleEncryptionProtectionScheme
{
    MF_SAMPLE_ENCRYPTION_PROTECTION_SCHEME_NONE = 0,
    MF_SAMPLE_ENCRYPTION_PROTECTION_SCHEME_AES_CTR = 1,
    MF_SAMPLE_ENCRYPTION_PROTECTION_SCHEME_AES_CBC = 2,
} MFSampleEncryptionProtectionScheme;
_WRT_MFAPI_GUID(MFSampleExtension_Encryption_ProtectionScheme, 0xd054d096, 0x28bb, 0x45da, 0x87, 0xec, 0x74, 0xf3, 0x51, 0x87, 0x14, 0x6);
_WRT_MFAPI_GUID(MFSampleExtension_Encryption_SubSample_Mapping, 0x8444F27A, 0x69A1, 0x48DA, 0xBD, 0x08, 0x11, 0xCE, 0xF3, 0x68, 0x30, 0xD2);
_WRT_MFAPI_GUID(MFSampleExtension_Content_KeyID, 0xc6c7f5b0, 0xacca, 0x415b, 0x87, 0xd9, 0x10, 0x44, 0x14, 0x69, 0xef, 0xc6);
_WRT_MFAPI_GUID(MF_MT_MAX_MASTERING_LUMINANCE, 0xd6c6b997, 0x272f, 0x4ca1, 0x8d, 0x0, 0x80, 0x42, 0x11, 0x1a, 0xf, 0xf6);
_WRT_MFAPI_GUID(MF_MT_MIN_MASTERING_LUMINANCE, 0x839a4460, 0x4e7e, 0x4b4f, 0xae, 0x79, 0xcc, 0x8, 0x90, 0x5c, 0x7b, 0x27);
#endif  // _WRT_MFAPI_GUID_SUPPLEMENT_
