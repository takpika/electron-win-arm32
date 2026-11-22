// win-arm32 real-header fix for ATL's <atlcomcli.h> (SAL _Check_return_ semantics).
// ATL is the MSVC SDK's (atlmfc/include, -idirafter), written for MSVC's <sal.h>, where
// _Check_return_ expands to nothing outside /analyze (shared/sal.h:554 -> _SAL2_Source_
// -> _Check_return_impl_, empty at :2066 in a non-/analyze build). On the *-windows-gnu
// triple mingw's <sal.h> is the one in effect and maps it to a GCC attribute instead
// (sal.h:12-13 __inner_checkReturn -> __attribute__((warn_unused_result)), :340
// _Check_return_ -> __checkReturn), so every ATL member annotated _Check_return_ becomes
// warn_unused_result and Chromium's -Werror turns an ignored HRESULT into a build error:
// electron/shell/browser/ui/win/jump_list.cc:170 `destinations_.CoCreateInstance(...)`
// (CComPtrBase::CoCreateInstance, atlcomcli.h:309) fails -Wunused-result. This -isystem #1
// shim gives ATL's own header the expansion it was written for, and only for its own text:
// _Check_return_ is saved, defined empty around the real header, then restored, so mingw's
// meaning is unchanged everywhere outside ATL.
#ifndef _WRT_ATLCOMCLI_SUPPLEMENT_
#define _WRT_ATLCOMCLI_SUPPLEMENT_
#include <sal.h>
#pragma push_macro("_Check_return_")
#undef _Check_return_
#define _Check_return_
#include_next <atlcomcli.h>
#pragma pop_macro("_Check_return_")
#endif  // _WRT_ATLCOMCLI_SUPPLEMENT_
