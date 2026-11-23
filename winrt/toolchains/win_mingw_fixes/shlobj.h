// win-arm32 (config-fix). llvm-mingw's <shlobj.h>:10 UNCONDITIONALLY `#include <wincrypt.h>`
// (the MS SDK's shlobj.h does not -- grep: zero hits), leaking wincrypt's
// X509_NAME/X509_EXTENSIONS/PKCS7_SIGNER_INFO macros (`#define X509_NAME ((LPCSTR)7)`).
// Those collide with boringssl's `typedef struct X509_name_st X509_NAME;` (openssl/base.h) in
// every TU that includes both shlobj and boringssl -- e.g. base/files/file_util_win.cc:12
// (<shlobj.h>) + :36 -> base/rand_util.h -> openssl/rand.h -> base.h -> "expected ')'".
// Chromium already defines the contract for exactly this in base/win/wincrypt_shim.h: pull
// wincrypt then `#undef` those three macros (callers that want the CryptoAPI constants use the
// WINCRYPT_* aliases). This shim applies the same undefs after mingw's shlobj pulls wincrypt,
// so shlobj presents the same (wincrypt-macro-free) surface the MS SDK's does. win_mingw_fixes
// is first on -isystem, so <shlobj.h> resolves here; #include_next pulls mingw's real shlobj.h.
#include_next <shlobj.h>
#undef X509_NAME
#undef X509_EXTENSIONS
#undef PKCS7_SIGNER_INFO
