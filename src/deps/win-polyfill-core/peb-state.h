#pragma once

#include "nt.h"

#include "once.h"

struct WinPolyfillPebState
{
    alignas(4096) PVOID Reserved[8];
    volatile ULONG_PTR WaitOnAddressHashTable[128];
    // These members shared with win-polyfill-peb-state.dll
    RTL_CRITICAL_SECTION PebLock;
    SRWLOCK CompareObjectHandlesSRWLock;
    HANDLE GlobalKeyedEventHandle;

    DWORD SystemVersion;
    ULONG DefaultSearchFlags;
    ULONG DllSafeMode;
    UNICODE_STRING DllDirectory; /* extra path for LdrSetDllDirectory */

    DWORD LdrDataTableEntrySize;
    DWORD LdrDdagNodeSize;
    PVOID LdrpHeap;
    PLIST_ENTRY LdrpHashTable;
    PRTL_RB_TREE LdrpModuleBaseAddressIndex;
    PRTL_RB_TREE LdrpMappingInfoIndex;
    PVOID FlsHeap;
};

extern "C" WinPolyfillPebState *__win_polyfill_peb_state_alloc(void *);

#ifdef __cplusplus
namespace
{
    typedef OnceContext<WinPolyfillPebState *, void *> PebStateOnce;
    __forceinline WinPolyfillPebState *
    PebStateGet(PebStateOnce::AllocFunction alloc_func = __win_polyfill_peb_state_alloc)
    {
        static uintptr_t ptr = 0;
        return PebStateOnce::Alloc(
            (WinPolyfillPebState **)&ptr, alloc_func, (void *)&ptr);
    }

} // namespace
#endif
