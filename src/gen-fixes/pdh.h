#pragma once

#include <windows.h>
#include <pdh.h>

#include "win32_api_pop.h"

extern "C"
{
PDH_FUNCTION
PdhOpenQuery(
    _In_opt_ LPCSTR       szDataSource,
    _In_     DWORD_PTR    dwUserData,
    _Out_    PDH_HQUERY * phQuery
);

}
