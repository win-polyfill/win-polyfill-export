#pragma once

#include <windows.h>

#include "win32_api_pop.h"

extern "C"
{

WINBASEAPI
BOOL
WINAPI
ReplaceFile(
    _In_       LPCSTR lpReplacedFileName,
    _In_       LPCSTR lpReplacementFileName,
    _In_opt_   LPCSTR lpBackupFileName,
    _In_       DWORD    dwReplaceFlags,
    _Reserved_ LPVOID   lpExclude,
    _Reserved_ LPVOID  lpReserved
    );

WINBASEAPI
_NullNull_terminated_
LPCH
WINAPI
GetEnvironmentStringsA(
    VOID
    );

WINBASEAPI
BOOL
WINAPI
GetBinaryType(
    _In_  LPCSTR lpApplicationName,
    _Out_ LPDWORD  lpBinaryType
    );
WINBASEAPI
LPSTR
WINAPI
lstrcat(
    _Inout_updates_z_(_String_length_(lpString1) + _String_length_(lpString2) + 1) LPSTR lpString1, // deprecated: annotation is as good as it gets
    _In_    LPCSTR lpString2
    );

WINBASEAPI
int
WINAPI
lstrcmp(
    _In_ LPCSTR lpString1,
    _In_ LPCSTR lpString2
    );

WINBASEAPI
int
WINAPI
lstrcmpi(
    _In_ LPCSTR lpString1,
    _In_ LPCSTR lpString2
    );

WINBASEAPI
LPSTR
WINAPI
lstrcpy(
    _Out_writes_(_String_length_(lpString2) + 1) LPSTR lpString1, // deprecated: annotation is as good as it gets
    _In_  LPCSTR lpString2
    );

WINBASEAPI
_Check_return_
_Success_(return != NULL)
_Post_satisfies_(return == lpString1)
_Ret_maybenull_
LPSTR
WINAPI
lstrcpyn(
    _Out_writes_(iMaxLength) LPSTR lpString1,
    _In_ LPCSTR lpString2,
    _In_ int iMaxLength
    );

WINBASEAPI
int
WINAPI
lstrlen(
    _In_ LPCSTR lpString
    );

WINBASEAPI
PSLIST_ENTRY
WINAPI
InterlockedPushListSList(
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PSLIST_ENTRY List,
    _Inout_ PSLIST_ENTRY ListEnd,
    _In_ ULONG Count
    );

WINBASEAPI
_Check_return_
_Success_(return != NULL)
_Post_satisfies_(return == lpString1)
_Ret_maybenull_
LPSTR
WINAPI
StrCpyNA(
    _Out_writes_(iMaxLength) LPSTR lpString1,
    _In_ LPCSTR lpString2,
    _In_ int iMaxLength
    );

SHSTDAPI_(DWORD_PTR) SHGetFileInfo(_In_ LPCSTR pszPath, DWORD dwFileAttributes, _Inout_updates_bytes_opt_(cbFileInfo) SHFILEINFOA *psfi,
    UINT cbFileInfo, UINT uFlags);

_Success_(return != 0)
SHSTDAPI_(BOOL) SHGetNewLinkInfo(_In_ LPCSTR pszLinkTo, _In_ LPCSTR pszDir, _Out_writes_(MAX_PATH) LPSTR pszName, _Out_ BOOL *pfMustCopy, _In_ UINT uFlags);

_Success_(return != 0)
SHSTDAPI_(BOOL) SHGetPathFromIDList(_In_ PCIDLIST_ABSOLUTE pidl, _Out_writes_(MAX_PATH) LPSTR pszPath);

SHSTDAPI_(UINT) DragQueryFile(_In_ HDROP hDrop, _In_ UINT iFile, _Out_writes_opt_(cch) LPSTR lpszFile, _In_ UINT cch);

SHSTDAPI SHLoadOLE(LPARAM lParam);
SHSTDAPI_(LONG) PathProcessCommand(_In_ PCWSTR pszSrc, _Out_writes_(cchDest) PWSTR pszDest, int cchDest, DWORD dwFlags);

SHSTDAPI_(UINT) ExtractIconEx(_In_ LPCSTR lpszFile, int nIconIndex, _Out_writes_opt_(nIcons) HICON *phiconLarge, _Out_writes_opt_(nIcons) HICON *phiconSmall, UINT nIcons);

SHSTDAPI_(PIDLIST_ABSOLUTE)     ILCreateFromPath(_In_ PCTSTR pszPath);

SHSTDAPI_(BOOL) IsLFNDrive(_In_opt_ LPCSTR pszPath);

SHSTDAPI_(PIDLIST_ABSOLUTE) SHBrowseForFolder(_In_ LPBROWSEINFOA lpbi);

SHSTDAPI_(int) SHFileOperation(_Inout_ LPSHFILEOPSTRUCTA lpFileOp);

SHSTDAPI_(BOOL) ShellExecuteEx(_Inout_ SHELLEXECUTEINFOA *pExecInfo);

SHSTDAPI_(BOOL) Shell_NotifyIcon(DWORD dwMessage, _In_ PNOTIFYICONDATAA lpData);

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

}

WINADVAPI
DWORD
WINAPI
NotifyServiceStatusChange (
    _In_        SC_HANDLE               hService,
    _In_        DWORD                   dwNotifyMask,
    _In_        PSERVICE_NOTIFYA        pNotifyBuffer
    );
