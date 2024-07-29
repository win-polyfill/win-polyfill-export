#pragma once

#include <stdint.h>

#include "nt.h"

#include "atomic.h"

#ifdef __cplusplus
namespace
{
    enum WinPolyfillOnce
    {
        OnceInit,
        OnceLoading,
        OnceFinished
    };

    typedef void (*call_once_context_func)(void *context);

    static __forceinline void
    call_once_context(long *flag, call_once_context_func func, void *context) noexcept
    {
        long onceCurrent = InterlockedCompareExchange(
            (volatile long *)flag,
            (long)WinPolyfillOnce::OnceLoading,
            (long)WinPolyfillOnce::OnceInit);
        switch (onceCurrent)
        {
        case WinPolyfillOnce::OnceInit:
            if (func)
                (func)(context);
            InterlockedExchange(
                (volatile long *)flag, (long)WinPolyfillOnce::OnceFinished);
            break;
        case WinPolyfillOnce::OnceLoading:
            do
            {
                // busy loop!
                YieldProcessor();
                onceCurrent =
                    InterlockedCompareExchange((volatile long *)flag, (long)0, (long)0);
            } while (onceCurrent == WinPolyfillOnce::OnceLoading);
            break;
        case WinPolyfillOnce::OnceFinished:
            // done
            break;
        default:
            break;
        }
    }

    constexpr uintptr_t WinPolyfillAllocInit = (uintptr_t)0;
    constexpr uintptr_t WinPolyfillAllocFreed = (uintptr_t)(UINTPTR_MAX - 2);
    constexpr uintptr_t WinPolyfillAllocOngoing = (uintptr_t)(UINTPTR_MAX - 1);
    constexpr uintptr_t WinPolyfillAllocFailed = (uintptr_t)(UINTPTR_MAX);

    template <typename T, typename C> struct OnceContext
    {
        typedef T Type;
        typedef T *Ptr;
        typedef Type (*AllocFunction)(C context);

        static AllocFunction ForFree()
        {
            return reinterpret_cast<AllocFunction>(WinPolyfillAllocFreed);
        }

        static AllocFunction ForGet()
        {
            return reinterpret_cast<AllocFunction>(WinPolyfillAllocInit);
        }

        static void Store(Type volatile *ptr, Type allocated)
        {
            __crt_interlocked_compare_exchange_pointer(
                ptr, allocated, WinPolyfillAllocOngoing);
        }

        /**
         * @brief
         * @param ptr The pointer used to storage the
         * @param alloc_func Used for allocating(handle or memory),
         *                   if it is ForFree(), then means allocating for free
         *                   if it is ForGet(), then means for getting the value
         * @param context The parameter used for alloc_func, can be size_t or anything other
         */
        static __forceinline Type
        Alloc(Type *ptr_input, AllocFunction alloc_func, C context = nullptr)
        {
            void *volatile *ptr = (void *volatile *)ptr_input;
            uintptr_t allocated;
            if (alloc_func == ForGet())
            {
                // Just getting the value, also do not change the state
                allocated = (uintptr_t)__crt_interlocked_read_pointer(ptr);
            }
            else
            {
                allocated = (uintptr_t)__crt_interlocked_compare_exchange_pointer(
                    ptr, WinPolyfillAllocOngoing, WinPolyfillAllocInit);
                switch (allocated)
                {
                case WinPolyfillAllocInit:
                {
                    if (alloc_func == ForFree())
                    {
                        allocated = WinPolyfillAllocFreed;
                    }
                    else
                    {
                        allocated = (uintptr_t)alloc_func(context);
                        if (allocated == WinPolyfillAllocInit)
                        {
                            allocated = WinPolyfillAllocFailed;
                        }
                        else
                        {
                            /* Alloc succeed */
                        }
                    }
                    Store(ptr_input, (Type)allocated);
                    break;
                }
                case WinPolyfillAllocOngoing:
                {
                    do
                    {
                        // busy loop!
                        YieldProcessor();
                        allocated = (uintptr_t)__crt_interlocked_read_pointer(ptr);
                    } while (allocated == WinPolyfillAllocOngoing);
                    break;
                }
                case WinPolyfillAllocFreed:
                    break;
                case WinPolyfillAllocFailed:
                    break;
                default:
                    if (alloc_func == ForFree())
                    {
                        // normal pointer, and the function pointer is tell us for free
                        // then save the freed state into it
                        // this is for not freed twice
                        allocated = (uintptr_t)__crt_interlocked_compare_exchange_pointer(
                            ptr, WinPolyfillAllocFreed, allocated);
                    }
                    break;
                }
            }

            /* Freed, Failed or Done*/
            if (allocated >= WinPolyfillAllocFreed)
            {
                return nullptr;
            }
            return (Type)allocated;
        }

        typedef Type (*AllocSizeFunction)(size_t sz);
        static __forceinline Type
        AllocSize(Type *ptr_input, AllocSizeFunction alloc_func, size_t sz)
        {
            return Alloc(
                ptr_input, (AllocFunction)alloc_func, reinterpret_cast<C>((uintptr_t)sz));
        }

        static Type AllocSizeZero(size_t sz) { return (Type)Alloc(sz, HEAP_ZERO_MEMORY); }

        static Type AllocSizeNormal(size_t sz) { return (Type)Alloc(sz, 0); }
    };

} // namespace
#endif
