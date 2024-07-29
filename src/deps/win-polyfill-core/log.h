#pragma once

#include "string.h"
#include "teb-state.h"
#include "version.h"

#if defined(WP_PRINTF_ENABLED)
#include "printf.h"
#endif

namespace
{
#if defined(WP_PRINTF_ENABLED)
    typedef OnceContext<decltype(&WriteFile), decltype(&WriteFile) *> WriteFileOnce;
    WriteFileOnce::Type WriteFileAlloc(WriteFileOnce::Ptr context)
    {
        auto name = MakeNtString(L"kernel32");
        PVOID DllHandle;
        if (NT_SUCCESS(LdrGetDllHandle(nullptr, nullptr, &name, &DllHandle)))
        {
            auto ProcedureName = MakeNtString("WriteFile");
            PVOID pfn;
            if (NT_SUCCESS(LdrGetProcedureAddress(DllHandle, &ProcedureName, 0, &pfn)))
            {
                return (WriteFileOnce::Type)pfn;
            }
        }
        return nullptr;
    }

    int wp_printf(const char *format, ...)
    {
        auto teb = NtCurrentTeb();
        auto peb = teb->ProcessEnvironmentBlock;
        auto pTebState = TebStateGet(teb, WinPolyfillTebStateAlloc);
        static WriteFileOnce::Type pfnWrite;
        auto pfnWriteFinal = WriteFileOnce::Alloc(&pfnWrite, WriteFileAlloc, &pfnWrite);
        int ret = -1;
        if (pTebState != nullptr && pfnWriteFinal)
        {
            va_list args;           //For declaration of list or arguments
            va_start(args, format); //Here you are starting your list from the format
            ret = vsnprintf_(
                pTebState->PrintBuffer, sizeof(pTebState->PrintBuffer), format, args);
            va_end(args);
            if (ret > 0)
            {
                HANDLE hOut = peb->ProcessParameters->StandardOutput;
                DWORD dwCount;
                pfnWriteFinal(hOut, pTebState->PrintBuffer, ret, &dwCount, nullptr);
                RtlFillMemory(pTebState->PrintBuffer, ret, 0);
            }
        }
        return ret;
    }
#else
    int wp_printf(const char *format, ...) { return -1; }
#endif

    unsigned short GetLoadCount(LDR_DATA_TABLE_ENTRY *pLdrEntry)
    {
        if (GetSystemVersionCompact() >= SYSTEM_NT_6_2_RTM)
        {
            return pLdrEntry->DdagNode->LoadCount;
        }
        else
        {
            return pLdrEntry->LoadCount;
        }
    }

    size_t FindModuleHandleInLoadOrderModuleList(
        HANDLE h,
        LDR_DATA_TABLE_ENTRY **pEntries,
        size_t count)
    {
        size_t i = 0;
        size_t pos_found = SIZE_MAX;
        auto pTeb = NtCurrentTeb();
        auto pLdr = pTeb->ProcessEnvironmentBlock->Ldr;
        auto pHeader = &pLdr->InLoadOrderModuleList;
        for (auto pItem = pHeader->Flink; pItem != pHeader;)
        {
            auto pLdrEntry =
                CONTAINING_RECORD(pItem, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            if (pLdrEntry->DllBase == h)
            {
                wp_printf("InLoadOrderLinks found:%d\n", i);
                pos_found = i;
            }
            wp_printf(
                "InLoadOrderLinks %d %S LoadCount:%d base:%p\n",
                i,
                pLdrEntry->BaseDllName.Buffer,
                GetLoadCount(pLdrEntry),
                pLdrEntry->DllBase);
            if (i < count && pEntries != nullptr)
            {
                pEntries[i] = pLdrEntry;
            }
            i += 1;
            pItem = pItem->Flink;
        }
        return i;
    }

    size_t FindModuleHandleInInitializationOrderModuleList(
        HANDLE h,
        LDR_DATA_TABLE_ENTRY **pEntries,
        size_t count)
    {
        int i = 0;
        size_t pos_found = SIZE_MAX;
        auto pTeb = NtCurrentTeb();
        auto pLdr = pTeb->ProcessEnvironmentBlock->Ldr;
        auto pHeader = &pLdr->InInitializationOrderModuleList;
        for (auto pItem = pHeader->Flink; pItem != pHeader;)
        {
            auto pLdrEntry =
                CONTAINING_RECORD(pItem, LDR_DATA_TABLE_ENTRY, InProgressLinks);
            if (pLdrEntry->DllBase == h)
            {
                wp_printf("InProgressLinks found:%d\n", i);
                pos_found = i;
            }
            wp_printf(
                "InProgressLinks %d %S LoadCount:%d base:%p\n",
                i,
                pLdrEntry->BaseDllName.Buffer,
                GetLoadCount(pLdrEntry),
                pLdrEntry->DllBase);
            if (i < count && pEntries != nullptr)
            {
                pEntries[i] = pLdrEntry;
            }
            i += 1;
            pItem = pItem->Flink;
        }
        return i;
    }
} // namespace
