// win-arm32 real-header wiring: supply the DataReader family llvm-mingw's
// windows.storage.streams.h omits. mingw ships the DataWriter side but NONE of IDataReader /
// IDataReaderFactory / IDataReaderStatics / runtimeclass DataReader / DataReaderLoadOperation
// (grep IDataReader = 0), so device/bluetooth/bluetooth_adapter_winrt.cc fails "no member named
// IDataReader in namespace Streams". This -isystem #1 shim pulls mingw's real header UNCHANGED
// (include_next -> keeps every mingw interface, enum, #define and _ENUM_DEFINED__ guard) and
// then the port-owned extension (win_sdk_supplement, widl-generated from the pinned xwin idl)
// that supplies ONLY the missing DataReader family; the extension re-#includes this header for
// the shared IBuffer/IInputStream/foundation types (guarded no-op), never redefining them.
// Additive form, matching the ledevice_ext / genericattributeprofile_ext pattern -- no full
// shadow, so mingw's 14 existing Streams interfaces are not re-emitted or altered.
#ifndef _WRT_STORAGE_STREAMS_DATAREADER_EXT_SHIM_
#define _WRT_STORAGE_STREAMS_DATAREADER_EXT_SHIM_

#include_next <windows.storage.streams.h>
#include <windows.storage.streams_ext.h>

#endif  // _WRT_STORAGE_STREAMS_DATAREADER_EXT_SHIM_
