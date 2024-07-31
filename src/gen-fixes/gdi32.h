#pragma once

#include <windows.h>

#include "win32_api_pop.h"

extern "C"
{

WINGDIAPI DWORD WINAPI GetGlyphOutline(
    _In_ HDC hdc,
    _In_ UINT uChar,
    _In_ UINT fuFormat,
    _Out_ LPGLYPHMETRICS lpgm,
    _In_ DWORD cjBuffer,
    _Out_writes_bytes_opt_(cjBuffer) LPVOID pvBuffer,
    _In_ CONST MAT2 *lpmat2);

WINGDIAPI DWORD WINAPI GetKerningPairs(
    _In_ HDC hdc,
    _In_ DWORD nPairs,
    _Out_writes_to_opt_(nPairs, return) LPKERNINGPAIR lpKernPair);

}
