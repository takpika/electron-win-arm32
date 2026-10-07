// win-arm32 (config-fix, port-owned toolchain shim). base/win/vector.h and its
// consumers use the Windows-SDK spelling VectorChangedEventHandler<T> (: Vector-
// ChangedEventHandler_impl<T>) for this parameterized delegate, both as an override
// parameter against IObservableVector<T>::add_VectorChanged and as an explicit
// __declspec(uuid(...)) specialization. mingw's windows.foundation.collections.h
// (first on -isystem) names the identical interface IVectorChangedEventHandler<T>
// (: IVectorChangedEventHandler_impl<T>, line 864; add_VectorChanged at 954), so the
// SDK spelling is absent. The two spellings must be ONE type: identity is required
// for the override to bind, and an explicitly-specializable primary template is
// required for the uuid sites — a `using` alias meets the first but cannot be
// explicitly specialized; a distinct derived struct meets the second but is a
// different type. Preprocessor name identity (below, AFTER include_next so mingw's
// own declarations are untouched) is the only construct that is both.
//
// HEADER-ORDERING ALTERNATIVE — EVALUATED AND REJECTED. The SDK-spelled family also
// ships in-tree at toolchains/xwin-sdk/sdk/include/winrt/windows.foundation.collections.h
// (VectorChangedEventHandler/_impl + IObservableVector/IVector/IVectorView WITH a
// detail::not_yet_specialized<> guard). Ordering that winrt dir ahead of mingw for this
// family was tested and does
// NOT work: the SDK collections header is incompatible with the mingw WinRT family the
// port is built on — it uses ABI::Windows::Foundation::AsyncStatus (an SDK enum-class form)
// where mingw has a C typedef enum, and defines MapChangedEventHandler while mingw's
// windows.foundation.h needs IMapChangedEventHandler (10 compile errors). Adopting it would
// mean replacing the ENTIRE mingw WinRT family, which the port is not built on. So this
// rename shim is the minimal correct fix, not a placeholder; the not_yet_specialized guard
// it "drops" is an SDK-family feature the mingw family (this port's baseline) never had.
#ifndef _WINRT_FIX_WINDOWS_FOUNDATION_COLLECTIONS_H_
#define _WINRT_FIX_WINDOWS_FOUNDATION_COLLECTIONS_H_
// mingw-w64 misspells IMapChangedEventArgs<K>'s first method: its header (line 706)
// declares get_CollectionChanged, the SDK's windows.foundation.collections.h (:1197) and
// the WinRT metadata name it get_CollectionChange. The vtable slot is the same, only the
// C++ name differs, so base/win/map.h's `get_CollectionChange(...) override` binds to
// nothing. Renaming the token BEFORE include_next makes mingw's declaration carry the
// SDK name (its C-interface members and accessor macros rename consistently).
#define get_CollectionChanged get_CollectionChange
#include_next <windows.foundation.collections.h>
#if defined(__cplusplus) && !defined(CINTERFACE)
#define VectorChangedEventHandler_impl IVectorChangedEventHandler_impl
#define VectorChangedEventHandler      IVectorChangedEventHandler
// The same SDK-vs-mingw spelling difference for the map delegate: base/win/map.h
// (:243, :376) names MapChangedEventHandler<K, V>, mingw names the identical interface
// IMapChangedEventHandler<K, V> (: IMapChangedEventHandler_impl<K, V>, lines 229-232).
#define MapChangedEventHandler_impl    IMapChangedEventHandler_impl
#define MapChangedEventHandler         IMapChangedEventHandler
#endif
#endif  // _WINRT_FIX_WINDOWS_FOUNDATION_COLLECTIONS_H_
