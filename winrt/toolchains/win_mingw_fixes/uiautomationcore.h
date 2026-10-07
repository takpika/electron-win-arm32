// win-arm32 config-fix. Pull the real mingw <uiautomationcore.h>, then supply
// the UI Automation "text-attribute value" enums that Chromium's accessibility layer
// (ui/accessibility/platform/ax_platform_node_win.cc &c., via <uiautomation.h>) uses but
// that llvm-mingw-20260505's <uiautomationcore.h> OMITS. These enums were added to the
// Windows SDK for the Text/TextRange pattern text attributes; mingw's header predates them.
//
// Root cause: mingw's <uiautomationcore.h> is older than the MS SDK and lacks the
// BulletStyle/FlowDirections/HorizontalTextAlignment/TextDecorationLineStyle enums (verified
// MISSING: grep "enum <Name>" in the mingw header = 0). The include order (clang-cl.exe:
// win_mingw_fixes -isystem FIRST, then mingw) makes <uiautomationcore.h> resolve here;
// #include_next reaches mingw's for everything else.
//
// MINIMAL: only the 4 enums Chromium actually references are supplied (scoped grep of
// ui/accessibility, ui/views, content/browser/accessibility, chrome/browser/accessibility:
// BulletStyle_ 8, FlowDirections_ 7, HorizontalTextAlignment_ 6, TextDecorationLineStyle_ 9).
// The SDK's other mingw-missing text-attr enums (AnimationStyle/CapStyle/FillType/
// OutlineStyles/VisualEffects) have 0 Chromium references and are intentionally NOT added
// (same policy as the sibling win_mingw_fixes/uiautomationclient.h).
//
// The definitions below are VERBATIM (values and enumerator names byte-identical) from the
// in-tree real MS SDK header toolchains/xwin-sdk/sdk/include/um/UIAutomationCore.h, so they
// carry the authentic SDK values. No hand-invented constants. This adds only enum TYPES the
// port's headers omit; it does not remove or alter anything mingw already provides.

#include_next <uiautomationcore.h>

#ifndef WRT_UIACORE_TEXTATTR_ENUMS_SUPPLEMENT
#define WRT_UIACORE_TEXTATTR_ENUMS_SUPPLEMENT

enum BulletStyle
    {
        BulletStyle_None	= 0,
        BulletStyle_HollowRoundBullet	= 1,
        BulletStyle_FilledRoundBullet	= 2,
        BulletStyle_HollowSquareBullet	= 3,
        BulletStyle_FilledSquareBullet	= 4,
        BulletStyle_DashBullet	= 5,
        BulletStyle_Other	= -1
    } ;

enum FlowDirections
    {
        FlowDirections_Default	= 0,
        FlowDirections_RightToLeft	= 0x1,
        FlowDirections_BottomToTop	= 0x2,
        FlowDirections_Vertical	= 0x4
    } ;

enum HorizontalTextAlignment
    {
        HorizontalTextAlignment_Left	= 0,
        HorizontalTextAlignment_Centered	= 1,
        HorizontalTextAlignment_Right	= 2,
        HorizontalTextAlignment_Justified	= 3
    } ;

enum TextDecorationLineStyle
    {
        TextDecorationLineStyle_None	= 0,
        TextDecorationLineStyle_Single	= 1,
        TextDecorationLineStyle_WordsOnly	= 2,
        TextDecorationLineStyle_Double	= 3,
        TextDecorationLineStyle_Dot	= 4,
        TextDecorationLineStyle_Dash	= 5,
        TextDecorationLineStyle_DashDot	= 6,
        TextDecorationLineStyle_DashDotDot	= 7,
        TextDecorationLineStyle_Wavy	= 8,
        TextDecorationLineStyle_ThickSingle	= 9,
        TextDecorationLineStyle_DoubleWavy	= 11,
        TextDecorationLineStyle_ThickWavy	= 12,
        TextDecorationLineStyle_LongDash	= 13,
        TextDecorationLineStyle_ThickDash	= 14,
        TextDecorationLineStyle_ThickDashDot	= 15,
        TextDecorationLineStyle_ThickDashDotDot	= 16,
        TextDecorationLineStyle_ThickDot	= 17,
        TextDecorationLineStyle_ThickLongDash	= 18,
        TextDecorationLineStyle_Other	= -1
    } ;

// ProviderOptions IS defined by mingw's header (identical 9 enumerators), but mingw omits
// the SDK's DEFINE_ENUM_FLAG_OPERATORS(ProviderOptions) — so `ProviderOptions_A |
// ProviderOptions_B` yields `int` and Chromium's `*ret = ...|...;` (ax_platform_node_win.cc
// GetProviderOptions) fails "assigning to 'ProviderOptions' from incompatible type 'int'".
// The real SDK (xwin UIAutomationCore.h) applies this macro; supply it here so `|` returns
// ProviderOptions, matching the SDK. The macro itself is mingw's own (winnt.h:722).
DEFINE_ENUM_FLAG_OPERATORS(ProviderOptions)

// Scroll-pattern sentinel: mingw omits it; verbatim value from the real SDK
// (xwin UIAutomationCore.h:957). Used by ax_platform_node_win.cc (scroll pattern).
const double UIA_ScrollPatternNoScroll	=	-1;

#endif  // WRT_UIACORE_TEXTATTR_ENUMS_SUPPLEMENT
