#pragma once

#include "nt.h"

#include <stdint.h>

namespace
{
    template <typename Char>
    size_t StringLength(_In_z_ const Char *_szString, size_t _cchMaxLength = -1)
    {
        if (!_szString)
            return 0;

        size_t _cchString = 0;
        for (; _cchMaxLength && *_szString; --_cchMaxLength, ++_szString)
        {
            ++_cchString;
        }

        return _cchString;
    }

    static void __fastcall ClearNtString(UNICODE_STRING *str)
    {
        str->Buffer = nullptr;
        str->MaximumLength = 0;
        str->Length = 0;
    }

    static UNICODE_STRING __fastcall MakeNtString(
        _In_z_ const wchar_t *_szString,
        size_t count)
    {
        const auto _cbString = count * sizeof(_szString[0]);
        UNICODE_STRING _Result = {
            (USHORT)min(UINT16_MAX, _cbString),
            (USHORT)min(UINT16_MAX, _cbString + sizeof(_szString[0])),
            const_cast<PWSTR>(_szString)};
        return _Result;
    }

    static UNICODE_STRING __fastcall MakeNtString(_In_z_ const wchar_t *_szString)
    {
        return MakeNtString(_szString, StringLength(_szString, UINT16_MAX / 2));
    }

    static STRING __fastcall MakeNtString(_In_z_ const char *_szString, size_t count)
    {
        const auto _cbString = count * sizeof(_szString[0]);

        STRING _Result = {
            (USHORT)min(UINT16_MAX, _cbString),
            (USHORT)min(UINT16_MAX, _cbString + sizeof(_szString[0])),
            const_cast<PSTR>(_szString)};
        return _Result;
    }

    static STRING __fastcall MakeNtString(_In_z_ const char *_szString)
    {
        return MakeNtString(_szString, StringLength(_szString, UINT16_MAX));
    }

    LONG NTAPI RtlCompareUnicodeStrings(
        _In_reads_(String1Length) PCWCH String1,
        _In_ SIZE_T String1Length,
        _In_reads_(String2Length) PCWCH String2,
        _In_ SIZE_T String2Length,
        _In_ BOOLEAN CaseInSensitive)
    {
        auto a = MakeNtString(String1, String1Length);
        auto b = MakeNtString(String2, String2Length);
        return RtlCompareUnicodeString(&a, &b, CaseInSensitive);
    }

    __forceinline LONG
    CompareNtString(const STRING *a, const STRING *b, BOOLEAN CaseInSensitive)
    {
        return RtlCompareString(a, b, CaseInSensitive);
    }

    __forceinline LONG
    CompareNtString(const char *a, const char *b, BOOLEAN CaseInSensitive)
    {
        const STRING as = MakeNtString(a);
        const STRING bs = MakeNtString(b);
        return CompareNtString(&as, &bs, CaseInSensitive);
    }

    __forceinline LONG CompareNtString(
        const UNICODE_STRING *a,
        const UNICODE_STRING *b,
        BOOLEAN CaseInSensitive)
    {
        return RtlCompareUnicodeString(a, b, CaseInSensitive);
    }

    // The result directory path will be one of
    // "C:"
    // "C:\"
    // "C:\somepath\dirname"
    const wchar_t *GetDirectoyEnd(const wchar_t *FilePath, size_t FilePathLength)
    {
        const wchar_t *FilePathEnd = FilePath + FilePathLength;
        if (FilePathLength == 0)
        {
            return FilePath;
        }
        for (; FilePathEnd > FilePath;)
        {
            --FilePathEnd;
            wchar_t c = *FilePathEnd;
            if (c == '\\' || c == '/')
            {
                break;
            }
        }
        if (FilePathEnd == FilePath + 2)
        {
            if (FilePath[1] == ':')
                FilePathEnd += 1;
        }
        else if (FilePathEnd == FilePath)
        {
            if (FilePathLength >= 2 && FilePath[0] && FilePath[1] == ':')
            {
                FilePathEnd += 2;
            }
        }
        return FilePathEnd;
    }

} // namespace
