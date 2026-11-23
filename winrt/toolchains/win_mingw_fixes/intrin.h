// win-arm32 config-fix. Pull the real mingw <intrin.h>, then supply the
// MSVC ARM intrinsics/constants that WRL (winrt/wrl/implements.h) needs but mingw's
// <intrin.h> omits on this -gnu target. Verified against the real WRL consumer and the
// real MS headers already in-tree (xwin-sdk), plus -fsyntax-only/-S probes with the
// build's own clang (llvm-mingw-20260505).
#include_next <intrin.h>
#if defined(__arm__)
// (0) __dmb/__dsb/__isb etc. — declare them PROPERLY via clang's own ARM ACLE header
//     (lib/clang/*/include/arm_acle.h). Without this, WRL's __dmb(_ARM_BARRIER_ISH) is a
//     -Wimplicit-function-declaration (an ERROR under -Werror; clang's own note says
//     "include the header <arm_acle.h>"). Real declaration = robust.
#  include <arm_acle.h>
// (1) *NoFence interlocked variants. clang DOES implement MSVC's ARM `_nf` (no-fence)
//     intrinsics natively (verified: `_InterlockedIncrement_nf` &c. lower to bare
//     ldrex/strex with NO dmb on --target=armv7-w64-windows-gnu), but they require a
//     DECLARATION (mingw's <intrin.h> declares neither the `_nf` builtins nor the
//     InterlockedXxxNoFence macros). So DECLARE the three `_nf` builtins the reachable
//     consumer (winrt/wrl/implements.h) uses and map the SDK macros to them — preserving
//     WRL's intended RELAXED ordering (NOT downgrading to a full fence). Signatures match
//     MSVC intrin.h. InterlockedDecrementNoFence is intentionally omitted: it is used only
//     by xwin um/winnt.h, unreachable under the mingw-first order (<winnt.h> -> mingw's).
// FAIL-LOUD guard: the `_nf` builtins exist ONLY under -fms-extensions (build/config/win/
// BUILD.gn:134 enables it). WITHOUT it, clang does not recognize them as builtins, and the
// declarations below would silently turn the InterlockedXxxNoFence calls into references to
// NONEXISTENT symbols `_Interlocked*_nf` (link failure / wrong codegen). Verified:
// `__has_builtin(_InterlockedIncrement_nf)` is 1 with -fms-extensions and 0 without. If the
// flag is ever dropped, break the build here instead of degrading WRL refcounting silently.
#  if !__has_builtin(_InterlockedIncrement_nf)
#    error "win_mingw_fixes/intrin.h: InterlockedXxxNoFence requires clang's MS `_nf` intrinsics, which need -fms-extensions (see build/config/win/BUILD.gn). Without it these map to nonexistent symbols."
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
long  _InterlockedIncrement_nf(long volatile*);
long  _InterlockedCompareExchange_nf(long volatile*, long, long);
void* _InterlockedCompareExchangePointer_nf(void* volatile*, void*, void*);
#  ifdef __cplusplus
}
#  endif
#  ifndef InterlockedIncrementNoFence
#    define InterlockedIncrementNoFence _InterlockedIncrement_nf
#  endif
#  ifndef InterlockedCompareExchangeNoFence
#    define InterlockedCompareExchangeNoFence _InterlockedCompareExchange_nf
#  endif
#  ifndef InterlockedCompareExchangePointerNoFence
#    define InterlockedCompareExchangePointerNoFence _InterlockedCompareExchangePointer_nf
#  endif
// (2) _ARM_BARRIER_ISH: ARM DMB "inner shareable" selector. Value verified byte-exact
//     against the real MS armintr.h already in-tree (xwin-sdk/crt/include/armintr.h:526,
//     `_ARM_BARRIER_ISH = 0xB`; = ARM DMB option ISH 0b1011); that header itself is
//     unusable here (it #errors without _M_ARM), so the single needed constant is
//     supplied directly with the SDK's value.
#  ifndef _ARM_BARRIER_ISH
#    define _ARM_BARRIER_ISH 0xB
#  endif
#endif
