// win-arm32 (config-fix). xwin's um/coml2api.h (pulled by xwin um/ole2.h, itself quote-included
// by xwin MediaFoundation headers e.g. mfcontentdecryptionmodule.h:30 `#include "ole2.h"`)
// redefines the structured-storage COM surface -- `tagSTGOPTIONS`, the StgCreatePropStg/
// StgOpenPropStg/FmtIdToPropStgName prop-set API, AND the ReadClassStg/WriteClassStg/
// ReadClassStm/WriteClassStm/GetHGlobalFromILockBytes/CreateILockBytesOnHGlobal/GetConvertStg
// storage helpers -- that mingw provides across <objbase.h> (STGOPTIONS + STGM_* macros),
// <propidl.h> (the Stg* prop functions + PROPVARIANT) and <ole2.h> (the 7 storage helpers).
// mingw ships no coml2api.h, so the two headers share no guard and clash ("redefinition of
// 'tagSTGOPTIONS'"). win_mingw_fixes is first on -isystem, so <coml2api.h> resolves HERE and
// xwin's coml2api.h is never reached; this shim reproduces the FULL surface from mingw's own
// headers. NOTE the 7 storage helpers live only in mingw's <ole2.h>, which is guard-skipped in
// exactly the TUs this shim serves (xwin's ole2.h already won _OLE2_H_), so they are re-declared
// here VERBATIM from mingw's ole2.h:48-52/126-137 -- a duplicate FUNCTION declaration is
// well-formed when mingw's ole2.h is instead the winner.
#ifndef _WINRT_COML2API_SHIM_
#define _WINRT_COML2API_SHIM_
#include <objbase.h>
#include <propidl.h>

#ifdef __cplusplus
extern "C" {
#endif
WINOLEAPI ReadClassStg(LPSTORAGE pStg, CLSID *pclsid);
WINOLEAPI WriteClassStg(LPSTORAGE pStg, REFCLSID rclsid);
WINOLEAPI ReadClassStm(LPSTREAM pStm, CLSID *pclsid);
WINOLEAPI WriteClassStm(LPSTREAM pStm, REFCLSID rclsid);
WINOLEAPI GetHGlobalFromILockBytes(LPLOCKBYTES plkbyt, HGLOBAL *phglobal);
WINOLEAPI CreateILockBytesOnHGlobal(HGLOBAL hGlobal, WINBOOL fDeleteOnRelease, LPLOCKBYTES *pplkbyt);
WINOLEAPI GetConvertStg(LPSTORAGE pStg);
#ifdef __cplusplus
}
#endif
#endif
