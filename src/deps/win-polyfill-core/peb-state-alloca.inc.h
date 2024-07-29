// 只能在一个cpp文件中包含
#pragma once

#include "nt.h"

#include "peb-state.h"

WinPolyfillPebState *__win_polyfill_peb_state_alloc(void *ctx)
{
    auto DllName = MakeNtString(L"win-polyfill-peb-state.dll");
    PVOID handle;
    NTSTATUS st = LdrLoadDll(nullptr, nullptr, &DllName, &handle);
    if (!NT_SUCCESS(st))
        return nullptr;
    if (!handle)
        return nullptr;

    decltype(&__win_polyfill_peb_state_alloc) pfn = nullptr;
    auto func_name = MakeNtString("win_polyfill_peb_state_alloc_get");
    st = LdrGetProcedureAddress(handle, &func_name, 0, (PVOID *)&pfn);
    if (!NT_SUCCESS(st))
        return nullptr;
    if (!pfn)
        return nullptr;
    return pfn(ctx);
}
