#pragma once

#include <windows.h>

#include "win32_api_pop.h"

extern "C"
{

WINADVAPI
DWORD
WINAPI
NotifyServiceStatusChange (
    _In_        SC_HANDLE               hService,
    _In_        DWORD                   dwNotifyMask,
    _In_        PSERVICE_NOTIFYA        pNotifyBuffer
    );

}
