#pragma once

#include "nt.h"

#include "alloca.h"
#include "loader-core.h"
#include "peb-state.h"
#include "string.h"
#include "version.h"

#define LDR_HASH_TABLE_ENTRIES 32
#define LDR_HASH_MASK (LDR_HASH_TABLE_ENTRIES - 1)

namespace internal
{
    namespace
    {

        BOOL
        CallInitRoutine(LDR_DATA_TABLE_ENTRY *pLdrEntry, DWORD dwReason, PVOID lpReserved)
        {
            BOOL ret = TRUE;
            auto InitRoutine = (__win_polyfill_dll_main_type)pLdrEntry->EntryPoint;
            if (InitRoutine)
                ret = InitRoutine((HINSTANCE)pLdrEntry->DllBase, dwReason, lpReserved);
            if (dwReason == DLL_PROCESS_ATTACH)
            {
                pLdrEntry->ProcessAttachCalled = true;
            }
            return ret;
        }

        LDR_DATA_TABLE_ENTRY *LdrpFindEntryByName(PEB *peb, const wchar_t *name) noexcept
        {
            auto pLdr = peb->Ldr;
            auto pHeader = &pLdr->InLoadOrderModuleList;
            auto str = MakeNtString(name);
            for (auto pItem = pHeader->Flink; pItem != pHeader; pItem = pItem->Flink)
            {
                auto pLdrEntry =
                    CONTAINING_RECORD(pItem, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                if (CompareNtString(&str, &pLdrEntry->BaseDllName, TRUE) == 0)
                {
                    return pLdrEntry;
                }
            }
            return nullptr;
        }

        NTSTATUS NTAPI LdrpFindHeapEnum(_In_ PVOID HeapHandle, _In_ PVOID Parameter)
        {
            auto pPebState = (WinPolyfillPebState *)Parameter;
            if (pPebState->LdrpHeap == HeapHandle)
                return STATUS_SUCCESS;
            auto NtdllLdrEntry =
                ::internal::LdrpFindEntryByName(NtCurrentPeb(), L"ntdll.dll");
            if (RtlValidateHeap(HeapHandle, 0, NtdllLdrEntry))
            {
                pPebState->LdrpHeap = HeapHandle;
                return STATUS_NO_MORE_ENTRIES;
            }
            return STATUS_SUCCESS;
        }

        NTSTATUS LdrpFindHeap(WinPolyfillPebState *pPebState)
        {
            pPebState->LdrpHeap = NtCurrentPeb()->ProcessHeap;
            auto SystemVersion = pPebState->SystemVersion;
            NTSTATUS st = STATUS_SUCCESS;
            if (SystemVersion >= SYSTEM_NT_5_1_RTM && SystemVersion < SYSTEM_NT_6_1_RTM)
            {
                if (SystemVersion >= SYSTEM_NT_6_0_RTM)
                {
                    // TODO: Windows Vista is unknow
                    pPebState->LdrpHeap = nullptr;
                }
                // Windows XP and 2003 have private LdrpHeap
                st = RtlEnumProcessHeaps(LdrpFindHeapEnum, pPebState);
                if (st == STATUS_NO_MORE_ENTRIES)
                {
                    st = STATUS_SUCCESS;
                }
            }
            else
            {
                // Windows 2000 and lower use ProcessHeap directly
                // Windows 7 and higher use ProcessHeap directly
            }

            if (NT_SUCCESS(st))
            {
                auto NtdllLdrEntry =
                    ::internal::LdrpFindEntryByName(NtCurrentPeb(), L"ntdll.dll");
                pPebState->LdrDataTableEntrySize =
                    RtlSizeHeap(pPebState->LdrpHeap, 0, NtdllLdrEntry);
                if (pPebState->SystemVersion >= SYSTEM_NT_6_2_RTM)
                {
                    pPebState->LdrDdagNodeSize =
                        RtlSizeHeap(pPebState->LdrpHeap, 0, NtdllLdrEntry->DdagNode);
                }
            }
            return st;
        }

        PRTL_BALANCED_NODE LdrpFindRbTreeRootNode(PRTL_BALANCED_NODE pNode)
        {
            for (;;)
            {
                auto pNodeParent = (PRTL_BALANCED_NODE)(pNode->ParentValue & (~7));
                if (pNodeParent == nullptr)
                    return pNode;
                pNode = pNodeParent;
            }
        }

        typedef int (*LdrpRbTreeCompare)(PRTL_BALANCED_NODE a, PRTL_BALANCED_NODE b);

        NTSTATUS LdrpRbTreeInsert(
            PRTL_RB_TREE pTree,
            PRTL_BALANCED_NODE pLdrNodeToInsert,
            LdrpRbTreeCompare compare)
        {
            if (!pTree)
                return STATUS_UNSUCCESSFUL;
            auto LdrNode = pTree->Root;
            bool bRight = false;

            while (true)
            {
                int cmp = compare(pLdrNodeToInsert, LdrNode);
                if (cmp < 0)
                {
                    if (!LdrNode->Left)
                        break;
                    LdrNode = LdrNode->Left;
                }
                else if (cmp > 0)
                {
                    if (!LdrNode->Right)
                    {
                        bRight = true;
                        break;
                    }
                    LdrNode = LdrNode->Right;
                }
                else
                {
                    // Found it
                    return STATUS_SUCCESS;
                }
            }

            if (!RtlRbInsertNodeEx(pTree, LdrNode, bRight, pLdrNodeToInsert))
            {
                return STATUS_UNSUCCESSFUL;
            }
            return STATUS_SUCCESS;
        }

        int LdrpRbTreeCompareDllBaseNode(PRTL_BALANCED_NODE a, PRTL_BALANCED_NODE b)
        {
            auto LdrEntryA =
                CONTAINING_RECORD(a, LDR_DATA_TABLE_ENTRY, BaseAddressIndexNode);
            auto LdrEntryB =
                CONTAINING_RECORD(b, LDR_DATA_TABLE_ENTRY, BaseAddressIndexNode);
            if (LdrEntryA->DllBase == LdrEntryB->DllBase)
                return 0;
            return LdrEntryA->DllBase < LdrEntryB->DllBase ? -1 : 1;
        }

        int LdrpRbTreeCompareTimeStamp(PRTL_BALANCED_NODE nodeA, PRTL_BALANCED_NODE nodeB)
        {
            auto a = CONTAINING_RECORD(nodeA, LDR_DATA_TABLE_ENTRY, MappingInfoIndexNode);
            auto b = CONTAINING_RECORD(nodeB, LDR_DATA_TABLE_ENTRY, MappingInfoIndexNode);
            if (a->TimeDateStamp != b->TimeDateStamp)
            {
                return a->TimeDateStamp < b->TimeDateStamp ? -1 : 1;
            }
            if (a->SizeOfImage != b->SizeOfImage)
            {
                return a->SizeOfImage < b->SizeOfImage ? -1 : 1;
            }
            if (a->DllBase == b->DllBase)
                return 0;
            return a->DllBase < b->DllBase ? -1 : 1;
        }

        PRTL_BALANCED_NODE
        LdrpFindRbTreeMinNode(PRTL_BALANCED_NODE pNodeRoot, LdrpRbTreeCompare compare)
        {
            auto pNodeParent = pNodeRoot;
            for (;;)
            {
                auto pNode = pNodeParent->Left;
                if (pNode == nullptr)
                {
                    return pNodeParent;
                }
                // int cmp = compare(pNode, pNodeParent);
                pNodeParent = pNode;
            }
        }

        void LdrpFindRbTree(
            PRTL_BALANCED_NODE pNode,
            PVOID DllBase,
            PRTL_RB_TREE &pTreeOut,
            LdrpRbTreeCompare compare)
        {
            RTL_RB_TREE TreeToFind;
            TreeToFind.Root = LdrpFindRbTreeRootNode(pNode);
            TreeToFind.Min = LdrpFindRbTreeMinNode(TreeToFind.Root, compare);

            auto pNtHeaders = RtlImageNtHeader(DllBase);

            auto sh = IMAGE_FIRST_SECTION(pNtHeaders);
            INT i;
            for (i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
            {
                if (!CompareNtString(".data", (LPCSTR)sh->Name, FALSE))
                {
                    break;
                }

                ++sh;
            }
            if (i == pNtHeaders->FileHeader.NumberOfSections)
                return;

            UINT_PTR stBegin = (UINT_PTR)DllBase + sh->VirtualAddress;
            UINT_PTR stEnd = stBegin + sh->Misc.VirtualSize - sizeof(TreeToFind);
            for (UINT_PTR ptr = stBegin; ptr <= stEnd; ptr += sizeof(SIZE_T))
            {
                auto stRet =
                    RtlCompareMemory((PVOID)ptr, &TreeToFind, sizeof(TreeToFind));
                if (stRet == sizeof(TreeToFind))
                {

                    PRTL_RB_TREE pTree = (PRTL_RB_TREE)ptr;
                    if (pTree && pTree->Root && pTree->Min)
                        pTreeOut = pTree;
                    break;
                }
            }
        }

        // TODO: Still not that reliable
        void LdrpFindLdrpRbTreeRoot(WinPolyfillPebState *pPebState)
        {
            // Only Windows 8 and latter have BaseAddressIndexNode
            if (pPebState->SystemVersion < SYSTEM_NT_6_2_RTM)
                return;

            auto peb = NtCurrentPeb();
            // Only L"kernel32.dll" are present in both LdrpModuleBaseAddressIndex
            // and LdrpMappingInfoIndex
            auto pLdrEntryUsed = LdrpFindEntryByName(peb, L"kernel32.dll");
            auto NtdllLdrEntry = LdrpFindEntryByName(peb, L"ntdll.dll");

            // Try to find the address of ntdll!LdrpModuleBaseAddressIndex
            LdrpFindRbTree(
                &pLdrEntryUsed->BaseAddressIndexNode,
                NtdllLdrEntry->DllBase,
                pPebState->LdrpModuleBaseAddressIndex,
                LdrpRbTreeCompareDllBaseNode);

            // Try to find the address of ntdll!LdrpMappingInfoIndex
            LdrpFindRbTree(
                &pLdrEntryUsed->MappingInfoIndexNode,
                NtdllLdrEntry->DllBase,
                pPebState->LdrpMappingInfoIndex,
                LdrpRbTreeCompareTimeStamp);
        }

        LDR_DATA_TABLE_ENTRY *LdrpEntryAlloc(WinPolyfillPebState *pPebState)
        {
            auto sEntryNtdllSize = pPebState->LdrDataTableEntrySize;
            auto pNewLdrEntry = (LDR_DATA_TABLE_ENTRY *)RtlAllocateHeap(
                pPebState->LdrpHeap, HEAP_ZERO_MEMORY, sEntryNtdllSize);
            return pNewLdrEntry;
        }

        ULONG NTAPI
        LdrpHashEntryRaw(WinPolyfillPebState *pPebState, _In_ UNICODE_STRING &DllBaseName)
        {
            ULONG result = 0;
            auto SystemVersion = pPebState->SystemVersion;
            if (SystemVersion < SYSTEM_NT_6_0_RTM)
            {
                result = RtlUpcaseUnicodeChar(DllBaseName.Buffer[0]) - 'A';
            }
            else if (SystemVersion < SYSTEM_NT_6_1_RTM)
            {
                result = RtlUpcaseUnicodeChar(DllBaseName.Buffer[0]) - 1;
            }
            else if (SystemVersion < SYSTEM_NT_6_2_RTM)
            {
                for (USHORT i = 0; i < (DllBaseName.Length / sizeof(wchar_t)); ++i)
                    result += 0x1003F * RtlUpcaseUnicodeChar(DllBaseName.Buffer[i]);
            }
            else
            {
                RtlHashUnicodeString(
                    &DllBaseName, TRUE, HASH_STRING_ALGORITHM_DEFAULT, &result);
            }
            return result;
        }

        ULONG NTAPI
        LdrpHashEntry(WinPolyfillPebState *pPebState, _In_ UNICODE_STRING &DllBaseName)
        {
            return LdrpHashEntryRaw(pPebState, DllBaseName) & LDR_HASH_MASK;
        }

        BOOL
        LdrpIsValidHashTable(WinPolyfillPebState *pPebState, PLIST_ENTRY LdrpHashTable)
        {
            // Additional checks are performed to ensure that the LdrpHashTable is valid.
            __try
            {
                for (ULONG i = 0; i < LDR_HASH_TABLE_ENTRIES; ++i)
                {
                    PLIST_ENTRY head = &LdrpHashTable[i], entry = head->Flink;

                    while (head != entry)
                    {
                        PLDR_DATA_TABLE_ENTRY current =
                            CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, HashLinks);
                        auto HashValue = LdrpHashEntry(pPebState, current->BaseDllName) &
                                         LDR_HASH_MASK;
                        if (HashValue != i)
                        {
                            return FALSE;
                        }

                        entry = entry->Flink;
                    }
                }
                return TRUE;
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                return FALSE;
            }
        }

        void LdrpFindHashTable(WinPolyfillPebState *pPebState)
        {
            auto Peb = NtCurrentTeb()->ProcessEnvironmentBlock;
            PLIST_ENTRY head = &Peb->Ldr->InInitializationOrderModuleList;
            PLIST_ENTRY entry = head->Flink;

            while (head != entry)
            {
                PLDR_DATA_TABLE_ENTRY current = CONTAINING_RECORD(
                    entry, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
                PLIST_ENTRY hashEntry = &current->HashLinks;

                if (hashEntry->Flink != hashEntry && hashEntry->Flink->Flink == hashEntry)
                {
                    auto HashValue = LdrpHashEntry(pPebState, current->BaseDllName);
                    PLIST_ENTRY table = hashEntry->Flink - HashValue;
                    if (LdrpIsValidHashTable(pPebState, table))
                    {
                        pPebState->LdrpHashTable = table;
                    }
                }

                entry = entry->Flink;
            }
        }

        void LdrpResolveBaseDllName(
            const UNICODE_STRING &FullDllName,
            UNICODE_STRING &BaseDllName)
        {
            PWSTR pp = NULL;
            auto p = FullDllName.Buffer;
            while (*p)
            {
                if (*p++ == (WCHAR)'\\')
                {
                    pp = p;
                }
            }
            if (pp != NULL)
            {
                BaseDllName.Length = (USHORT)((ULONG_PTR)p - (ULONG_PTR)pp);
                BaseDllName.MaximumLength = BaseDllName.Length + sizeof(WCHAR);
                BaseDllName.Buffer = (PWSTR)(((ULONG_PTR)FullDllName.Buffer) +
                                             (FullDllName.Length - BaseDllName.Length));
            }
            else
            {
                BaseDllName = FullDllName;
            }
        }

        NTSTATUS
        LdrpAllocateUnicodeString(
            WinPolyfillPebState *pPebState,
            OUT PUNICODE_STRING StringOut,
            IN USHORT Length)
        {
            NTSTATUS st =
                STATUS_INTERNAL_ERROR; // returned if someone messes up and forgets to otherwise set it

            if (StringOut != NULL)
            {
                StringOut->Length = 0;
                StringOut->MaximumLength = 0;
                StringOut->Buffer = NULL;
            }

            if ((StringOut == NULL) || ((Length % sizeof(WCHAR)) != 0))
            {
                st = STATUS_INVALID_PARAMETER;
                goto Exit;
            }

            StringOut->Buffer =
                (PWCH)RtlAllocateHeap(pPebState->LdrpHeap, 0, Length + sizeof(WCHAR));
            if (StringOut->Buffer == NULL)
            {
                st = STATUS_NO_MEMORY;
                goto Exit;
            }

            StringOut->Buffer[Length / sizeof(WCHAR)] = L'\0';
            StringOut->Length = 0;

            // If the true length of the buffer can be represted in 16 bits, store it; otherwise
            // store the biggest number we can.
            if (Length != UNICODE_STRING_MAX_BYTES)
                StringOut->MaximumLength = Length + sizeof(WCHAR);
            else
                StringOut->MaximumLength = Length;

            st = STATUS_SUCCESS;
        Exit:
            return st;
        }

        NTSTATUS
        LdrpCopyUnicodeString(
            WinPolyfillPebState *pPebState,
            OUT PUNICODE_STRING StringOut,
            IN PCUNICODE_STRING StringIn)
        {
            NTSTATUS st = STATUS_INTERNAL_ERROR;
            ULONG BytesNeeded = 0;

            if (StringOut != NULL)
            {
                StringOut->Length = 0;
                StringOut->MaximumLength = 0;
                StringOut->Buffer = NULL;
            }

            if ((StringOut == NULL) || (StringIn == NULL))
            {
                st = STATUS_INVALID_PARAMETER;
                goto Exit;
            }

            st = RtlValidateUnicodeString(0, StringIn);
            if (!NT_SUCCESS(st))
                goto Exit;

            st = LdrpAllocateUnicodeString(pPebState, StringOut, StringIn->Length);
            if (!NT_SUCCESS(st))
                goto Exit;

            RtlCopyMemory(StringOut->Buffer, StringIn->Buffer, StringIn->Length);
            StringOut->Length = StringIn->Length;

            st = STATUS_SUCCESS;
        Exit:
            return st;
        }

        VOID LdrpFreeUnicodeString(
            WinPolyfillPebState *pPebState,
            IN OUT PUNICODE_STRING StringIn)
        {
            if (StringIn != NULL)
            {
                if (StringIn->Buffer != NULL)
                {
                    RtlFreeHeap(pPebState->LdrpHeap, 0, StringIn->Buffer);
                }

                StringIn->Length = 0;
                StringIn->MaximumLength = 0;
                StringIn->Buffer = NULL;
            }
        }

        BOOL LdrpInitializeLdrDataTableEntry(
            WinPolyfillPebState *pPebState,
            PLDR_DATA_TABLE_ENTRY entry,
            PVOID DllBase,
            PIMAGE_NT_HEADERS headers,
            const UNICODE_STRING &FullDllName)
        {
            entry->FullDllName = FullDllName;
            LdrpResolveBaseDllName(FullDllName, entry->BaseDllName);

            bool CorImage = false;
            bool CorILOnly = false;
            bool ImageDll = false;
            if (headers->FileHeader.Characteristics & IMAGE_FILE_DLL)
            {
                ImageDll = true;
            }
            auto &com = headers->OptionalHeader
                            .DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
            if (com.Size && com.VirtualAddress)
            {
                CorImage = true;
                auto cor = PIMAGE_COR20_HEADER(LPBYTE(DllBase) + com.VirtualAddress);
                if (cor->Flags & ReplacesCorHdrNumericDefines::COMIMAGE_FLAGS_ILONLY)
                {
                    CorILOnly = true;
                }
            }

            auto WindowsVersion = pPebState->SystemVersion;
            if (WindowsVersion >= SYSTEM_WIN11_21H2)
            {
                entry->CheckSum = headers->OptionalHeader.CheckSum;
            }

            if (WindowsVersion >= SYSTEM_NT_4_0_RTM)
            {
                entry->TimeDateStamp = headers->FileHeader.TimeDateStamp;
            }
            entry->SizeOfImage = headers->OptionalHeader.SizeOfImage;
            entry->DllBase = DllBase;

            if (WindowsVersion >= SYSTEM_NT_6_2_RTM)
            {
                // The RbTree needs TimeDateStamp/SizeOfImage/DllBase
                entry->OriginalBase = headers->OptionalHeader.ImageBase;
                NtQuerySystemTime(&entry->LoadTime);

                entry->LoadReason = LoadReasonDynamicLoad;
                NTSTATUS st = STATUS_SUCCESS;
                st = LdrpRbTreeInsert(
                    pPebState->LdrpModuleBaseAddressIndex,
                    &entry->BaseAddressIndexNode,
                    LdrpRbTreeCompareDllBaseNode);
                if (!NT_SUCCESS(st))
                    return FALSE;
                st = LdrpRbTreeInsert(
                    pPebState->LdrpMappingInfoIndex,
                    &entry->MappingInfoIndexNode,
                    LdrpRbTreeCompareTimeStamp);
                if (!NT_SUCCESS(st))
                    return FALSE;

                entry->DdagNode = (decltype(entry->DdagNode))RtlAllocateHeap(
                    pPebState->LdrpHeap, HEAP_ZERO_MEMORY, pPebState->LdrDdagNodeSize);
                if (!entry->DdagNode)
                    return FALSE;

                entry->NodeModuleLink.Flink = &entry->DdagNode->Modules;
                entry->NodeModuleLink.Blink = &entry->DdagNode->Modules;
                entry->DdagNode->Modules.Flink = &entry->NodeModuleLink;
                entry->DdagNode->Modules.Blink = &entry->NodeModuleLink;
                entry->DdagNode->State = LdrModulesReadyToRun;

                entry->LoadNotificationsSent = true;
                entry->InLegacyLists = true;
                entry->InIndexes = true;
                entry->InExceptionTable = false;
            }

            if (WindowsVersion >= SYSTEM_NT_6_0_RTM && WindowsVersion < SYSTEM_NT_6_2_RTM)
            {
                // Windows Vista to Windows 7
                InitializeListHead(&entry->nt_6_0.ForwarderLinks);
                InitializeListHead(&entry->nt_6_0.ServiceTagLinks);
                InitializeListHead(&entry->nt_6_0.StaticLinks);
                if (WindowsVersion >= SYSTEM_NT_6_1_RTM)
                {
                    entry->nt_6_0.OriginalBase = headers->OptionalHeader.ImageBase;
                    NtQuerySystemTime(&entry->nt_6_0.LoadTime);
                }
            }

            if (WindowsVersion < SYSTEM_NT_6_2_RTM)
            {
                entry->LoadCount = 1;
            }
            else
            {
                // Windows 8 and upper
                entry->DdagNode->LoadCount = 1;
                // TODO: Verify this on Windows 8,
                // Why it's 6
                entry->ObsoleteLoadCount = 6;
                if (WindowsVersion < SYSTEM_WIN10_RTM)
                {
                    entry->DdagNode->ReferenceCount = 1;
                }
                else
                {
                    entry->ReferenceCount = 1;
                }
            }
            if (ImageDll)
            {
                ULONG_PTR ep = headers->OptionalHeader.AddressOfEntryPoint;
                if (ep != 0)
                    ep += (ULONG_PTR)DllBase;
                entry->EntryPoint = (VOID *)ep;
            }
            if (WindowsVersion >= SYSTEM_NT_5_1_RTM)
            {
                entry->CorImage = CorImage;
                entry->CorILOnly = CorILOnly;
            }

            entry->ImageDll = ImageDll;
            entry->LoadInProgress = false;
            entry->EntryProcessed = true;

            entry->DontCallForThreads = false;
            entry->ProcessAttachCalled = false;

            auto peb = NtCurrentPeb();
            InsertTailList(&peb->Ldr->InLoadOrderModuleList, &entry->InLoadOrderLinks);
            InsertTailList(
                &peb->Ldr->InMemoryOrderModuleList, &entry->InMemoryOrderLinks);
            InsertTailList(
                &peb->Ldr->InInitializationOrderModuleList, &entry->InProgressLinks);
            InitializeListHead(&entry->HashLinks);
            auto HashEntry = LdrpHashEntryRaw(pPebState, entry->BaseDllName);
            if (pPebState->SystemVersion >= SYSTEM_NT_6_2_RTM)
            {
                entry->BaseNameHashValue = HashEntry;
            }
            /* Insert into hash table */
            InsertTailList(
                &pPebState->LdrpHashTable[HashEntry & LDR_HASH_MASK], &entry->HashLinks);
            return TRUE;
        }

        NTSTATUS RtlFreeDependencies(
            WinPolyfillPebState *pPebState,
            PLDR_DATA_TABLE_ENTRY LdrEntry)
        {
            _LDR_DDAG_NODE *DependentDdgeNode = nullptr;
            PLDR_DATA_TABLE_ENTRY ModuleEntry = nullptr;
            PLDRP_CSLIST_DEPENDENT head = LdrEntry->DdagNode->Dependencies, entry = head;
            HANDLE heap = NtCurrentPeb()->ProcessHeap;
            auto WindowsVersion = pPebState->SystemVersion;
            BOOL IsWin8 =
                WindowsVersion >= SYSTEM_NT_6_2_RTM && WindowsVersion < SYSTEM_WIN10_RTM;
            if (!LdrEntry->DdagNode->Dependencies)
                return STATUS_SUCCESS;

            //find all dependencies and free
            do
            {
                DependentDdgeNode = entry->DependentDdagNode;
                if (DependentDdgeNode->Modules.Flink->Flink !=
                    &DependentDdgeNode->Modules)
                    __fastfail(FAST_FAIL_CORRUPT_LIST_ENTRY);
                ModuleEntry = decltype(ModuleEntry)(
                    (size_t)DependentDdgeNode->Modules.Flink -
                    offsetof(_LDR_DATA_TABLE_ENTRY, NodeModuleLink));
                if (ModuleEntry->DdagNode != DependentDdgeNode)
                    __fastfail(FAST_FAIL_CORRUPT_LIST_ENTRY);
                if (!DependentDdgeNode->IncomingDependencies)
                    __fastfail(FAST_FAIL_CORRUPT_LIST_ENTRY);
                PLDRP_CSLIST_INCOMMING _last = DependentDdgeNode->IncomingDependencies,
                                       _entry = _last;
                _LDR_DDAG_NODE *CurrentDdagNode;
                ULONG State = 0;
                PVOID Cookies;

                //Acquire LoaderLock
                do
                {
                    if (!NT_SUCCESS(LdrLockLoaderLock(
                            LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY, &State, &Cookies)))
                        __fastfail(FAST_FAIL_FATAL_APP_EXIT);
                } while (State != LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED);
                do
                {
                    CurrentDdagNode =
                        (decltype(CurrentDdagNode))((size_t)_entry->IncommingDdagNode &
                                                    ~1);
                    if (CurrentDdagNode == LdrEntry->DdagNode)
                    {
                        //node is head
                        if (_entry == DependentDdgeNode->IncomingDependencies)
                        {
                            //only one node in list
                            if (_entry->NextIncommingEntry ==
                                (PSINGLE_LIST_ENTRY)
                                    DependentDdgeNode->IncomingDependencies)
                            {
                                DependentDdgeNode->IncomingDependencies = nullptr;
                            }
                            else
                            {
                                //find the last node in the list
                                PSINGLE_LIST_ENTRY i = _entry->NextIncommingEntry;
                                while (i->Next != (PSINGLE_LIST_ENTRY)_entry)
                                    i = i->Next;
                                i->Next = _entry->NextIncommingEntry;
                                DependentDdgeNode->IncomingDependencies =
                                    (PLDRP_CSLIST_INCOMMING)_entry->NextIncommingEntry;
                            }
                        }
                        //node is not head
                        else
                        {
                            _last->NextIncommingEntry = _entry->NextIncommingEntry;
                        }
                        break;
                    }

                    //save the last entry
                    if (_last != _entry)
                        _last = (decltype(_last))_last->NextIncommingEntry;
                    _entry = (decltype(_entry))_entry->NextIncommingEntry;
                } while (_entry != _last);

                //free LoaderLock
                LdrUnlockLoaderLock(0, Cookies);

                entry = (decltype(entry))entry->NextDependentEntry;

                //free it
                if (IsWin8)
                {
                    //Update win8 dep count
                    LDR_DDAG_NODE *win8_node = (decltype(win8_node))ModuleEntry->DdagNode;
                    if (!win8_node->DependencyCount)
                        __fastfail(FAST_FAIL_CORRUPT_LIST_ENTRY);
                    --win8_node->DependencyCount;
                    if (!ModuleEntry->DdagNode->LoadCount &&
                        win8_node->ReferenceCount == 1 && !win8_node->DependencyCount)
                    {
                        win8_node->LoadCount = 1;
                        LdrUnloadDll(ModuleEntry->DllBase);
                    }
                }
                else
                {
                    LdrUnloadDll(ModuleEntry->DllBase);
                }
                RtlFreeHeap(heap, 0, LdrEntry->DdagNode->Dependencies);

                //lookup next dependent.
                LdrEntry->DdagNode->Dependencies =
                    (PLDRP_CSLIST_DEPENDENT)(entry == head ? nullptr : entry);
            } while (entry != head);

            return STATUS_SUCCESS;
        }

        void
        LdrEntryDestroy(WinPolyfillPebState *pPebState, LDR_DATA_TABLE_ENTRY *pLdrEntry)
        {
            if (pLdrEntry == nullptr)
                return;
            auto WindowsVersion = pPebState->SystemVersion;
            if (WindowsVersion >= SYSTEM_NT_6_2_RTM)
            {
                RtlFreeDependencies(pPebState, pLdrEntry);

                RtlRbRemoveNode(
                    pPebState->LdrpMappingInfoIndex, &pLdrEntry->MappingInfoIndexNode);
                RtlRbRemoveNode(
                    pPebState->LdrpModuleBaseAddressIndex,
                    &pLdrEntry->BaseAddressIndexNode);

                RtlFreeHeap(pPebState->LdrpHeap, 0, pLdrEntry->DdagNode);
            }

            if (WindowsVersion >= SYSTEM_NT_6_0_RTM && WindowsVersion < SYSTEM_NT_6_2_RTM)
            {
                PLIST_ENTRY head = &pLdrEntry->nt_6_0.ForwarderLinks, next = head->Flink;
                while (head != next)
                {
                    PLDR_DATA_TABLE_ENTRY dep = *(decltype(&dep))((size_t *)next + 2);
                    LdrUnloadDll(dep->DllBase);
                    next = next->Flink;
                    RtlFreeHeap(pPebState->LdrpHeap, 0, next->Blink);
                }
            }
            LdrpFreeUnicodeString(pPebState, &pLdrEntry->FullDllName);
            if (pLdrEntry->DllBase)
            {
                NtUnmapViewOfSection(NtCurrentProcess(), pLdrEntry->DllBase);
            }

            if (pLdrEntry->InMemoryOrderLinks.Flink != nullptr)
            {
                RemoveEntryList(&pLdrEntry->InLoadOrderLinks);
                RemoveEntryList(&pLdrEntry->InInitializationOrderLinks);
                RemoveEntryList(&pLdrEntry->InMemoryOrderLinks);
                RemoveEntryList(&pLdrEntry->HashLinks);
            }
            RtlFreeHeap(pPebState->LdrpHeap, 0, pLdrEntry);
        }

        void LdrEntryDetach(
            WinPolyfillPebState *pPebState,
            LDR_DATA_TABLE_ENTRY *pLdrEntry,
            PVOID lpReserved)
        {
            ::internal::CallInitRoutine(pLdrEntry, DLL_PROCESS_DETACH, lpReserved);
            LdrEntryDestroy(pPebState, pLdrEntry);
        }

        void LdrpEntrySetup(
            WinPolyfillPebState *pPebState,
            LDR_DATA_TABLE_ENTRY *pNewLdrEntry,
            PVOID DllBase,
            PIMAGE_NT_HEADERS pNtHeaders,
            wchar_t *FullDllName)
        {
            UNICODE_STRING FullDllNameUnicode = MakeNtString(FullDllName);
            UNICODE_STRING FullDllNameUnicodeLdrp;
            LdrpCopyUnicodeString(
                pPebState, &FullDllNameUnicodeLdrp, &FullDllNameUnicode);
            LdrpInitializeLdrDataTableEntry(
                pPebState, pNewLdrEntry, DllBase, pNtHeaders, FullDllNameUnicodeLdrp);
        }

        PVOID __fastcall LdrpGetDataDirectory(
            LPBYTE pBaseAddress,
            PIMAGE_NT_HEADERS pNtHeaders,
            __in ULONG dwDirectory,
            __out PULONG pSize)
        {
            auto &_DataDirectory = pNtHeaders->OptionalHeader.DataDirectory[dwDirectory];

            *pSize = _DataDirectory.Size;
            if (_DataDirectory.Size == 0 || _DataDirectory.VirtualAddress == 0)
                return nullptr;

            return PBYTE(pBaseAddress) + _DataDirectory.VirtualAddress;
        }

        NTSTATUS
        LdrpGetIat(
            LPBYTE DllBase,
            PIMAGE_NT_HEADERS pNtHeaders,
            PVOID *pIATBase,
            SIZE_T *pIATSize)
        {

            ULONG LittleIATSize;
            PVOID IATBase;

            // Determine the location and size of the IAT. If the linker did
            // not tell use explicitly, then use the location and size of the
            // image section that contains the import table.
            IATBase = LdrpGetDataDirectory(
                DllBase, pNtHeaders, IMAGE_DIRECTORY_ENTRY_IAT, &LittleIATSize);
            if (IATBase == nullptr)
            {
                PIMAGE_SECTION_HEADER NtSection = IMAGE_FIRST_SECTION(pNtHeaders);
                ULONG Rva =
                    pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                        .VirtualAddress;
                if (Rva != 0)
                {
                    for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
                    {
                        if (Rva >= NtSection->VirtualAddress &&
                            Rva < (NtSection->VirtualAddress + NtSection->SizeOfRawData))
                        {
                            IATBase = (PVOID)(DllBase + NtSection->VirtualAddress);

                            LittleIATSize = NtSection->Misc.VirtualSize;
                            if (LittleIATSize == 0)
                            {
                                LittleIATSize = NtSection->SizeOfRawData;
                            }
                            break;
                        }

                        ++NtSection;
                    }
                }
            }
            *pIATSize = LittleIATSize;
            *pIATBase = IATBase;
            return STATUS_SUCCESS;
        }

        NTSTATUS
        LdrpResolveImportTable(_In_ LPBYTE base, _In_ PIMAGE_NT_HEADERS pNtHeaders)
        {
            NTSTATUS status = STATUS_SUCCESS;
            do
            {
                __try
                {
                    PVOID IATBase;
                    SIZE_T IATSize;
                    DWORD count = 0;
                    ULONG OldProtect;
                    status = LdrpGetIat(base, pNtHeaders, &IATBase, &IATSize);
                    if (!NT_SUCCESS(status))
                        break;
                    if (IATBase == nullptr || IATSize == 0)
                        break;
                    status = NtProtectVirtualMemory(
                        NtCurrentProcess(),
                        &IATBase,
                        &IATSize,
                        PAGE_READWRITE,
                        &OldProtect);
                    if (!NT_SUCCESS(status))
                        break;
                    PIMAGE_DATA_DIRECTORY dir =
                        &pNtHeaders->OptionalHeader
                             .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
                    PIMAGE_IMPORT_DESCRIPTOR importDesc = nullptr;

                    if (dir && dir->Size)
                    {
                        importDesc = PIMAGE_IMPORT_DESCRIPTOR(base + dir->VirtualAddress);
                    }

                    if (importDesc)
                    {
                        for (; importDesc->Name; ++importDesc)
                        {
                            LPCSTR DllNameASCII = (LPCSTR)(base + importDesc->Name);
                            uintptr_t *thunkRef;
                            PVOID *funcRef;
                            PVOID handle = nullptr;
                            ANSI_STRING AnsiString;
                            UNICODE_STRING DllNameUnicode;
                            RtlInitAnsiString(&AnsiString, DllNameASCII);
                            status = RtlAnsiStringToUnicodeString(
                                &DllNameUnicode, &AnsiString, TRUE);
                            if (!NT_SUCCESS(status))
                                break;
                            status = LdrGetDllHandle(
                                nullptr, nullptr, &DllNameUnicode, &handle);
                            RtlFreeUnicodeString(&DllNameUnicode);
                            if (!NT_SUCCESS(status))
                                break;
                            if (!handle)
                            {
                                status = STATUS_DLL_NOT_FOUND;
                                break;
                            }

                            thunkRef =
                                (uintptr_t *)(base + (importDesc->OriginalFirstThunk
                                                          ? importDesc->OriginalFirstThunk
                                                          : importDesc->FirstThunk));
                            funcRef = (PVOID *)(base + importDesc->FirstThunk);
                            while (*thunkRef)
                            {
                                ANSI_STRING ProcedureNameANSI;
                                PANSI_STRING ProcedureName = nullptr;
                                ULONG ProcedureNumber = 0;
                                bool IsOrdinal = IMAGE_SNAP_BY_ORDINAL(*thunkRef);
                                if (IsOrdinal)
                                {
                                    ProcedureNumber = IMAGE_ORDINAL(*thunkRef);
                                }
                                else
                                {
                                    RtlInitAnsiString(
                                        &ProcedureNameANSI,
                                        (LPCSTR)PIMAGE_IMPORT_BY_NAME(base + (*thunkRef))
                                            ->Name);
                                    ProcedureName = &ProcedureNameANSI;
                                }

                                status = LdrGetProcedureAddress(
                                    (HMODULE)handle,
                                    ProcedureName,
                                    ProcedureNumber,
                                    funcRef);
                                if (!NT_SUCCESS(status))
                                {
                                    break;
                                }
                                if (!*funcRef)
                                {
                                    status = STATUS_ENTRYPOINT_NOT_FOUND;
                                    break;
                                }
                                ++thunkRef;
                                ++funcRef;
                            }

                            if (!NT_SUCCESS(status))
                                break;
                        }
                    }
                    NtProtectVirtualMemory(
                        NtCurrentProcess(), &IATBase, &IATSize, OldProtect, &OldProtect);
                    NtFlushInstructionCache(NtCurrentProcess(), IATBase, IATSize);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    status = GetExceptionCode();
                }
            } while (false);
            return status;
        }

        NTSTATUS
        LdrRelocateImage(IN PVOID NewBase, PIMAGE_NT_HEADERS pNtHeaders)
        {
            ULONG TotalCountBytes = 0;
            // Locate the relocation section.
            PIMAGE_BASE_RELOCATION NextBlock =
                (PIMAGE_BASE_RELOCATION)LdrpGetDataDirectory(
                    (LPBYTE)NewBase,
                    pNtHeaders,
                    IMAGE_DIRECTORY_ENTRY_BASERELOC,
                    &TotalCountBytes);

            // It is possible for a file to have no relocations, but the relocations
            // must not have been stripped.
            if (!NextBlock || !TotalCountBytes)
            {
                return STATUS_SUCCESS;
            }

            NTSTATUS Status = STATUS_SUCCESS;
            // If the image has a relocation table, then apply the specified fixup
            // information to the image.
            LONGLONG Diff = (PCHAR)NewBase - (PCHAR)pNtHeaders->OptionalHeader.ImageBase;
            while (TotalCountBytes)
            {
                ULONG SizeOfBlock = NextBlock->SizeOfBlock;
                TotalCountBytes -= SizeOfBlock;
                SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
                SizeOfBlock /= sizeof(USHORT);
                PUSHORT NextOffset =
                    (PUSHORT)((PCHAR)NextBlock + sizeof(IMAGE_BASE_RELOCATION));

                ULONG_PTR VA = (ULONG_PTR)NewBase + NextBlock->VirtualAddress;

                if (!(NextBlock =
                          LdrProcessRelocationBlock(VA, SizeOfBlock, NextOffset, Diff)))
                {
                    Status = STATUS_INVALID_IMAGE_FORMAT;
                    break;
                }
            }
            return Status;
        }

        NTSTATUS
        LdrpSetProtection(IN PVOID Base, PIMAGE_NT_HEADERS pNtHeaders, IN BOOLEAN Reset)
        {
            HANDLE CurrentProcessHandle = NtCurrentProcess();
            SIZE_T RegionSize;
            ULONG NewProtect, OldProtect;
            NTSTATUS st;

            auto SectionHeader =
                (PIMAGE_SECTION_HEADER)((ULONG_PTR)pNtHeaders + sizeof(ULONG) +
                                        sizeof(IMAGE_FILE_HEADER) +
                                        pNtHeaders->FileHeader.SizeOfOptionalHeader);

            for (WORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections;
                 ++i, ++SectionHeader)
            {
                if (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE) &&
                    (SectionHeader->SizeOfRawData))
                {
                    //
                    // Object isn't writeable and has a non-zero on disk size, change it.
                    //
                    if (Reset)
                    {
                        if (SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE)
                        {
                            NewProtect = PAGE_EXECUTE;
                        }
                        else
                        {
                            NewProtect = PAGE_READONLY;
                        }
                        NewProtect |=
                            (SectionHeader->Characteristics & IMAGE_SCN_MEM_NOT_CACHED)
                                ? PAGE_NOCACHE
                                : 0;
                    }
                    else
                    {
                        NewProtect = PAGE_READWRITE;
                    }
                    PVOID VirtualAddress =
                        (PVOID)((ULONG_PTR)Base + SectionHeader->VirtualAddress);
                    RegionSize = SectionHeader->SizeOfRawData;

                    if (RegionSize != 0)
                    {
                        st = NtProtectVirtualMemory(
                            CurrentProcessHandle,
                            &VirtualAddress,
                            &RegionSize,
                            NewProtect,
                            &OldProtect);

                        if (!NT_SUCCESS(st))
                        {
                            return st;
                        }
                    }
                }
            }

            if (Reset)
            {
                NtFlushInstructionCache(NtCurrentProcess(), NULL, 0);
            }
            return STATUS_SUCCESS;
        }

        NTSTATUS LdrpLink(
            const wchar_t *NtFullDllName,
            HANDLE SectionHandle,
            PIMAGE_NT_HEADERS pNtHeaders,
            PVOID &ViewBase,
            SIZE_T &ViewSize)
        {
            NTSTATUS st;
            do
            {
                st = LdrpSetProtection(ViewBase, pNtHeaders, FALSE);
                if (!NT_SUCCESS(st))
                {
                    break;
                }
                __try
                {
                    st = LdrRelocateImage(ViewBase, pNtHeaders);
                }
                __except (EXCEPTION_EXECUTE_HANDLER)
                {
                    st = GetExceptionCode();
                }
                PTEB Teb = NtCurrentTeb();
                auto ArbitraryUserPointer = Teb->NtTib.ArbitraryUserPointer;
                Teb->NtTib.ArbitraryUserPointer = (PVOID)NtFullDllName;
                // arrange for debugger to pick up the image name
                st = NtMapViewOfSection(
                    SectionHandle,
                    NtCurrentProcess(),
                    (PVOID *)&ViewBase,
                    0L,
                    0L,
                    NULL,
                    &ViewSize,
                    ViewShare,
                    0L,
                    PAGE_READWRITE);
                Teb->NtTib.ArbitraryUserPointer = ArbitraryUserPointer;
                if (st = STATUS_CONFLICTING_ADDRESSES)
                {
                    st = STATUS_SUCCESS;
                }
                if (!NT_SUCCESS(st))
                {
                    break;
                }
                st = LdrpSetProtection(ViewBase, pNtHeaders, TRUE);
            } while (0);
            return st;
        }

        NTSTATUS LdrLoadDllInit(
            WinPolyfillPebState *pPebState,
            HANDLE SectionHandle,
            PVOID ViewBase,
            SIZE_T ViewSize,
            _In_ wchar_t *NtFullDllName,
            _Out_ PVOID *DllHandle)
        {
            auto st = STATUS_SUCCESS;
            LDR_DATA_TABLE_ENTRY *pNewLdrEntry = NULL;
            do
            {
                auto pNtHeaders = RtlImageNtHeader(ViewBase);
                if (!pNtHeaders)
                {
                    st = STATUS_INVALID_PARAMETER;
                    break;
                }
                st = LdrpLink(
                    NtFullDllName, SectionHandle, pNtHeaders, ViewBase, ViewSize);
                if (!NT_SUCCESS(st))
                    break;
                pNewLdrEntry = LdrpEntryAlloc(pPebState);
                if (pNewLdrEntry == nullptr)
                {
                    break;
                }
                st = LdrpResolveImportTable((LPBYTE)ViewBase, pNtHeaders);
                if (!NT_SUCCESS(st))
                    break;
                LdrpEntrySetup(
                    pPebState, pNewLdrEntry, ViewBase, pNtHeaders, NtFullDllName);
                if (!CallInitRoutine(pNewLdrEntry, DLL_PROCESS_ATTACH, nullptr))
                {
                    st = STATUS_DLL_INIT_FAILED;
                }

            } while (0);
            if (!NT_SUCCESS(st))
            {
                if (pNewLdrEntry)
                {
                    LdrEntryDestroy(pPebState, pNewLdrEntry);
                }
                else
                {
                    NtUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                }
            }
            else
            {
                *DllHandle = pNewLdrEntry->DllBase;
            }
            return st;
        }

        NTSTATUS LdrLoadDllMemoryNoLock(
            WinPolyfillPebState *pPebState,
            _In_ wchar_t *NtFullDllName,
            PVOID ViewBaseInput,
            size_t ViewSizeInput,
            _Out_ PVOID *DllHandle)
        {
            auto st = STATUS_SUCCESS;
            SIZE_T ViewSize = 0;
            PVOID ViewBase = NULL;
            *DllHandle = nullptr;
            HANDLE SectionHandle = NULL;
            do
            {
                PIMAGE_NT_HEADERS pNtHeadersInput = RtlImageNtHeader(ViewBaseInput);
                if (pNtHeadersInput == nullptr)
                {
                    st = STATUS_INVALID_PARAMETER;
                    break;
                }
                LARGE_INTEGER SectionSize = {pNtHeadersInput->OptionalHeader.SizeOfImage};
                st = NtCreateSection(
                    &SectionHandle,
                    SECTION_MAP_READ | SECTION_MAP_WRITE | SECTION_MAP_EXECUTE,
                    NULL,
                    (PLARGE_INTEGER)&SectionSize,
                    PAGE_EXECUTE_READWRITE,
                    SEC_COMMIT,
                    NULL);
                if (!NT_SUCCESS(st))
                    break;
                ViewBase = NULL;
                ViewSize = 0;
                st = NtMapViewOfSection(
                    SectionHandle,
                    NtCurrentProcess(),
                    (PVOID *)&ViewBase,
                    NULL,
                    NULL,
                    NULL,
                    &ViewSize,
                    ViewShare,
                    NULL,
                    PAGE_EXECUTE_READWRITE);
                if (!NT_SUCCESS(st))
                    break;
                // Copy headers
                RtlCopyMemory(
                    ViewBase,
                    ViewBaseInput,
                    pNtHeadersInput->OptionalHeader.SizeOfHeaders);
                // Copy sections
                PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeadersInput);
                for (DWORD i = 0; i < pNtHeadersInput->FileHeader.NumberOfSections;
                     ++i, ++section)
                {
                    if (section->SizeOfRawData)
                    {
                        LPBYTE DestAddr = (LPBYTE)ViewBase + section->VirtualAddress;
                        RtlCopyMemory(
                            DestAddr,
                            LPBYTE(ViewBaseInput) + section->PointerToRawData,
                            section->SizeOfRawData);
                    }
                }
                st = LdrLoadDllInit(
                    pPebState,
                    SectionHandle,
                    ViewBase,
                    ViewSize,
                    NtFullDllName,
                    DllHandle);
            } while (0);
            if (SectionHandle)
            {
                NtClose(SectionHandle);
            }
            return st;
        }
    } // namespace
} // namespace internal
