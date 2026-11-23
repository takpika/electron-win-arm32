// win-arm32 real-header supplement: MF protected-content interfaces llvm-mingw omits.
// mingw ships <mfidl.h> without IMFTrustedInput / IMFContentEnabler /
// IMFContentProtectionManager (all Media-Foundation protected-content / DRM interfaces the
// SDK <mfidl.h> declares); media/renderers/win/* + media/cdm/win/* need them. This -isystem
// #1 shim pulls mingw real <mfidl.h> then adds those interface definitions VERBATIM from the
// SDK (xwin um/mfidl.h) with their IIDs re-attached via mingw __CRT_UUID_DECL (values from
// the SDK MIDL_INTERFACE tags).
#ifndef _WRT_MFIDL_PROTECTED_CONTENT_SHIM_
#define _WRT_MFIDL_PROTECTED_CONTENT_SHIM_
#include_next <mfidl.h>
#if defined(__cplusplus) && !defined(CINTERFACE)
// This TU sees exactly ONE <mfidl.h>: xwin um/mfidl.h and mingw's mfidl.h share the same top-level
// guard __mfidl_h__ (xwin 33/34, mingw 16/17), so whichever is reached first wholly suppresses the
// other. Some MF/xwin headers quote-include mfidl.h relative-first to xwin's copy (e.g. xwin
// mfmediaengine.h:430), which DEFINES these interfaces AND the MFENABLETYPE storage; on that
// "xwin-first" path our #include_next above reaches mingw's copy as a no-op. On the "mingw-only"
// path mingw's copy (lacking these interfaces + GUIDs) wins and the shim must supply them.
//
// __IMFTrustedInput_INTERFACE_DEFINED__ is the precise detector of "xwin's <mfidl.h> won": xwin sets
// it, mingw's copy never does (it has no such interface). Because BOTH the interface bodies AND the
// MFENABLETYPE_* GUID storage sit behind xwin's single __mfidl_h__ file guard, the presence of that
// macro entails xwin supplied BOTH -- so the gate below covers both. NO __X_INTERFACE_DEFINED__
// macro is pre-set by the shim (a later xwin include is already a no-op via __mfidl_h__, so setting
// one would be dead code and would elide xwin's own IID_/Vtbl/COBJMACROS decls in a live order).
//
// GATED (mingw-only): the interface bodies AND the MFENABLETYPE storage. On the xwin-first path
// xwin um/mfidl.h:8026-8028 EXTERN_GUID already emits the storage -- in THIS toolchain it expands
// via mingw rpcndr.h:857 to `EXTERN_C const IID DECLSPEC_SELECTANY itf={...}`, a DEFINITION (mingw
// wins the two-rpcndr.h race by -isystem order; proven by clang -E)
// -- so re-emitting unconditionally would be a "redefinition of
// MFENABLETYPE_MF_RebootRequired" (verified). UNGATED (both paths): only the __CRT_UUID_DECL, below
// -- xwin provides NO __uuidof binding (mingw __uuidof reads only __CRT_UUID_DECL; winnt.h:220
// DECLSPEC_UUID empty; xwin uses MIDL_INTERFACE/DECLSPEC_UUID), so it is needed on the xwin path too
// and never collides. Both orders compile+link with __uuidof==SDK-IID.
#if !defined(__IMFTrustedInput_INTERFACE_DEFINED__)

MIDL_INTERFACE("542612C4-A1B8-4632-B521-DE11EA64A0B0")
    IMFTrustedInput : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetInputTrustAuthority( 
            /* [in] */ DWORD dwStreamID,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ __RPC__deref_out_opt IUnknown **ppunkObject) = 0;
        
    };

MIDL_INTERFACE("D3C4EF59-49CE-4381-9071-D5BCD044C770")
    IMFContentEnabler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEnableType( 
            /* [out] */ __RPC__out GUID *pType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnableURL( 
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcchURL) LPWSTR *ppwszURL,
            /* [out] */ __RPC__out DWORD *pcchURL,
            /* [unique][out][in] */ __RPC__inout_opt MF_URL_TRUST_STATUS *pTrustStatus) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEnableData( 
            /* [size_is][size_is][out] */ __RPC__deref_out_ecount_full_opt(*pcbData) BYTE **ppbData,
            /* [out] */ __RPC__out DWORD *pcbData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAutomaticSupported( 
            /* [out] */ __RPC__out BOOL *pfAutomatic) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AutomaticEnable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE MonitorEnable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Cancel( void) = 0;
        
    };

