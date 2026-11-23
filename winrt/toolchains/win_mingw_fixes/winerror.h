// win-arm32 real-header supplement: WinRT async HRESULT facility codes.
//
// llvm-mingw's <winerror.h> stops short of the WinRT async-operation HRESULTs that the
// Windows SDK defines (see the pinned xwin shared/winerror.h:30617/30716/30725). WRL's
// <wrl/async.h> (SDK-spelled, pulled from the xwin gap-fill because mingw ships no
// wrl/async.h) uses E_ILLEGAL_STATE_CHANGE, E_ILLEGAL_DELEGATE_ASSIGNMENT and
// E_ASYNC_OPERATION_NOT_STARTED in AsyncBase's state machine, so they must exist.
//
// This -isystem #1 shim pulls mingw's real <winerror.h> and appends ONLY those three
// missing #defines, VERBATIM from the Windows SDK (identical HRESULT values). Guarded so
// this is a pure superset -- if a future mingw defines them, the guards no-op.
#ifndef _WRT_WINERROR_ASYNC_SUPPLEMENT_
#define _WRT_WINERROR_ASYNC_SUPPLEMENT_

#include_next <winerror.h>

#ifndef E_CHANGED_STATE
#define E_CHANGED_STATE               _HRESULT_TYPEDEF_(0x8000000CL)
#endif
#ifndef E_ILLEGAL_STATE_CHANGE
#define E_ILLEGAL_STATE_CHANGE        _HRESULT_TYPEDEF_(0x8000000DL)
#endif
#ifndef E_ILLEGAL_DELEGATE_ASSIGNMENT
#define E_ILLEGAL_DELEGATE_ASSIGNMENT _HRESULT_TYPEDEF_(0x80000018L)
#endif
#ifndef E_ASYNC_OPERATION_NOT_STARTED
#define E_ASYNC_OPERATION_NOT_STARTED _HRESULT_TYPEDEF_(0x80000019L)
#endif
#ifndef E_ILLEGAL_METHOD_CALL
#define E_ILLEGAL_METHOD_CALL         _HRESULT_TYPEDEF_(0x8000000EL)
#endif
#ifndef JSCRIPT_E_CANTEXECUTE
#define JSCRIPT_E_CANTEXECUTE         _HRESULT_TYPEDEF_(0x89020001L)
#endif
// DirectComposition HRESULT the SDK <winerror.h>:61920 defines and llvm-mingw omits;
// gpu/command_buffer/service/shared_image/dcomp_surface_image_backing.cc:255 DCHECK_NE's against it.
#ifndef DCOMPOSITION_ERROR_SURFACE_BEING_RENDERED
#define DCOMPOSITION_ERROR_SURFACE_BEING_RENDERED _HRESULT_TYPEDEF_(0x88980801L)
#endif

#endif  // _WRT_WINERROR_ASYNC_SUPPLEMENT_
