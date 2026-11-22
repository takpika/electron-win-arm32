// win-arm32 mingw-defect fix: make <cryptuiapi.h> self-contained.
// The Windows SDK's <cryptuiapi.h> (um/cryptuiapi.h:24-26) includes <wintrust.h>,
// <wincrypt.h> and <prsht.h>; llvm-mingw's includes only the first two, yet its
// CRYPTUI_VIEWCERTIFICATE_STRUCT uses LPCPROPSHEETPAGEW/A from <prsht.h>. A TU that
// includes <cryptuiapi.h> without <prsht.h> first (chrome/browser/ui/views/
// certificate_viewer_win.cc, chrome/browser/ui/webui/settings/settings_utils_win.cc)
// fails "unknown type name 'LPCPROPSHEETPAGEW'". This -isystem #1 shim includes
// <prsht.h> -- as the SDK header does -- and then mingw's real <cryptuiapi.h>.
#ifndef _WRT_CRYPTUIAPI_SELF_CONTAINED_
#define _WRT_CRYPTUIAPI_SELF_CONTAINED_
#include <windows.h>
#include <prsht.h>
#include_next <cryptuiapi.h>
#endif  // _WRT_CRYPTUIAPI_SELF_CONTAINED_
