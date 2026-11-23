// win-arm32 config-fix. Pull the real mingw <uiautomationclient.h>, then supply
// the two UI Automation event/pattern id constants that Chromium's accessibility layer
// references but that llvm-mingw-20260505's <uiautomationclient.h> OMITS (it predates these
// SDK additions). Include order (clang-cl.exe: win_mingw_fixes -isystem FIRST, then mingw)
// makes <uiautomationclient.h> resolve here; #include_next reaches mingw's for the rest.
//
// Values are VERBATIM from the in-tree real MS SDK header
// toolchains/xwin-sdk/sdk/include/um/UIAutomationClient.h (lines 848, 764) — authentic SDK
// ids, not invented. Only the two ids Chromium actually uses are added (grep of ui/ and
// content/: UIA_ActiveTextPositionChangedEventId in 2 files, UIA_SelectionPattern2Id in 1;
// the SDK's other two mingw-missing ids — UIA_SummaryChangeId, UIA_SayAsInterpretAsMetadataId
// — have 0 Chromium references and are intentionally NOT added, keeping this minimal).

#include_next <uiautomationclient.h>

#ifndef WRT_UIACLIENT_EVENTID_SUPPLEMENT
#define WRT_UIACLIENT_EVENTID_SUPPLEMENT

const long UIA_SelectionPattern2Id	=	10034;
const long UIA_ActiveTextPositionChangedEventId	=	20036;

#endif  // WRT_UIACLIENT_EVENTID_SUPPLEMENT
