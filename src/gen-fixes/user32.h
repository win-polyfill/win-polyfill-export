#pragma once

#include <windows.h>

EXTERN_C_START

WINUSERAPI
long
WINAPI
BroadcastSystemMessage(
    _In_ DWORD flags,
    _Inout_opt_ LPDWORD lpInfo,
    _In_ UINT Msg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam);

WINUSERAPI
BOOL
WINAPI
CallMsgFilter(
    _In_ LPMSG lpMsg,
    _In_ int nCode);

WINUSERAPI
BOOL
WINAPI
GetAltTabInfo(
    _In_opt_ HWND hwnd,
    _In_ int iItem,
    _Inout_ PALTTABINFO pati,
    _Out_writes_opt_(cchItemText) LPSTR pszItemText,
    _In_ UINT cchItemText);

WINUSERAPI
UINT
WINAPI
GetWindowModuleFileName(
    _In_ HWND hwnd,
    _Out_writes_to_(cchFileNameMax, return) LPSTR pszFileName,
    _In_ UINT cchFileNameMax);

WINUSERAPI
BOOL
WINAPI
IsDialogMessage(
    _In_ HWND hDlg,
    _In_ LPMSG lpMsg);

WINUSERAPI
UINT
WINAPI
RealGetWindowClass(
    _In_ HWND hwnd,
    _Out_writes_to_(cchClassNameMax, return) LPSTR ptszClassName,
    _In_ UINT cchClassNameMax);

WINUSERAPI
int
WINAPI
TranslateAccelerator(
    _In_ HWND hWnd,
    _In_ HACCEL hAccTable,
    _In_ LPMSG lpMsg);

BOOLEAN
__stdcall
SystemFunction036(
    _Out_writes_bytes_(RandomBufferLength) PVOID RandomBuffer,
    _In_ ULONG RandomBufferLength
    );

NTSTATUS
__stdcall
SystemFunction040(
    _Inout_updates_bytes_(MemorySize) PVOID Memory,
    _In_ ULONG MemorySize,
    _In_ ULONG OptionFlags
    );

NTSTATUS
__stdcall
SystemFunction041(
    _Inout_updates_bytes_(MemorySize) PVOID Memory,
    _In_ ULONG MemorySize,
    _In_ ULONG OptionFlags
    );

EXTERN_C_END
