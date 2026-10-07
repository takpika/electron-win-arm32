// win-arm32 real-header supplement: the Windows SDK's _USE_MATH_DEFINES contract.
//
// The Windows SDK's <math.h> (xwin sdk/include/ucrt/math.h:989-1009) defines M_PI and the
// other M_* constants AFTER its include guard (#endif // _INC_MATH at :985), under
// `#if defined _USE_MATH_DEFINES && !defined _MATH_DEFINES_DEFINED`, so a file that defines
// _USE_MATH_DEFINES and includes <math.h> gets them even when <math.h> was already included
// without it. Windows code relies on that (third_party/s2cellid/src/s2/s1angle.h:19-26
// defines _USE_MATH_DEFINES shortly before its own #include <math.h>). llvm-mingw's <math.h>
// defines them inside its include guard, so a second inclusion yields nothing.
//
// This header (no include guard, so every inclusion re-evaluates it, as the SDK's block is)
// pulls llvm-mingw's real <math.h> and then appends the SDK block VERBATIM, except that it
// is also skipped when M_PI is already defined -- i.e. when llvm-mingw's own block (same
// constants) was taken.
#include_next <math.h>

#if defined _USE_MATH_DEFINES && !defined _MATH_DEFINES_DEFINED && !defined M_PI
    #define _MATH_DEFINES_DEFINED
    // Definitions of useful mathematical constants
    //
    // Define _USE_MATH_DEFINES before including <math.h> to expose these macro
    // definitions for common math constants.  These are placed under an #ifdef
    // since these commonly-defined names are not part of the C or C++ standards
    #define M_E        2.71828182845904523536   // e
    #define M_LOG2E    1.44269504088896340736   // log2(e)
    #define M_LOG10E   0.434294481903251827651  // log10(e)
    #define M_LN2      0.693147180559945309417  // ln(2)
    #define M_LN10     2.30258509299404568402   // ln(10)
    #define M_PI       3.14159265358979323846   // pi
    #define M_PI_2     1.57079632679489661923   // pi/2
    #define M_PI_4     0.785398163397448309616  // pi/4
    #define M_1_PI     0.318309886183790671538  // 1/pi
    #define M_2_PI     0.636619772367581343076  // 2/pi
    #define M_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
    #define M_SQRT2    1.41421356237309504880   // sqrt(2)
    #define M_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)
#endif
