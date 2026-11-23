// win-arm32 real-header fix for <minappmodel.h> (package-identity length constants).
// llvm-mingw's generic-w64-mingw32/include/minappmodel.h opens with
//   #ifdef _MINAPPMODEL_H_
//   #define _MINAPPMODEL_H_
// (an inverted include guard, still so in mingw-w64 master), so on first inclusion the
// whole body -- every PACKAGE_*_LENGTH constant -- is skipped. <appmodel.h> includes it
// for these constants; electron/shell/common/application_info_win.cc sizes its
// GetPackageFamilyName() buffer with PACKAGE_FAMILY_NAME_MAX_LENGTH and fails
// "use of undeclared identifier". This -isystem #1 shim pulls mingw's header unchanged
// and adds ONLY the constant that code reads plus the two it is defined from, with the
// Windows SDK's values and partition (um/minappmodel.h:14, :27, :31, :46-47).
#ifndef _WRT_MINAPPMODEL_SUPPLEMENT_
#define _WRT_MINAPPMODEL_SUPPLEMENT_
#include_next <minappmodel.h>
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#ifndef PACKAGE_NAME_MAX_LENGTH
#define PACKAGE_NAME_MAX_LENGTH 50
#endif
#ifndef PACKAGE_PUBLISHERID_MAX_LENGTH
#define PACKAGE_PUBLISHERID_MAX_LENGTH 13
#endif
#ifndef PACKAGE_FAMILY_NAME_MAX_LENGTH
#define PACKAGE_FAMILY_NAME_MAX_LENGTH \
  (PACKAGE_NAME_MAX_LENGTH + 1 + PACKAGE_PUBLISHERID_MAX_LENGTH)
#endif
#endif
#endif  // _WRT_MINAPPMODEL_SUPPLEMENT_
