#pragma once

#include <windows.h>


EXTERN_C_START

SHSTDAPI_(UINT) DragQueryFile(_In_ HDROP hDrop, _In_ UINT iFile, _Out_writes_opt_(cch) LPSTR lpszFile, _In_ UINT cch);
SHSTDAPI_(UINT) ExtractIconEx(_In_ LPCSTR lpszFile, int nIconIndex, _Out_writes_opt_(nIcons) HICON *phiconLarge, _Out_writes_opt_(nIcons) HICON *phiconSmall, UINT nIcons);
SHSTDAPI_(DWORD_PTR) SHGetFileInfo(_In_ LPCSTR pszPath, DWORD dwFileAttributes, _Inout_updates_bytes_opt_(cbFileInfo) SHFILEINFOA *psfi,
    UINT cbFileInfo, UINT uFlags);

_Success_(return != 0)
SHSTDAPI_(BOOL) SHGetNewLinkInfo(_In_ LPCSTR pszLinkTo, _In_ LPCSTR pszDir, _Out_writes_(MAX_PATH) LPSTR pszName, _Out_ BOOL *pfMustCopy, _In_ UINT uFlags);

_Success_(return != 0)
SHSTDAPI_(BOOL) SHGetPathFromIDList(_In_ PCIDLIST_ABSOLUTE pidl, _Out_writes_(MAX_PATH) LPSTR pszPath);

SHSTDAPI SHLoadOLE(LPARAM lParam);
SHSTDAPI_(LONG) PathProcessCommand(_In_ PCWSTR pszSrc, _Out_writes_(cchDest) PWSTR pszDest, int cchDest, DWORD dwFlags);
SHSTDAPI_(PIDLIST_ABSOLUTE) ILCreateFromPath(_In_ PCSTR pszPath);
SHSTDAPI_(BOOL) IsLFNDrive(_In_opt_ LPCSTR pszPath);
SHSTDAPI_(PIDLIST_ABSOLUTE) SHBrowseForFolder(_In_ LPBROWSEINFOA lpbi);
SHSTDAPI_(int) SHFileOperation(_Inout_ LPSHFILEOPSTRUCTA lpFileOp);
SHSTDAPI_(BOOL) ShellExecuteEx(_Inout_ SHELLEXECUTEINFOA *pExecInfo);
SHSTDAPI_(BOOL) Shell_NotifyIcon(DWORD dwMessage, _In_ PNOTIFYICONDATAA lpData);

EXTERN_C_END
