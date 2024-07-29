#pragma once

#define _LCRT_STRINGIZE_(x) #x
#define _LCRT_STRINGIZE(x) _LCRT_STRINGIZE_(x)

#define DO_PRAGMA(X) _Pragma(#X)
#if defined(__clang__)
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("clang diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("clang diagnostic pop")
#define PRAGMA_DIAGNOSTIC_ERROR(X) DO_PRAGMA(clang diagnostic error X)
#define PRAGMA_DIAGNOSTIC_WARNING(X) DO_PRAGMA(clang diagnostic warning X)
#define PRAGMA_DIAGNOSTIC_IGNORED(X) DO_PRAGMA(clang diagnostic ignored X)
#elif defined(__GNUC__)
#define PRAGMA_DIAGNOSTIC_PUSH _Pragma("GCC diagnostic push")
#define PRAGMA_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#define PRAGMA_DIAGNOSTIC_ERROR(X) DO_PRAGMA(GCC diagnostic error X)
#define PRAGMA_DIAGNOSTIC_WARNING(X) DO_PRAGMA(GCC diagnostic warning X)
#define PRAGMA_DIAGNOSTIC_IGNORED(X) DO_PRAGMA(GCC diagnostic ignored X)
#else
#define PRAGMA_DIAGNOSTIC_PUSH
#define PRAGMA_DIAGNOSTIC_POP
#define PRAGMA_DIAGNOSTIC_ERROR(X)
#define PRAGMA_DIAGNOSTIC_WARNING(X)
#define PRAGMA_DIAGNOSTIC_IGNORED(X)
#endif

PRAGMA_DIAGNOSTIC_IGNORED("-Wc++11-narrowing")
PRAGMA_DIAGNOSTIC_IGNORED("-Wdeprecated-declarations")
PRAGMA_DIAGNOSTIC_IGNORED("-Wdeprecated-enum-compare")
PRAGMA_DIAGNOSTIC_IGNORED("-Wignored-pragmas")
PRAGMA_DIAGNOSTIC_IGNORED("-Winconsistent-missing-override")
PRAGMA_DIAGNOSTIC_IGNORED("-Wint-to-void-pointer-cast")
PRAGMA_DIAGNOSTIC_IGNORED("-Winvalid-constexpr")
PRAGMA_DIAGNOSTIC_IGNORED("-Winvalid-noreturn")
PRAGMA_DIAGNOSTIC_IGNORED("-Wmicrosoft-cast")
PRAGMA_DIAGNOSTIC_IGNORED("-Wparentheses")
PRAGMA_DIAGNOSTIC_IGNORED("-Wswitch")
PRAGMA_DIAGNOSTIC_IGNORED("-Wvoid-pointer-to-int-cast")
PRAGMA_DIAGNOSTIC_IGNORED("-Winvalid-token-paste")
PRAGMA_DIAGNOSTIC_IGNORED("-Wmicrosoft-include")
PRAGMA_DIAGNOSTIC_IGNORED("-Warray-bounds")
PRAGMA_DIAGNOSTIC_IGNORED("-Winvalid-pp-token")

PRAGMA_DIAGNOSTIC_IGNORED("-Wunknown-warning-option")
PRAGMA_DIAGNOSTIC_IGNORED("-Wmicrosoft-string-literal-from-predefined")

// The _VCRT_DECLARE_ALTERNATE_NAME macro provides an architecture-neutral way
// of specifying /alternatename comments to the linker.  It prepends the leading
// decoration character for x86 and hybrid and leaves names unmodified for other
// architectures.
#if defined _M_IX86
#if defined _M_HYBRID
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX "#"
#else
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX "_"
#endif
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX_DATA "_"
#elif defined _M_ARM64EC
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX "#"
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX_DATA ""
#elif defined _M_X64 || defined _M_ARM || defined _M_ARM64
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX ""
#define _VCRT_DECLARE_ALTERNATE_NAME_PREFIX_DATA ""
#else
#error Unsupported architecture
#endif

#if defined _M_ARM64EC

#define _VCRT_DECLARE_ALTERNATE_NAME(name, alternate_name)                               \
    __pragma(comment(                                                                    \
        linker,                                                                          \
        "/alternatename:" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #name                      \
        "=" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #alternate_name))

#else

#define _VCRT_DECLARE_ALTERNATE_NAME(name, alternate_name)                               \
    __pragma(comment(                                                                    \
        linker,                                                                          \
        "/alternatename:" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #name                      \
        "=" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX #alternate_name))

#endif

#define _VCRT_DECLARE_ALTERNATE_NAME_DATA(name, alternate_name)                          \
    __pragma(comment(                                                                    \
        linker,                                                                          \
        "/alternatename:" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX_DATA #name                 \
        "=" _VCRT_DECLARE_ALTERNATE_NAME_PREFIX_DATA #alternate_name))

#define _VCRT_PRAGMA_EXPORT_ALIAS(a, b) __pragma(comment(linker, "/EXPORT:" a "=" b ""))
#define _VCRT_PRAGMA_EXPORT(a) __pragma(comment(linker, "/EXPORT:" a))

#if defined _M_IX86
#define _CRT_LINKER_SYMBOL_PREFIX "_"
#elif defined _M_X64 || defined _M_ARM || defined _M_ARM64
#define _CRT_LINKER_SYMBOL_PREFIX ""
#else
#error Unsupported architecture
#endif

#define _CRT_LINKER_FORCE_INCLUDE(name)                                                  \
    __pragma(comment(linker, "/include:" _CRT_LINKER_SYMBOL_PREFIX #name))

#define _CRTALLOC(x) __declspec(allocate(x))

// X86
// COMDAT; sym= __imp__CoGetApartmentType@8
// COMDAT; sym= __imp_CoGetApartmentType
// COMDAT; sym= _CoGetApartmentType@8

// X64
// COMDAT; sym= CoGetApartmentType
// COMDAT; sym= __imp_CoGetApartmentType

#define EXTERN_C_LCRT                                                                    \
    __pragma(warning(disable : 4483)) extern "C" __declspec(selectany) void const *const

#if defined(_M_IX86)
// x86的符号存在@ 我们使用 __identifier 特性解决
#define _LCRT_DEFINE_IAT_SYMBOL(_FUNCTION, _SIZE)                                        \
    EXTERN_C_LCRT __identifier(_LCRT_STRINGIZE(_imp__##_FUNCTION##@##_SIZE)) =           \
        reinterpret_cast<void const *>(wp_##_FUNCTION);                                  \
    EXTERN_C_LCRT __identifier(_LCRT_STRINGIZE(_imp_##_FUNCTION)) =                      \
        reinterpret_cast<void const *>(wp_##_FUNCTION);                                  \
    EXTERN_C_LCRT __identifier(_LCRT_STRINGIZE(_FUNCTION##@##_SIZE)) =                   \
        reinterpret_cast<void const *>(wp_##_FUNCTION);
#else
#define _LCRT_DEFINE_IAT_SYMBOL(_FUNCTION, _SIZE)                                        \
    EXTERN_C_LCRT __identifier(_LCRT_STRINGIZE(_FUNCTION)) =                             \
        reinterpret_cast<void const *>(wp_##_FUNCTION);                                  \
    EXTERN_C_LCRT __identifier(_LCRT_STRINGIZE(__imp_##_FUNCTION)) =                     \
        reinterpret_cast<void const *>(wp_##_FUNCTION);
#endif

#define _CRT_PARAMS_FALLBACK0 void
#define _CRT_PARAMS_FALLBACK4 int a0
#define _CRT_PARAMS_FALLBACK8 _CRT_PARAMS_FALLBACK4, int a1
#define _CRT_PARAMS_FALLBACK12 _CRT_PARAMS_FALLBACK8, int a2
#define _CRT_PARAMS_FALLBACK16 _CRT_PARAMS_FALLBACK12, int a3
#define _CRT_PARAMS_FALLBACK20 _CRT_PARAMS_FALLBACK16, int a4
#define _CRT_PARAMS_FALLBACK24 _CRT_PARAMS_FALLBACK20, int a5
#define _CRT_PARAMS_FALLBACK28 _CRT_PARAMS_FALLBACK24, int a6
#define _CRT_PARAMS_FALLBACK32 _CRT_PARAMS_FALLBACK28, int a7
#define _CRT_PARAMS_FALLBACK36 _CRT_PARAMS_FALLBACK32, int a8
#define _CRT_PARAMS_FALLBACK40 _CRT_PARAMS_FALLBACK36, int a9
#define _CRT_PARAMS_FALLBACK44 _CRT_PARAMS_FALLBACK40, int a10
#define _CRT_PARAMS_FALLBACK48 _CRT_PARAMS_FALLBACK44, int a11
#define _CRT_PARAMS_FALLBACK52 _CRT_PARAMS_FALLBACK48, int a12
#define _CRT_PARAMS_FALLBACK56 _CRT_PARAMS_FALLBACK52, int a13
#define _CRT_PARAMS_FALLBACK60 _CRT_PARAMS_FALLBACK56, int a14
#define _CRT_PARAMS_FALLBACK64 _CRT_PARAMS_FALLBACK60, int a15
#define _CRT_PARAMS_FALLBACK68 _CRT_PARAMS_FALLBACK64, int a16

#define _CRT_PARAMS_EXPAND(SIZE) _CRT_PARAMS_FALLBACK##SIZE
