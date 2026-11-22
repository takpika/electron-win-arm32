// win-arm32 real-header supplement: network property-bag names.
// The Windows SDK's <netlistmgr.h> (um/netlistmgr.h:209-210) defines the
// INetworkConnection property-bag keys
//   #define NA_InternetConnectivityV4 L"NA_InternetConnectivityV4"
//   #define NA_InternetConnectivityV6 L"NA_InternetConnectivityV6"
// whose values are NLM_INTERNET_CONNECTIVITY flags. llvm-mingw's <netlistmgr.h>
// ships the NLM_INTERNET_CONNECTIVITY enum but none of the NA_* key names, so
// components/security_interstitials/content/captive_portal_helper_win.cc fails
// "use of undeclared identifier". This -isystem #1 shim pulls mingw's real
// <netlistmgr.h> and adds ONLY the two keys that code reads, spelled exactly as
// the SDK spells them.
// The SDK header also includes <ocidl.h> (um/netlistmgr.h:171), which declares
// IConnectionPoint / IConnectionPointContainer; mingw's stops at <oaidl.h> /
// <objidl.h>. net/base/network_cost_change_notifier_win.cc (Chromium 108)
// subscribes to INetworkCostManagerEvents through those interfaces after
// including only <netlistmgr.h>, so the shim includes <ocidl.h> as the SDK does.
#ifndef _WRT_NETLISTMGR_SUPPLEMENT_
#define _WRT_NETLISTMGR_SUPPLEMENT_
#include_next <netlistmgr.h>
#include <ocidl.h>
#ifndef NA_InternetConnectivityV4
#define NA_InternetConnectivityV4 L"NA_InternetConnectivityV4"
#endif
#ifndef NA_InternetConnectivityV6
#define NA_InternetConnectivityV6 L"NA_InternetConnectivityV6"
#endif
#endif  // _WRT_NETLISTMGR_SUPPLEMENT_
