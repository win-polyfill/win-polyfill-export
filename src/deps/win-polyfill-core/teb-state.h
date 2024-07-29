#pragma once

#include "nt.h"

#include "once.h"
#include "version.h"

typedef struct TlsIndexTracked TlsIndexTracked;
struct WinPolyfillTebState
{
    alignas(4096) PVOID Reserved[8];
    char PrintBuffer[1024];
    PTEB Self;
    LIST_ENTRY ThreadLinks;
    RTL_CRITICAL_SECTION TlsFixupLock;
    TlsIndexTracked *TlsIndexTracked;
    PVOID FlsDataBlock;
};

#ifdef __cplusplus
namespace
{

    __forceinline WinPolyfillTebState **WinPolyfillTebStateEntryGet(TEB *pTeb)
    {
#ifndef _WIN64
        auto version = GetSystemVersionCompact();
        if (version < SYSTEM_WIN10_1507)
        {
            return (WinPolyfillTebState **)((uintptr_t)(pTeb) + 0x0FF8);
        }
        else if (version < SYSTEM_WIN11_21H2)
        {
            /* Use 8 byte just before TxFsContext */
            return (WinPolyfillTebState **)((uintptr_t)(pTeb) + 0x01D0 - 0x0008);
        }
#endif
        return (WinPolyfillTebState **)((uintptr_t)(pTeb) + 0x1FF0);
    }

    WinPolyfillTebState *WinPolyfillTebStateAlloc(TEB *context)
    {
        WinPolyfillTebState * state = nullptr;
        SIZE_T dwSize = sizeof(WinPolyfillTebState);
        NTSTATUS st = NtAllocateVirtualMemory(NtCurrentProcess(), (PVOID*)&state,
            NULL, &dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (state)
        {
            RtlInitializeCriticalSection(&state->TlsFixupLock);
            InitializeListHead(&state->ThreadLinks);
            state->Self = context;
        }
        return state;
    }

    typedef OnceContext<WinPolyfillTebState*, TEB*> TebStateOnce;

    inline WinPolyfillTebState *TebStateGet(
        TEB *pTeb,
        TebStateOnce::AllocFunction alloc_func = TebStateOnce::ForGet())
    {
        auto pTebStateEntry = WinPolyfillTebStateEntryGet(pTeb);
        return TebStateOnce::Alloc(pTebStateEntry, alloc_func, pTeb);
    }

} // namespace
#endif
