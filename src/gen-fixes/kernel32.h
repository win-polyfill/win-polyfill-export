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


#if defined(_X86_)
unsigned WINAPI
InterlockedCompareExchange(
    _Inout_ _Interlocked_operand_ unsigned volatile *Destination,
    _In_ unsigned Exchange,
    _In_ unsigned Comperand
    );

unsigned WINAPI
InterlockedDecrement(
    _Inout_ _Interlocked_operand_ unsigned volatile *Addend
    );

unsigned WINAPI
InterlockedExchange(
    _Inout_ _Interlocked_operand_ unsigned volatile *Target,
    _In_ unsigned Value
    );

unsigned WINAPI
InterlockedExchangeAdd(
    _Inout_ _Interlocked_operand_ unsigned volatile *Addend,
    _In_ unsigned Value
    );

unsigned WINAPI
InterlockedIncrement(
    _Inout_ _Interlocked_operand_ unsigned volatile *Addend
    );

LONG64 WINAPI
InterlockedCompareExchange64 (
    _Inout_ _Interlocked_operand_ LONG64 volatile *Destination,
    _In_ LONG64 ExChange,
    _In_ LONG64 Comperand
    );

#endif /* defined(_X86_) */

WINBASEAPI
PSLIST_ENTRY
WINAPI
InterlockedPushListSList(
    _Inout_ PSLIST_HEADER ListHead,
    _Inout_ PSLIST_ENTRY List,
    _Inout_ PSLIST_ENTRY ListEnd,
    _In_ ULONG Count
    );

}
