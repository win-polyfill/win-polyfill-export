#pragma once

#include "win-polyfill-arch.h"

#include <minwindef.h>

typedef BOOL(__stdcall *__win_polyfill_dll_main_type)(
    HINSTANCE const hInstance,
    DWORD dwReason,
    PVOID lpReserved);

typedef void (
    *__win_polyfill_peb_state_handle_process_callback)(PVOID hInstance, PVOID lpReserved);
