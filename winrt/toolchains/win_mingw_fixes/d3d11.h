// win-arm32 real-header supplement: APP_DEPRECATED_HRESULT.
// The Windows SDK's <d3d11.h> marks the ID3D11VideoDecoder::DecoderExtension /
// ID3D11VideoContext::VideoProcessorSetOutputExtension etc. return type with a deprecation
// typedef `APP_DEPRECATED_HRESULT` (SDK d3d11.h:10024-10028: `typedef HRESULT
// APP_DEPRECATED_HRESULT;`). llvm-mingw's <d3d11.h> ships the same ID3D11VideoDecoder
// interface but returns plain HRESULT and never defines APP_DEPRECATED_HRESULT, so
// media/base/win/d3d11_mocks.h (which mocks those methods with the SDK spelling) fails
// "unexpected type name". This -isystem #1 shim pulls mingw's real <d3d11.h> and adds ONLY
// the missing typedef, guarded byte-for-byte as the SDK guards it (same value: HRESULT).
#ifndef _WRT_D3D11_APP_DEPRECATED_HRESULT_SHIM_
#define _WRT_D3D11_APP_DEPRECATED_HRESULT_SHIM_
#include_next <d3d11.h>
#if !defined(APP_DEPRECATED_HRESULT) && !defined(APP_DEPRECATED_HRESULT_TYPEDEF)
#define APP_DEPRECATED_HRESULT_TYPEDEF
typedef HRESULT APP_DEPRECATED_HRESULT;
#endif
#endif  // _WRT_D3D11_APP_DEPRECATED_HRESULT_SHIM_
