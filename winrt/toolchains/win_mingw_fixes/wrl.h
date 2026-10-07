// win-arm32 real-header wiring: restore the WRL umbrella headers llvm-mingw's <wrl.h> omits.
// The Windows SDK's <wrl.h> pulls wrl/client.h + wrl/implements.h + wrl/module.h + wrl/event.h.
// llvm-mingw's <wrl.h> COMMENTS OUT implements.h and event.h (lines 11,13: `/* #include
// <wrl/implements.h> */`) because mingw ships neither, so any TU that includes <wrl.h> and uses
// Microsoft::WRL::RuntimeClass / RuntimeClassFlags / RuntimeClassType / Callback fails
// "unknown template name 'RuntimeClass'" (16+ media_foundation TUs, e.g.
// media/renderers/win/media_foundation_stream_wrapper.h:43). <wrl/implements.h> and
// <wrl/event.h> DO resolve -- to the real SDK copies (xwin winrt/wrl/) -- once actually
// included; the port's wrl/client.h + wrappers/corewrappers.h forwarders satisfy what they need
// from mingw. This -isystem #1 shim pulls mingw's real <wrl.h> then adds exactly the two headers
// mingw commented out, matching the SDK umbrella.
#ifndef _WRT_WRL_UMBRELLA_SHIM_
#define _WRT_WRL_UMBRELLA_SHIM_
#include_next <wrl.h>
#include <wrl/implements.h>
#include <wrl/event.h>
#endif  // _WRT_WRL_UMBRELLA_SHIM_
