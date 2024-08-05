#pragma once

#include <windows.h>

EXTERN_C_START

VOID
WINAPI
InstallHinfSection(
    _In_ HWND Window,
    _In_ HINSTANCE ModuleHandle,
    _In_ PCSTR CommandLine,
    _In_ INT ShowCommand
    );

WINSETUPAPI
BOOL
WINAPI
SetupCommitFileQueue(
    _In_opt_ HWND Owner,
    _In_ HSPFILEQ QueueHandle,
    _In_ PSP_FILE_CALLBACK_A MsgHandler,
    _In_ PVOID Context
    );


WINSETUPAPI
UINT
WINAPI
SetupDefaultQueueCallback(
    _In_ PVOID Context,
    _In_ UINT Notification,
    _In_ UINT_PTR Param1,
    _In_ UINT_PTR Param2
    );

WINSETUPAPI
BOOL
WINAPI
SetupScanFileQueue(
    _In_ HSPFILEQ FileQueue,
    _In_ DWORD Flags,
    _In_opt_ HWND Window,
    _In_opt_ PSP_FILE_CALLBACK_A CallbackRoutine,
    _In_opt_ PVOID CallbackContext,
    _Out_ PDWORD Result
    );

EXTERN_C_END
