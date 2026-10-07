// win-arm32 (config-fix). Part of the WRL event-path family redirect; see ../client.h for
// the full rationale. mingw ships wrl/wrappers/corewrappers.h with only
// HString/HStringReference/RoInitializeWrapper -- no SRWLock/Event/CriticalSection/Mutex/
// Semaphore that xwin event.h/implements.h require -- and its HString/RoInitializeWrapper
// are mingw's own implementations. Forward to xwin's real MS WRL corewrappers.h so the
// whole event-path family (client/implements/corewrappers/event) is one consistent source.
// HString/RoInitialize lifetime + ref-count are device-verified, not compile-only.
// The SDK's winrt/ directory is reached through its sibling um/ and shared/ search-path
// directories, as in ../client.h.
#include <../winrt/wrl/wrappers/corewrappers.h>