MIDL_INTERFACE("ACF92459-6A61-42bd-B57C-B43E51203CB0")
    IMFContentProtectionManager : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE BeginEnableContent( 
            /* [in] */ IMFActivate *pEnablerActivate,
            /* [in] */ IMFTopology *pTopo,
            /* [in] */ IMFAsyncCallback *pCallback,
            /* [in] */ IUnknown *punkState) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE EndEnableContent(
            /* [in] */ IMFAsyncResult *pResult) = 0;

    };

// MF content-enabler MFENABLETYPE_* GUID STORAGE -- gated on the SAME condition as the interface
// bodies above, and for the same reason: these GUIDs live in xwin um/mfidl.h:8026-8028 as
// EXTERN_GUID(...) which, in THIS toolchain, expands via mingw rpcndr.h:857
// (EXTERN_C const IID DECLSPEC_SELECTANY itf = {...}, a DEFINITION -- mingw wins the two-rpcndr.h
// race because -isystem orders mingw before xwin/shared). So on the xwin-first path xwin's copy
// ALREADY emits these three as selectany COMDAT storage; re-emitting here would be a redefinition
// (verified: without this gate, clang reports "redefinition of MFENABLETYPE_MF_RebootRequired" vs
// xwin mfidl.h:8028). On the mingw-only path mingw's <mfidl.h> declares none of the three (grep=0)
// and mingw libmfuuid.a defines none (llvm-nm: 0), and the consumer
// media_foundation_protection_manager.cc odr-uses all three without <initguid.h>, so the shim
// must supply the selectany definition. __IMFTrustedInput_INTERFACE_DEFINED__ is the precise
// detector of "xwin's <mfidl.h> won this TU": xwin sets it, mingw's copy (lacking these
// interfaces) never does -- and because BOTH the interfaces and the MFENABLETYPE storage sit
// behind xwin's single __mfidl_h__ file guard, the presence of one entails the presence of the
// other. This is not incidental coupling: it is one file's contents gated by one detector.
#ifndef _WRT_MFIDL_ENABLETYPE_
#define _WRT_MFIDL_ENABLETYPE_
#define _WRT_MFIDL_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1,b2,b3,b4,b5,b6,b7,b8 } }
_WRT_MFIDL_GUID(MFENABLETYPE_MF_RebootRequired, 0x6d4d3d4b, 0x0ece, 0x4652, 0x8b, 0x3a, 0xf2, 0xd2, 0x42, 0x60, 0xd8, 0x87);
_WRT_MFIDL_GUID(MFENABLETYPE_MF_UpdateRevocationInformation, 0xe558b0b5, 0xb3c4, 0x44a0, 0x92, 0x4c, 0x50, 0xd1, 0x78, 0x93, 0x23, 0x85);
_WRT_MFIDL_GUID(MFENABLETYPE_MF_UpdateUntrustedComponent, 0x9879f3d6, 0xcee2, 0x48e6, 0xb5, 0x73, 0x97, 0x67, 0xab, 0x17, 0x2f, 0x16);
#endif  // _WRT_MFIDL_ENABLETYPE_
#endif  // !__IMFTrustedInput_INTERFACE_DEFINED__ (interfaces + MFENABLETYPE storage: both come from
        // xwin's <mfidl.h> when it won the TU, so the shim re-emits neither)

// __uuidof binding for the three interfaces -- emitted UNCONDITIONALLY (outside the gate above),
// because it is the ONE thing xwin's copy does NOT provide on either path: mingw's __uuidof(T) ==
// __mingw_uuidof<T>() (_mingw.h:588) is fed ONLY by __CRT_UUID_DECL specializations, never by
// xwin's MIDL_INTERFACE/DECLSPEC_UUID (mingw winnt.h:220 defines DECLSPEC_UUID empty). So an
// xwin-first TU has the interface types + the GUID storage but NO __uuidof binding unless this
// shim supplies it here. One-shot per TU via _WRT_MFIDL_PROTECTED_CONTENT_SHIM_; the type named is
// xwin's own IMFTrustedInput etc. on the xwin path, this shim's verbatim copy on the mingw path;
// there is no competing __CRT_UUID_DECL in either order, so no redefinition.
__CRT_UUID_DECL(IMFTrustedInput, 0x542612C4, 0xA1B8, 0x4632, 0xB5, 0x21, 0xDE, 0x11, 0xEA, 0x64, 0xA0, 0xB0)
__CRT_UUID_DECL(IMFContentEnabler, 0xD3C4EF59, 0x49CE, 0x4381, 0x90, 0x71, 0xD5, 0xBC, 0xD0, 0x44, 0xC7, 0x70)
__CRT_UUID_DECL(IMFContentProtectionManager, 0xACF92459, 0x6A61, 0x42bd, 0xB5, 0x7C, 0xB4, 0x3E, 0x51, 0x20, 0x3C, 0xB0)

