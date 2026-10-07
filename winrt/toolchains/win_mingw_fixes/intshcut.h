// win-arm32 real-header supplement: IID of IUniformResourceLocatorW.
// The Windows SDK's <intshcut.h> (um/intshcut.h:250) declares
//   DECLARE_INTERFACE_IID_(IUniformResourceLocatorW, IUnknown,
//                          "cabb0da0-da57-11cf-9974-0020afd79762")
// so __uuidof(IUniformResourceLocatorW) / IID_PPV_ARGS work. llvm-mingw's
// <intshcut.h> declares the same interface with plain DECLARE_INTERFACE_ (no IID
// attached; the GUID exists only as IID_IUniformResourceLocatorW in isguids.h), so
// chrome/utility/importer/ie_importer_win.cc's IID_PPV_ARGS(&url_locator) fails
// "cannot call operator __uuidof on a type with no GUID". This -isystem #1 shim
// pulls mingw's real <intshcut.h> and attaches ONLY that IID, with the SDK's value,
// through mingw's own __CRT_UUID_DECL mechanism.
#ifndef _WRT_INTSHCUT_SUPPLEMENT_
#define _WRT_INTSHCUT_SUPPLEMENT_
#include_next <intshcut.h>
#if defined(__cplusplus)
__CRT_UUID_DECL(IUniformResourceLocatorW, 0xcabb0da0, 0xda57, 0x11cf, 0x99, 0x74,
                0x00, 0x20, 0xaf, 0xd7, 0x97, 0x62)
#endif
#endif  // _WRT_INTSHCUT_SUPPLEMENT_
