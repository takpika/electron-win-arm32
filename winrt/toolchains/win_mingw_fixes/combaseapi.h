// win-arm32 mingw fix. mingw-w64's <combaseapi.h> lacks the
// STDMETHOD_CHPE_PATCHABLE macro (an ARM64EC/CHPE-only variant used by the
// MSVC-SDK WRL headers). CHPE patching does not exist on ARM32, so it degrades
// to a plain STDMETHOD. Pull the real mingw header, then add the macro.
#include_next <combaseapi.h>
#ifndef STDMETHOD_CHPE_PATCHABLE
#define STDMETHOD_CHPE_PATCHABLE(method) STDMETHOD(method)
#endif
