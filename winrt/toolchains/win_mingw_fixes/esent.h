// win-arm32 mingw-defect fix: make <esent.h> self-contained.
// llvm-mingw's <esent.h> includes only <_mingw_unicode.h> but uses the mingw base
// macros defined by <_mingw.h> (__LONG32 in `typedef unsigned __LONG32 JET_COLUMNID;`,
// __C89_NAMELESS, ...). It therefore compiles only if some earlier header already
// pulled <_mingw.h>. chrome/utility/importer/edge_database_reader_win.h includes
// <esent.h> first (as the SDK header allows), so the TU fails "expected ';' after
// top level declarator" at JET_COLUMNID. This -isystem #1 shim includes <_mingw.h>
// (the header that defines those macros) and then mingw's real <esent.h>.
#ifndef _WRT_ESENT_SELF_CONTAINED_
#define _WRT_ESENT_SELF_CONTAINED_
#include <_mingw.h>
#include_next <esent.h>
// Second mingw defect: its A/W selector for JetGetTableColumnInfo has a typo --
// `#define JetGetTableColumnInfoW __MINGW_NAME_AW(JetGetTableColumnInfo)` (defines the
// W name instead of the generic one), so the generic JetGetTableColumnInfo that every
// other Jet* function gets is missing and edge_database_reader_win.cc fails
// "undeclared identifier". Supply the generic name exactly as the SDK selects it
// (um/esent.h:4769-4773):
#ifndef JetGetTableColumnInfo
#ifdef JET_UNICODE
#define JetGetTableColumnInfo JetGetTableColumnInfoW
#else
#define JetGetTableColumnInfo JetGetTableColumnInfoA
#endif
#endif
#endif  // _WRT_ESENT_SELF_CONTAINED_