// --- Win11 MF extended-camera-control interfaces (SDK <mfidl.h>, NTDDI_WIN10_VB+) that llvm-mingw
// omits. media/capture/video/win/video_capture_device_mf_win.* uses IMFExtendedCameraController
// (via IMFGetService::GetService with IID_IMFExtendedCameraController) and IMFExtendedCameraControl
// (via ComPtr, needs __uuidof). Same structure/reasoning as the protected-content block above:
// each interface BODY is gated on ITS OWN xwin interface guard (set iff xwin's <mfidl.h> won this TU
// AND its NTDDI gate was active, so the guard covaries exactly with "xwin already defined it");
// the __CRT_UUID_DECL specializations are UNCONDITIONAL (xwin's DECLSPEC_UUID does not feed mingw's
// __uuidof). Bodies are verbatim from xwin um/mfidl.h (22239-22261, 22378-22391); only the
// MIDL_INTERFACE line is at column 0 (matching the sibling blocks' accepted style).
#if !defined(__IMFExtendedCameraControl_INTERFACE_DEFINED__)
MIDL_INTERFACE("38E33520-FCA1-4845-A27A-68B7C6AB3789")
    IMFExtendedCameraControl : public IUnknown
    {
    public:
        virtual ULONGLONG STDMETHODCALLTYPE GetCapabilities( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetFlags(
            /* [annotation][in] */
            _In_  ULONGLONG ulFlags) = 0;

        virtual ULONGLONG STDMETHODCALLTYPE GetFlags( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE LockPayload(
            /* [annotation][out] */
            _Outptr_result_buffer_(*pulPayload)  BYTE **ppPayload,
            /* [annotation][out] */
            _Out_  ULONG *pulPayload) = 0;

        virtual HRESULT STDMETHODCALLTYPE UnlockPayload( void) = 0;

        virtual HRESULT STDMETHODCALLTYPE CommitSettings( void) = 0;

    };
#endif  // !__IMFExtendedCameraControl_INTERFACE_DEFINED__

#if !defined(__IMFExtendedCameraController_INTERFACE_DEFINED__)
MIDL_INTERFACE("B91EBFEE-CA03-4AF4-8A82-A31752F4A0FC")
    IMFExtendedCameraController : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetExtendedCameraControl(
            /* [annotation][in] */
            _In_  DWORD dwStreamIndex,
            /* [annotation][in] */
            _In_  ULONG ulPropertyId,
            /* [annotation][out] */
            _COM_Outptr_  IMFExtendedCameraControl **ppControl) = 0;

    };
#endif  // !__IMFExtendedCameraController_INTERFACE_DEFINED__

// IID_IMFExtendedCameraController STORAGE: the consumer names the IID symbol explicitly
// (video_capture_device_mf_win.cc:977 GetService(GUID_NULL, IID_IMFExtendedCameraController, ...)).
// xwin um/mfidl.h:22374 only DECLARES it (`EXTERN_C const IID IID_IMFExtendedCameraController;`) and
// mingw's arm32 libmfuuid.a defines it NOWHERE (llvm-nm: 0) -- so on BOTH include orders it is an
// unresolved link symbol. Emit a real selectany COMDAT definition (idempotent via its own guard);
// this is a definition compatible with xwin's extern declaration and COMDAT-folds if ever duplicated.
#ifndef _WRT_MFIDL_EXTCAM_IID_
#define _WRT_MFIDL_EXTCAM_IID_
EXTERN_C const GUID DECLSPEC_SELECTANY IID_IMFExtendedCameraController =
    { 0xB91EBFEE, 0xCA03, 0x4AF4, { 0x8A, 0x82, 0xA3, 0x17, 0x52, 0xF4, 0xA0, 0xFC } };
#endif  // _WRT_MFIDL_EXTCAM_IID_

__CRT_UUID_DECL(IMFExtendedCameraControl, 0x38E33520, 0xFCA1, 0x4845, 0xA2, 0x7A, 0x68, 0xB7, 0xC6, 0xAB, 0x37, 0x89)
__CRT_UUID_DECL(IMFExtendedCameraController, 0xB91EBFEE, 0xCA03, 0x4AF4, 0x8A, 0x82, 0xA3, 0x17, 0x52, 0xF4, 0xA0, 0xFC)
#endif  // __cplusplus && !CINTERFACE
#endif  // _WRT_MFIDL_PROTECTED_CONTENT_SHIM_
