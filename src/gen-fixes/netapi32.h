#pragma once

#include <windows.h>

#include "win32_api_pop.h"

EXTERN_C_START

#if defined(_X86_)
DSGETDCAPI
DWORD
WINAPI
DsGetDcOpen(
    _In_ LPCSTR DnsName,
    _In_ ULONG OptionFlags,
    _In_opt_ LPCSTR SiteName,
    _In_opt_ GUID *DomainGuid,
    _In_opt_ LPCSTR DnsForestName,
    _In_ ULONG DcFlags,
    _Out_ PHANDLE RetGetDcContext
    );

_Success_(return == ERROR_SUCCESS)
DSGETDCAPI
DWORD
WINAPI
DsGetDcNext(
    _In_ HANDLE GetDcContextHandle,
    _Out_opt_ PULONG SockAddressCount,
    _Outptr_opt_result_buffer_(*SockAddressCount) LPSOCKET_ADDRESS *SockAddresses,
    _Outptr_opt_result_nullonfailure_ LPSTR *DnsHostName
    );
#endif
DSGETDCAPI
VOID
WINAPI
DsGetDcClose(
    _In_ HANDLE GetDcContextHandle
    );

EXTERN_C_END
