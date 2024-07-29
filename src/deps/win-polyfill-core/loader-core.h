#pragma once

#include "nt.h"

#include "alloca.h"
#include "string.h"

namespace internal
{
    namespace
    {
        wchar_t *GetFullDllNameByApplicationDir(PPEB peb, const wchar_t *DllNameInMemory)
        {
            auto DllNameLength = StringLength(DllNameInMemory);
            auto &ImagePathName = peb->ProcessParameters->ImagePathName;
            auto PathEnd = GetDirectoyEnd(
                ImagePathName.Buffer, ImagePathName.Length / sizeof(wchar_t));
            auto ApplicationDirectoryLength = PathEnd - ImagePathName.Buffer;

            auto FullDllNameLength = ApplicationDirectoryLength + 1 + DllNameLength;
            auto FullImageName = (wchar_t *)Alloc(
                (FullDllNameLength + 1) * sizeof(wchar_t), HEAP_ZERO_MEMORY);
            if (FullImageName == nullptr)
                return nullptr;
            RtlMoveMemory(
                FullImageName,
                ImagePathName.Buffer,
                ApplicationDirectoryLength * sizeof(wchar_t));
            FullImageName[ApplicationDirectoryLength] = L'\\';
            RtlMoveMemory(
                FullImageName + ApplicationDirectoryLength + 1,
                DllNameInMemory,
                DllNameLength * sizeof(wchar_t));
            FullImageName[FullDllNameLength] = L'\0';
            return FullImageName;
        }

        LDR_DATA_TABLE_ENTRY *LdrpFindEntry(PEB *peb, PVOID handle)
        {
            auto pLdr = peb->Ldr;
            auto pHeader = &pLdr->InLoadOrderModuleList;
            for (auto pItem = pHeader->Flink; pItem != pHeader; pItem = pItem->Flink)
            {
                auto pLdrEntry =
                    CONTAINING_RECORD(pItem, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                if (handle == pLdrEntry->DllBase)
                {
                    return pLdrEntry;
                }
            }
            return nullptr;
        }

        NTSTATUS GetDllFullPath(
            LPCWSTR DllName,
            DWORD dwFlags,
            PWSTR *pDllPath,
            PWSTR *pDllFullPath,
            PWSTR *lpFilePart)
        {
            PWSTR DllPath = nullptr;
            PWSTR SearchPaths = nullptr;
            NTSTATUS Status = LdrGetDllPath(DllName, dwFlags, &DllPath, &SearchPaths);
            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
            ULONG BufferLength =
                RtlDosSearchPath_U(DllPath, DllName, L".dll", 0, nullptr, nullptr);
            if (BufferLength == 0)
            {
                return STATUS_DLL_NOT_FOUND;
            }
            PWSTR DllFullPath = (PWSTR)Alloc(BufferLength);
            ULONG BufferLengthNew = RtlDosSearchPath_U(
                DllPath, DllName, L".dll", BufferLength, DllFullPath, lpFilePart);
            if (BufferLengthNew == 0 || BufferLengthNew > BufferLength)
            {
                RtlReleasePath(DllPath);
                Free(DllFullPath);
                return STATUS_DLL_NOT_FOUND;
            }
            *pDllPath = DllPath;
            *pDllFullPath = DllFullPath;
            return Status;
        }

    } // namespace
} // namespace internal
