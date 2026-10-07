// win-arm32 real-header supplement: Text Services Framework (TSF) constants + one compartment GUID
// declaration the SDK <msctf.h> has and llvm-mingw's <msctf.h> omits. ui/base/ime/win/tsf_bridge.cc +
// tsf_event_router.cc use them (sentence-mode / phrase-prediction input handling).
#ifndef _WRT_MSCTF_SUPPLEMENT_
#define _WRT_MSCTF_SUPPLEMENT_
#include_next <msctf.h>

// scalar constants (values byte-for-byte from the SDK <msctf.h>: 866/1023/932):
#ifndef TF_CLIENTID_NULL
#define TF_CLIENTID_NULL    ((TfClientId)0)
#endif
#ifndef TF_INVALID_EDIT_COOKIE
#define TF_INVALID_EDIT_COOKIE ( 0 )
#endif
#ifndef TF_SENTENCEMODE_PHRASEPREDICT
#define TF_SENTENCEMODE_PHRASEPREDICT 0x0008
#endif

// GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE: the SDK <msctf.h>:819 declares it (EXTERN_C const
// GUID) and llvm-mingw's <msctf.h> does not; tsf_bridge.cc:238 odr-uses it. Its value is defined by
// the SDK Uuid.Lib the build links (sdk/lib/um/arm, member msctf_g.obj; GUID_PROP_COMPOSING, which
// mingw's <msctf.h>:592 already declares, is defined there too), so only the declaration is added.
#ifndef _WRT_MSCTF_SENTENCE_GUID_
#define _WRT_MSCTF_SENTENCE_GUID_
EXTERN_C const GUID GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE;
#endif

// ITfCandidateListUIElement: the SDK <msctf.h> declares it (candidate-window UI element) and
// llvm-mingw's <msctf.h> omits it. ui/base/ime/win/tsf_event_router.cc:245-246 does
// ComPtr<ITfCandidateListUIElement> + ui_element.As(&...), which needs the interface body AND
// __uuidof. Same structure as the mfidl.h shim: the BODY is gated on the SDK's own interface guard
// (set iff xwin's <msctf.h> won this TU and already defined it -- covaries exactly), and the
// __CRT_UUID_DECL is UNCONDITIONAL (xwin's MIDL_INTERFACE/DECLSPEC_UUID does not feed mingw's
// __uuidof, which reads only __CRT_UUID_DECL). Body verbatim from xwin um/msctf.h:12339-12371
// (8 methods, base ITfUIElement -- which mingw's <msctf.h> already provides, used above at line 242).
#if defined(__cplusplus) && !defined(CINTERFACE)
#if !defined(__ITfCandidateListUIElement_INTERFACE_DEFINED__)
MIDL_INTERFACE("ea1ea138-19df-11d7-a6d2-00065b84435c")
    ITfCandidateListUIElement : public ITfUIElement
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetUpdatedFlags(
            /* [out] */ DWORD *pdwFlags) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetDocumentMgr(
            /* [out] */ ITfDocumentMgr **ppdim) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCount(
            /* [out] */ UINT *puCount) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetSelection(
            /* [out] */ UINT *puIndex) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetString(
            /* [in] */ UINT uIndex,
            /* [out] */ BSTR *pstr) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPageIndex(
            /* [length_is][size_is][out] */ UINT *pIndex,
            /* [in] */ UINT uSize,
            /* [out] */ UINT *puPageCnt) = 0;

        virtual HRESULT STDMETHODCALLTYPE SetPageIndex(
            /* [size_is][in] */ UINT *pIndex,
            /* [in] */ UINT uPageCnt) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetCurrentPage(
            /* [out] */ UINT *puPage) = 0;

    };
#endif  // !__ITfCandidateListUIElement_INTERFACE_DEFINED__
__CRT_UUID_DECL(ITfCandidateListUIElement, 0xea1ea138, 0x19df, 0x11d7, 0xa6, 0xd2, 0x00, 0x06, 0x5b, 0x84, 0x43, 0x5c)
#endif  // __cplusplus && !CINTERFACE

#endif  // _WRT_MSCTF_SUPPLEMENT_
