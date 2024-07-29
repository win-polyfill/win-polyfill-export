/*++
Copyright (c) Microsoft Corporation.
Licensed under the MIT license.
--*/

#pragma once

#include "ntcsrmsg.h"

NTSYSAPI
NTSTATUS
NTAPI
CsrClientCallServer(
    PCSR_API_MSG m,
    PCSR_CAPTURE_HEADER CaptureBuffer OPTIONAL,
    ULONG ApiNumber,
    ULONG ArgLength
);

NTSYSAPI
PCSR_CAPTURE_HEADER
NTAPI
CsrAllocateCaptureBuffer(
    IN ULONG CountMessagePointers,
    IN ULONG Size
    );

NTSYSAPI
VOID
NTAPI
CsrCaptureMessageBuffer(
    IN OUT PCSR_CAPTURE_HEADER CaptureBuffer,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length,
    OUT PVOID *CapturedBuffer
    );

NTSYSAPI
VOID
NTAPI
CsrFreeCaptureBuffer(
    IN PCSR_CAPTURE_HEADER CaptureBuffer
    );
