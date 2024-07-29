/*++
Copyright (c) Microsoft Corporation.
Licensed under the MIT license.
--*/

#pragma once

#include "ntlpcapi.h"

struct _CSR_CAPTURE_HEADER;
typedef struct _CSR_CAPTURE_HEADER CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;
typedef struct _CSR_CAPTURE_HEADER {
    ULONG Length;
    PCSR_CAPTURE_HEADER RelatedCaptureBuffer;
    ULONG CountMessagePointers;
    PCHAR FreeSpace;
    ULONG_PTR MessagePointerOffsets[1]; // Offsets within CSR_API_MSG of pointers
} CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef struct _CSR_API_CONNECTINFO {
    IN ULONG ExpectedVersion;
    OUT ULONG CurrentVersion;
    OUT HANDLE ObjectDirectory;
    OUT PVOID SharedSectionBase;
    OUT PVOID SharedStaticServerData;
    OUT PVOID SharedSectionHeap;
    OUT ULONG DebugFlags;
    OUT ULONG SizeOfPebData;
    OUT ULONG SizeOfTebData;
    OUT ULONG NumberOfServerDllNames;
    OUT HANDLE ServerProcessId;
} CSR_API_CONNECTINFO, *PCSR_API_CONNECTINFO;

typedef struct _CSR_CLIENTCONNECT_MSG {
    IN ULONG ServerDllIndex;
    IN OUT PVOID ConnectionInformation;
    IN OUT ULONG ConnectionInformationLength;
} CSR_CLIENTCONNECT_MSG, *PCSR_CLIENTCONNECT_MSG;

typedef struct _CSR_THREADCONNECT_MSG {
    HANDLE SectionHandle;
    HANDLE EventPairHandle;
    OUT PCHAR MessageStack;
    OUT ULONG MessageStackSize;
    OUT ULONG RemoteViewDelta;
} CSR_THREADCONNECT_MSG, *PCSR_THREADCONNECT_MSG;

typedef struct _CSR_PROFILE_CONTROL_MSG {
    IN ULONG ProfileControlFlag;
} CSR_PROFILE_CONTROL_MSG, *PCSR_PROFILE_CONTROL_MSG;

typedef struct _CSR_IDENTIFY_ALERTABLE_MSG {
    IN CLIENT_ID ClientId;
} CSR_IDENTIFY_ALERTABLE_MSG, *PCSR_IDENTIFY_ALERTABLE_MSG;

typedef struct _CSR_SETPRIORITY_CLASS_MSG {
    IN HANDLE ProcessHandle;
    IN ULONG PriorityClass;
} CSR_SETPRIORITY_CLASS_MSG, *PCSR_SETPRIORITY_CLASS_MSG;

typedef struct _CSR_API_MSG {
    PORT_MESSAGE h;
    union {
        CSR_API_CONNECTINFO ConnectionRequest;
        struct {
            PCSR_CAPTURE_HEADER CaptureBuffer;
            ULONG ApiNumber;
            ULONG ReturnValue;
            ULONG Reserved;
            union {
                CSR_CLIENTCONNECT_MSG ClientConnect;
                CSR_THREADCONNECT_MSG ThreadConnect;
                CSR_PROFILE_CONTROL_MSG ProfileControl;
                CSR_IDENTIFY_ALERTABLE_MSG IdentifyAlertable;
                CSR_SETPRIORITY_CLASS_MSG PriorityClass;
                ULONG_PTR ApiMessageData[39];
            } u;
        };
    };
} CSR_API_MSG, *PCSR_API_MSG;

#define CONSRV_SERVERDLL_INDEX          2
#define CONSRV_FIRST_API_NUMBER         512

#define CSR_MAKE_API_NUMBER(DllIndex, ApiIndex)                                          \
    (ULONG)(((DllIndex) << 16) | (ApiIndex))
