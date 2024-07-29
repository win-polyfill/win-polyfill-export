#pragma once

#include "nt.h"

#include <intrin.h>

#ifdef __cplusplus
namespace
{
    template <typename T, typename V>
    static __forceinline T *__fastcall __crt_interlocked_exchange_pointer(
        T *const volatile *target,
        V const value) noexcept
    {
        // This is required to silence a spurious unreferenced formal parameter
        // warning.
        UNREFERENCED_PARAMETER(value);

        return reinterpret_cast<T *>(
            _InterlockedExchangePointer((void **)target, (void *)value));
    }

    template <typename T, typename E, typename C>
    static __forceinline T *__fastcall __crt_interlocked_compare_exchange_pointer(
        T *const volatile *target,
        E const exchange,
        C const comparand) noexcept
    {
        UNREFERENCED_PARAMETER(exchange);  // These are required to silence spurious
        UNREFERENCED_PARAMETER(comparand); // unreferenced formal parameter warnings.

        return reinterpret_cast<T *>(_InterlockedCompareExchangePointer(
            (void **)target, (void *)exchange, (void *)comparand));
    }

    static __forceinline __int32 __fastcall __crt_interlocked_read_32(
        __int32 const volatile *target) noexcept
    {
#if defined _M_ARM64 || defined _M_ARM64EC
        __int32 const result =
            __load_acquire32(reinterpret_cast<const volatile unsigned __int32 *>(target));
        return result;
#elif defined _M_IX86 || defined _M_X64
        __int32 const result = *target;
        _ReadWriteBarrier();
        return result;
#elif defined _M_ARM
        __int32 const result =
            __iso_volatile_load32(reinterpret_cast<int const volatile *>(target));
        __crt_interlocked_memory_barrier();
        return result;
#else
#error Unsupported architecture
#endif
    }

#if defined _WIN64
    static __forceinline __int64 __fastcall __crt_interlocked_read_64(
        __int64 const volatile *target) noexcept
    {
#if defined _M_ARM64 || defined _M_ARM64EC
        __int64 const result =
            __load_acquire64(reinterpret_cast<const volatile unsigned __int64 *>(target));
        return result;
#elif defined _M_X64
        __int64 const result = *target;
        _ReadWriteBarrier();
        return result;
#else
#error Unsupported architecture
#endif
    }
#endif // _WIN64

    template <typename T>
    static __forceinline T *__fastcall __crt_interlocked_read_pointer(
        T *const volatile *target) noexcept
    {
#ifdef _WIN64
        return (T *)__crt_interlocked_read_64((__int64 *)target);
#else
        return (T *)__crt_interlocked_read_32((__int32 *)target);
#endif
    }

} // namespace
#endif
