#pragma once

#include <windows.h>

EXTERN_C_START

WINADVAPI
DWORD
WINAPI
NotifyServiceStatusChange (
    _In_        SC_HANDLE               hService,
    _In_        DWORD                   dwNotifyMask,
    _In_        PSERVICE_NOTIFYA        pNotifyBuffer
    );

EXTERN_C_END
