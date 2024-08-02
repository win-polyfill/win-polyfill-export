#pragma once

#include <windows.h>

#include "win32_api_pop.h"

extern "C" {

_Check_return_ int __cdecl abs(_In_ int _X);

_Check_return_ _VCRTIMP void _CONST_RETURN *__cdecl memchr(
    _In_reads_bytes_opt_(_MaxCount) void const *_Buf,
    _In_ int _Val,
    _In_ size_t _MaxCount);

_Check_return_
    _VCRTIMP char _CONST_RETURN *__cdecl strchr(_In_z_ char const *_Str, _In_ int _Val);

_Check_return_ _ACRTIMP char _CONST_RETURN *__cdecl strpbrk(
    _In_z_ char const *_Str,
    _In_z_ char const *_Control);

_Check_return_
    _VCRTIMP char _CONST_RETURN *__cdecl strrchr(_In_z_ char const *_Str, _In_ int _Ch);

_Check_return_ _Ret_maybenull_ _VCRTIMP char _CONST_RETURN *__cdecl strstr(
    _In_z_ char const *_Str,
    _In_z_ char const *_SubStr);

_Check_return_
_When_(return != NULL, _Ret_range_(_Str, _Str + _String_length_(_Str) - 1))
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcschr(
    _In_z_ wchar_t const* _Str,
    _In_   wchar_t        _Ch
    );

_Check_return_
_ACRTIMP wchar_t _CONST_RETURN* __cdecl wcspbrk(
    _In_z_ wchar_t const* _String,
    _In_z_ wchar_t const* _Control
    );

_Check_return_
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcsrchr(
    _In_z_ wchar_t const* _Str,
    _In_   wchar_t        _Ch
    );

_Check_return_ _Ret_maybenull_
_When_(return != NULL, _Ret_range_(_Str, _Str + _String_length_(_Str) - 1))
_VCRTIMP wchar_t _CONST_RETURN* __cdecl wcsstr(
    _In_z_ wchar_t const* _Str,
    _In_z_ wchar_t const* _SubStr
    );
}
