// win-arm32 (config-fix, port-owned toolchain shim). Make MSVC-style
// __declspec(uuid("..."))/__uuidof — which Chromium uses for the hand-specialized
// parameterized WinRT interface IIDs (base/win/vector.h + the IObservableVector/
// IVectorView/VectorChangedEventHandler uuid sites in device/, chrome/webshare/,
// base/win/*_unittest.cc) — actually resolve under the llvm-mingw -gnu triple,
// WITHOUT disturbing mingw's own COM IID mechanism.
//
// DEFECT (in the untouched mingw headers): on the -gnu triple _MSC_VER is undefined,
// so _mingw.h sets USE___UUIDOF==0 and maps `__uuidof(type)` -> `__mingw_uuidof<type>()`.
// mingw's interfaces carry their IID via __CRT_UUID_DECL, which provides EXPLICIT
// specializations of __mingw_uuidof<type>() (mingw's MIDL_INTERFACE(x) is a bare
// `struct` -- rpcndr.h:855 -- with NO __declspec(uuid), so clang's native __uuidof
// cannot be used for mingw types). But the PRIMARY template __mingw_uuidof<T>()
// (guiddef.h:32/34) is declared-but-UNDEFINED. Chromium's WinRT code supplies IIDs
// via __declspec(uuid("...")) on interface template specializations that have NO
// __CRT_UUID_DECL -> those hit the undefined primary -> undefined-symbol link error
// (proven: `U __mingw_uuidof<...IVectorChangedEventHandler<int>>`), and the
// __declspec(uuid) is silently ignored.
//
// FIX: DEFINE the primary __mingw_uuidof<T>() (which mingw leaves undefined) to fall
// back to clang's native __uuidof operator, which reads __declspec(uuid). This is
// purely ADDITIVE and non-invasive: __CRT_UUID_DECL's explicit specializations still
// win for every mingw type (an explicit specialization always outranks the primary),
// so mingw's own IIDs are completely unchanged; the primary is used ONLY for types
// that carry __declspec(uuid) but no __CRT_UUID_DECL -- exactly Chromium's
// hand-specialized WinRT interfaces. The build already enables -fms-extensions, under
// which clang honors __declspec(uuid)/__uuidof. (For a type with neither mechanism the
// primary now yields a clear compile error instead of an undefined-symbol link error --
// strictly better, and only for a genuinely GUID-less type.)
#include_next <guiddef.h>

// Guard our ADDITION (not the mingw header, which has its own guard) so the primary
// template is defined exactly once per TU even though guiddef.h is included many times.
#if defined(__cplusplus) && (USE___UUIDOF == 0) && defined(__clang__) && \
    !defined(_WINRT_FIX_MINGW_UUIDOF_PRIMARY_)
#define _WINRT_FIX_MINGW_UUIDOF_PRIMARY_ 1
extern "C++" {
#pragma push_macro("__uuidof")
#undef __uuidof
#if __cpp_constexpr >= 200704l && __cpp_inline_variables >= 201606L
template<typename T> constexpr const GUID &__mingw_uuidof() { return __uuidof(T); }
#else
template<typename T> const GUID &__mingw_uuidof() { return __uuidof(T); }
#endif
#pragma pop_macro("__uuidof")
}
#endif
