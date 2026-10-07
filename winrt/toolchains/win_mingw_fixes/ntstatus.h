// win-arm32 real-header supplement: STATUS_DEVICE_HARDWARE_ERROR.
// The Windows SDK's <ntstatus.h> (shared/ntstatus.h) defines
//   #define STATUS_DEVICE_HARDWARE_ERROR     ((NTSTATUS)0xC0000483L)
// llvm-mingw's <ntstatus.h> ships the neighbouring device-error codes
// (STATUS_DEVICE_DATA_ERROR, STATUS_IO_DEVICE_ERROR, ...) but not this one, so
// third_party/lzma_sdk/google/seven_zip_reader.cc (which maps in-page errors to
// kIoError) fails "use of undeclared identifier". This -isystem #1 shim pulls
// mingw's real <ntstatus.h> and adds ONLY the missing code, with the SDK's value.
#ifndef _WRT_NTSTATUS_SUPPLEMENT_
#define _WRT_NTSTATUS_SUPPLEMENT_
#include_next <ntstatus.h>
#ifndef STATUS_DEVICE_HARDWARE_ERROR
#define STATUS_DEVICE_HARDWARE_ERROR ((NTSTATUS)0xC0000483L)
#endif
#endif  // _WRT_NTSTATUS_SUPPLEMENT_
