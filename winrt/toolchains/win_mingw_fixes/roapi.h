// win-arm32 real-header supplement: WinRT activation-factory registration helpers.
// The Windows SDK's <roapi.h> (winrt/roapi.h:193-205) provides, in namespace
// Windows::Foundation,
//   __inline HRESULT RegisterActivationFactories(HSTRING*, PFNGETACTIVATIONFACTORY*,
//                                                UINT32, RO_REGISTRATION_COOKIE*)
//   __inline void RevokeActivationFactories(RO_REGISTRATION_COOKIE)
// thin wrappers over RoRegisterActivationFactories / RoRevokeActivationFactories.
// llvm-mingw's <roapi.h> declares those Ro* functions and the same namespace
// (Initialize/Uninitialize/GetActivationFactory) but not these two, and the real WRL
// <wrl/module.h> (win_sdk_supplement, needed by chrome/elevation_service and
// chrome/notification_helper) calls them. The SDK's same namespace also provides
//   template<class T> HRESULT ActivateInstance(HSTRING, T**)   (winrt/roapi.h:178-199)
// which the real WRL <wrl/client.h>:932-937 (ComPtrRef overload) forwards to and
// electron/shell/browser/notifications/win/windows_toast_notification.cc calls;
// mingw's <roapi.h> declares RoActivateInstance but not this template. This -isystem
// #1 shim pulls mingw's real <roapi.h> and adds ONLY those three helpers with the
// SDK's bodies.
#ifndef _WRT_ROAPI_SUPPLEMENT_
#define _WRT_ROAPI_SUPPLEMENT_
#include_next <roapi.h>
#if defined(__cplusplus)
namespace Windows {
namespace Foundation {
template <class T>
__inline HRESULT ActivateInstance(HSTRING activatableClassId, T** instance) {
  *instance = nullptr;

  IInspectable* pInspectable;
  HRESULT hr = RoActivateInstance(activatableClassId, &pInspectable);
  if (SUCCEEDED(hr)) {
    if (__uuidof(T) == __uuidof(IInspectable)) {
      *instance = static_cast<T*>(pInspectable);
    } else {
      hr = pInspectable->QueryInterface(IID_PPV_ARGS(instance));
      pInspectable->Release();
    }
  }
  return hr;
}
__inline HRESULT RegisterActivationFactories(
    HSTRING* activatableClassIds,
    PFNGETACTIVATIONFACTORY* activationFactoryCallbacks,
    UINT32 count,
    RO_REGISTRATION_COOKIE* cookie) {
  return RoRegisterActivationFactories(activatableClassIds,
                                       activationFactoryCallbacks, count, cookie);
}
__inline void RevokeActivationFactories(RO_REGISTRATION_COOKIE cookie) {
  RoRevokeActivationFactories(cookie);
}
}  // namespace Foundation
}  // namespace Windows
#endif
#endif  // _WRT_ROAPI_SUPPLEMENT_
