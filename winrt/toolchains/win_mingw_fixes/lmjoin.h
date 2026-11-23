// win-arm32 mingw fix (same mechanism as this dir's wincrypt.h/tchar.h).
// llvm-mingw's <lmjoin.h> predates the Windows 10 Azure-AD-join APIs: it lacks
// NetGetAadJoinInformation / NetFreeAadJoinInformation and the DSREG_* types that
// the real Windows SDK (xwin um/LMJoin.h) declares under _WIN32_WINNT >= WIN10.
// Chromium's base/win/win_util.cc uses these (resolved dynamically via
// GetProcAddress, so declarations-only is sufficient — no import library).
//
// Pull in mingw's real <lmjoin.h> first (so its NET_API_FUNCTION/typedefs/guard
// are in effect), then supplement ONLY the missing AAD declarations, copied
// VERBATIM from the SDK (SAL annotations dropped — they are no-op hints).
// mingw and the SDK share the include guard __LMJOIN_H__ (verified in both), so
// once mingw's header runs, the SDK's um/LMJoin.h no-ops if ever reached — no
// redefinition. Our own re-entry guard below makes re-inclusion idempotent.
// Matches [[winrt-real-header-integration]] / [[winrt-real-headers-no-shims]].
#include_next <lmjoin.h>

#ifndef WINRT_LMJOIN_AAD_SUPPLEMENT
#define WINRT_LMJOIN_AAD_SUPPLEMENT

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

typedef enum _DSREG_JOIN_TYPE {
  DSREG_UNKNOWN_JOIN = 0,
  DSREG_DEVICE_JOIN = 1,
  DSREG_WORKPLACE_JOIN = 2
} DSREG_JOIN_TYPE, *PDSREG_JOIN_TYPE;

typedef struct _DSREG_USER_INFO {
  LPWSTR pszUserEmail;
  LPWSTR pszUserKeyId;
  LPWSTR pszUserKeyName;
} DSREG_USER_INFO, *PDSREG_USER_INFO;

#ifndef __WINCRYPT_H__  // kept in sync with wincrypt.h (per the SDK's own note).
typedef const struct _CERT_CONTEXT* PCCERT_CONTEXT;
#endif  // __WINCRYPT_H__

typedef struct _DSREG_JOIN_INFO {
  DSREG_JOIN_TYPE joinType;
  PCCERT_CONTEXT pJoinCertificate;
  LPWSTR pszDeviceId;
  LPWSTR pszIdpDomain;
  LPWSTR pszTenantId;
  LPWSTR pszJoinUserEmail;
  LPWSTR pszTenantDisplayName;
  LPWSTR pszMdmEnrollmentUrl;
  LPWSTR pszMdmTermsOfUseUrl;
  LPWSTR pszMdmComplianceUrl;
  LPWSTR pszUserSettingSyncUrl;
  DSREG_USER_INFO* pUserInfo;
} DSREG_JOIN_INFO, *PDSREG_JOIN_INFO;

#ifdef __cplusplus
extern "C" {
#endif

HRESULT NET_API_FUNCTION
NetGetAadJoinInformation(LPCWSTR pcszTenantId, PDSREG_JOIN_INFO* ppJoinInfo);

VOID NET_API_FUNCTION
NetFreeAadJoinInformation(PDSREG_JOIN_INFO pJoinInfo);

#ifdef __cplusplus
}
#endif

#endif  // (_WIN32_WINNT >= _WIN32_WINNT_WIN10)

#endif  // WINRT_LMJOIN_AAD_SUPPLEMENT
