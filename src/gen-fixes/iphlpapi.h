#pragma once

#include <windows.h>

EXTERN_C_START

DWORD
AllocateAndGetTcpExTableFromStack(
    _Outptr_ PVOID         *ppTcpTable,
    _In_        BOOL          bOrder,
    _In_        HANDLE        hHeap,
    _In_        DWORD         dwFlags,
    _In_        DWORD         dwFamily
    );

DWORD
AllocateAndGetUdpExTableFromStack(
    _Outptr_ PVOID         *ppUdpTable,
    _In_        BOOL          bOrder,
    _In_        HANDLE        hHeap,
    _In_        DWORD         dwFlags,
    _In_        DWORD         dwFamily
    );

EXTERN_C_END
