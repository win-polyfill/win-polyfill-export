#include "gen-exports-api.h"

#include "gen-exports.h"

#include <type_traits>
#include <typeinfo>

#include "win32_api_pop.h"

#if defined(__GNUC__) || defined(__clang__)
#define CC_CDECL __attribute__((cdecl))
#define CC_STDCALL __attribute__((stdcall))
#define CC_FASTCALL __attribute__((fastcall))
#define CC_VECTORCALL __attribute__((vectorcall))
#if defined(__INTEL_COMPILER)
#define CC_REGCALL __attribute__((regcall))
#else
#define CC_REGCALL CC_CDECL
#endif
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define CC_CDECL __cdecl
#define CC_STDCALL __stdcall
#define CC_FASTCALL __fastcall
#define CC_VECTORCALL __vectorcall
#if defined(__INTEL_COMPILER)
#define CC_REGCALL __regcall
#else
#define CC_REGCALL CC_CDECL
#endif
#else
#define CC_CDECL
#define CC_STDCALL
#define CC_FASTCALL
#define CC_VECTORCALL
#define CC_REGCALL
#endif

struct CallingConventions
{
    using Cdecl = void(CC_CDECL *)();
    using Stdcall = void(CC_STDCALL *)();
    using Fastcall = void(CC_FASTCALL *)();
    using Vectorcall = void(CC_VECTORCALL *)();
    using Regcall = void(CC_REGCALL *)();
};


template <typename R, typename... Args> struct FunctionTraitsBase
{
    using RetType = R;
    static constexpr size_t ArgCount = sizeof...(Args);
};

template <typename F, int = 0> struct FunctionTraits;

template <typename R, typename... Args>
struct FunctionTraits<R(CC_CDECL *)(Args...), 0> : FunctionTraitsBase<R, Args...>
{
    using Pointer = R(CC_CDECL *)(Args...);
    using PointerClean = R (*)(Args...);
    using CallingConvention = CallingConventions::Cdecl;
    static constexpr int CallingConventionId = 0;
};

template <typename R, typename... Args>
struct FunctionTraits<
    R(CC_STDCALL *)(Args...),
    std::is_same_v<CallingConventions::Cdecl, CallingConventions::Stdcall> ? 1 : 0>
    : FunctionTraitsBase<R, Args...>
{
    using Pointer = R(CC_STDCALL *)(Args...);
    using PointerClean = R (*)(Args...);
    using CallingConvention = CallingConventions::Stdcall;
    static constexpr int CallingConventionId = 1;
};

#if defined(_X86_)
template <typename R, typename... Args>
struct FunctionTraits<
    R(CC_FASTCALL *)(Args...),
    std::is_same_v<CallingConventions::Cdecl, CallingConventions::Fastcall> ? 1 : 0>
    : FunctionTraitsBase<R, Args...>
{
    using Pointer = R(CC_FASTCALL *)(Args...);
    using PointerClean = R (*)(Args...);
    using CallingConvention = CallingConventions::Fastcall;
    static constexpr int CallingConventionId = 2;
};
#endif

template <typename T> struct FunctionTraits<T, 0>
{
    static constexpr int CallingConventionId = 3;
};

template <typename T>
std::enable_if_t<std::is_void_v<T>> CreateArgTypeInfo(ArtTypeInfo &info)
{
    info.function_id = -1;
    info.size = 0;
    info.alignment = 0;
    info.name = typeid(T).name();
    info.raw_name = typeid(T).raw_name();
}

template <typename T>
std::enable_if_t<std::is_pointer_v<T>> CreateArgTypeInfo(ArtTypeInfo &info)
{
    info.is_pointer = true;
    info.function_id = FunctionTraits<T>::CallingConventionId;
    info.size = sizeof(T);
    info.alignment = alignof(T);
    info.name = typeid(T).name();
    info.raw_name = typeid(T).raw_name();
}

template <typename T>
std::enable_if_t<!std::is_pointer_v<T> && !std::is_void_v<T>>
CreateArgTypeInfo(ArtTypeInfo &info)
{
    info.function_id = 4;
    info.size = sizeof(T);
    info.alignment = alignof(T);
    info.name = typeid(T).name();
    info.raw_name = typeid(T).raw_name();
}

