// win-arm32 real-header supplement: I_RpcOpenClientProcess.
// The Windows SDK's <rpcdcep.h> (shared/rpcdcep.h:1245-1254, NTDDI_VISTA and later)
// declares
//   RPCRTAPI RPC_STATUS RPC_ENTRY I_RpcOpenClientProcess(
//       RPC_BINDING_HANDLE Binding, unsigned long DesiredAccess, void** ClientProcess);
// llvm-mingw's <rpcdcep.h> omits the declaration although its librpcrt4.a import
// library exports the function (I_RpcOpenClientProcess / __imp_I_RpcOpenClientProcess),
// so chrome/elevation_service/elevator.cc fails "use of undeclared identifier". This
// -isystem #1 shim pulls mingw's real <rpcdcep.h> and adds ONLY that declaration,
// with the SDK's signature and linkage.
#ifndef _WRT_RPCDCEP_SUPPLEMENT_
#define _WRT_RPCDCEP_SUPPLEMENT_
#include_next <rpcdcep.h>
#ifdef __cplusplus
extern "C" {
#endif
RPCRTAPI RPC_STATUS RPC_ENTRY I_RpcOpenClientProcess(RPC_BINDING_HANDLE Binding,
                                                     unsigned long DesiredAccess,
                                                     void** ClientProcess);
#ifdef __cplusplus
}
#endif
#endif  // _WRT_RPCDCEP_SUPPLEMENT_
