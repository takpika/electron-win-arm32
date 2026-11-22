// win-arm32 real-header supplement: IMFMediaEngineProtectedContent + the
// MF_MEDIA_ENGINE_PROTECTION_FLAGS enum the SDK <mfmediaengine.h> declares and llvm-mingw omits
// (media/renderers/win/media_engine_extension.* etc.). Pulls mingw real header + adds them,
// IID via __CRT_UUID_DECL from the SDK MIDL_INTERFACE tag. Needs the protected-content
// interfaces from <mfidl.h> (IMFContentProtectionManager) -- included below.
#ifndef _WRT_MFMEDIAENGINE_PROTECTED_SHIM_
#define _WRT_MFMEDIAENGINE_PROTECTED_SHIM_
#include_next <mfmediaengine.h>
#include <mfidl.h>
#if defined(__cplusplus) && !defined(CINTERFACE)
// Emit the enum + interface BODY only when the REAL SDK <mfmediaengine.h> (xwin) has NOT already
// been pulled into this TU. mingw's <mfmediaengine.h> and xwin's um/mfmediaengine.h share the SAME
// file guard __mfmediaengine_h__, so a shim-first TU wholly skips xwin's copy (mine defines both);
// an xwin-first TU (e.g. media_foundation_cdm_factory.h -> mfcontentdecryptionmodule.h quote-includes
// xwin's copy) already has both, and re-emitting would redefine. xwin's copy, when it wins, DEFINES
// the enum (SDK line 5615) and the interface (SDK lines 2914-3082) and, incidentally, sets
// __IMFMediaEngineProtectedContent_INTERFACE_DEFINED__ -- so reading that macro is a sound (if
// file-guard-derived) proxy for "xwin already supplied enum+interface". We do NOT #define that macro
// ourselves: doing so would assert xwin's full contract (IID_ symbol, C-style Vtbl/COBJMACROS block)
// that this shim does not supply, and it is unnecessary (the shared __mfmediaengine_h__ already
// suppresses xwin's copy in the shim-first order).
#if !defined(__IMFMediaEngineProtectedContent_INTERFACE_DEFINED__)

typedef 
enum MF_MEDIA_ENGINE_PROTECTION_FLAGS
    {
        MF_MEDIA_ENGINE_ENABLE_PROTECTED_CONTENT	= 1,
        MF_MEDIA_ENGINE_USE_PMP_FOR_ALL_CONTENT	= 2,
        MF_MEDIA_ENGINE_USE_UNPROTECTED_PMP	= 4
    } 	MF_MEDIA_ENGINE_PROTECTION_FLAGS;

MIDL_INTERFACE("9f8021e8-9c8c-487e-bb5c-79aa4779938c")
    IMFMediaEngineProtectedContent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShareResources( 
            /* [annotation] */ 
            _In_  IUnknown *pUnkDeviceContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRequiredProtections( 
            /* [annotation][out] */ 
            _Out_  DWORD *pFrameProtectionFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOPMWindow( 
            /* [annotation][in] */ 
            _In_  HWND hwnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE TransferVideoFrame( 
            /* [annotation][in] */ 
            _In_  IUnknown *pDstSurf,
            /* [annotation][in] */ 
            _In_opt_  const MFVideoNormalizedRect *pSrc,
            /* [annotation][in] */ 
            _In_  const RECT *pDst,
            /* [annotation][in] */ 
            _In_opt_  const MFARGB *pBorderClr,
            /* [annotation][out] */ 
            _Out_  DWORD *pFrameProtectionFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetContentProtectionManager( 
            /* [annotation][in] */ 
            _In_opt_  IMFContentProtectionManager *pCPM) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetApplicationCertificate( 
            /* [annotation][in] */ 
            _In_reads_bytes_(cbBlob)  const BYTE *pbBlob,
            /* [annotation][in] */ 
            _In_  DWORD cbBlob) = 0;
        
    };
#endif  // !__IMFMediaEngineProtectedContent_INTERFACE_DEFINED__ (enum + interface body only)

// __CRT_UUID_DECL is emitted UNCONDITIONALLY (outside the interface-body gate): mingw's
// __uuidof(T) == __mingw_uuidof<__typeof(T)>() (_mingw.h:588) is fed ONLY by __CRT_UUID_DECL
// specializations, never by xwin's MIDL_INTERFACE/DECLSPEC_UUID (mingw winnt.h:220 defines
// DECLSPEC_UUID empty). So in an xwin-first TU the interface exists but has NO __uuidof binding
// unless this shim supplies it, and ComPtr::As<IMFMediaEngineProtectedContent>
// (media/renderers/win/media_foundation_renderer.cc:330) requires it. The SDK never emits
// __CRT_UUID_DECL itself, and this file's own _WRT_MFMEDIAENGINE_PROTECTED_SHIM_ guard makes the
// declaration one-shot per TU, so there is never a competing/duplicate specialization in either
// include order (xwin-first or shim-first).
__CRT_UUID_DECL(IMFMediaEngineProtectedContent, 0x9f8021e8, 0x9c8c, 0x487e, 0xbb, 0x5c, 0x79, 0xaa, 0x47, 0x79, 0x93, 0x8c)
#endif  // __cplusplus && !CINTERFACE
#endif  // _WRT_MFMEDIAENGINE_PROTECTED_SHIM_
