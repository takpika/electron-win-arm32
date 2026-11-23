// win-arm32 (config-fix). mingw's <ncrypt.h> predates the TPM PSS-salt-size properties added
// to the Windows SDK; net/ssl/ssl_platform_key_win.cc uses NCRYPT_PCP_PSS_SALT_SIZE_PROPERTY
// and NCRYPT_TPM_PSS_SALT_SIZE_HASHSIZE -> "use of undeclared identifier". #include_next
// mingw's real header and append the two missing definitions VERBATIM from the SDK ncrypt.h,
// each #ifndef-guarded.
#include_next <ncrypt.h>

#ifndef NCRYPT_PCP_PSS_SALT_SIZE_PROPERTY
#define NCRYPT_PCP_PSS_SALT_SIZE_PROPERTY L"PSS Salt Size"
#endif
#ifndef NCRYPT_TPM_PSS_SALT_SIZE_HASHSIZE
#define NCRYPT_TPM_PSS_SALT_SIZE_HASHSIZE 0x00000002
#endif