template <typename Last> void type_name(FunctionInfo *info)
{
    ArtTypeInfo typeInfo;
    CreateArgTypeInfo<Last>(typeInfo);
    function_info_add_parameter(info, &typeInfo);
}

template <typename First, typename Second, typename... Rest>
void type_name(FunctionInfo *info)
{
    ArtTypeInfo typeInfo;
    CreateArgTypeInfo<First>(typeInfo);
    function_info_add_parameter(info, &typeInfo);
    type_name<Second, Rest...>(info);
}

template <typename F> struct TypeName;

template <typename R, typename... Args> struct TypeName<R (*)(Args...)>
{
    static void
    init(const char *module_name, const char *function_name, int CallingConventionId)
    {
        FunctionInfo *info = function_info_alloca(module_name, function_name, CallingConventionId);
        type_name<R, Args...>(info);
    }
};

#if 0
void gen_functions()
{
#define DEFINE_THUNK(Module, Function) typedef decltype(&Function) Function##Type;
#include "apis-gen/ntdll-export-declared.h"
#undef DEFINE_THUNK
}
#else

void gen_functions()
{
#define DEFINE_THUNK_DATA(Module, Function)
#define DEFINE_THUNK_VAARG(Module, Function)

#if 1
#define DEFINE_THUNK(Module, Function)                                                   \
    {                                                                                    \
        typedef FunctionTraits<decltype(&Function)> Traits;                              \
        TypeName<Traits::PointerClean>::init(                                            \
            _LCRT_STRINGIZE(Module),                                                     \
            _LCRT_STRINGIZE(Function),                                                   \
            Traits::CallingConventionId);                                                \
    }
#else
#define DEFINE_THUNK(Module, Function)                                                   \
    {                                                                                    \
        typedef FunctionTraits<decltype(::Function)> Traits;                             \
    }
#endif

#include "ntdll_full.h"

#include "kernel32_full.h"

#include "advapi32_full.h"
#include "cfgmgr32_full.h"
#include "crypt32_full.h"
#include "dbghelp_full.h"
#include "esent_full.h"
#include "gdi32_full.h"
#include "iphlpapi_full.h"
#include "netapi32_full.h"
#include "ole32_full.h"
#include "pdh_full.h"
#include "powrprof_full.h"
#include "psapi_full.h"
#include "setupapi_full.h"
#include "shell32_full.h"
#include "shlwapi_full.h"
#include "user32_full.h"
#include "userenv_full.h"
#include "version_full.h"
#include "winhttp_full.h"
#include "ws2_32_full.h"


// Not present on Windows 2000
#include "bcrypt_full.h"
#include "bcryptprimitives_full.h"
#include "bluetoothapis_full.h"
#include "d3d11_full.h"
#include "d3d12_full.h"
#include "d3d9_full.h"
#include "dwmapi_full.h"
#include "dwrite_full.h"
#include "dxgi_full.h"
#include "dxva2_full.h"
#include "mf_full.h"
#include "mfplat_full.h"
#include "mfreadwrite_full.h"
#include "ncrypt_full.h"
#include "ndfapi_full.h"
#include "propsys_full.h"
#include "shcore_full.h"
#include "uiautomationcore_full.h"
#include "uxtheme_full.h"
#include "wevtapi_full.h"
#include "winusb_full.h"
#include "zipfldr_full.h"

#undef DEFINE_THUNK
}

#endif

#if 0
void test()
{
    typedef decltype(&LdrLockLoaderLock) LdrLockLoaderLockType;
    typedef decltype(&RtlHashUnicodeString) RtlHashUnicodeStringType;
    typedef decltype(&test) TestType;
    constexpr int a = FunctionTraits<TestType>::ArgCount;
    constexpr int y = FunctionTraits<RtlHashUnicodeStringType>::ArgCount;
    static_assert(FunctionTraits<TestType>::CallingConventionId == 0, "");
    static_assert(FunctionTraits<RtlHashUnicodeStringType>::CallingConventionId == 1, "");
    FunctionTraits<RtlHashUnicodeStringType>::NthArg<0> z;
    static_assert(y == 4, "!");
}
#endif
