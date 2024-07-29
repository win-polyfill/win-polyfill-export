#pragma once

#include "win-polyfill-core/nt.h"

namespace internal
{
    namespace
    {
        CONST UCHAR FillMask[] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF};
        CONST UCHAR ZeroMask[] = {0xFF, 0xFE, 0xFC, 0xF8, 0xf0, 0xe0, 0xc0, 0x80, 0x00};

        __forceinline VOID RtlInitializeBitMap(
            IN PRTL_BITMAP BitMapHeader,
            IN PULONG BitMapBuffer,
            IN ULONG SizeOfBitMap)
        {
            BitMapHeader->SizeOfBitMap = SizeOfBitMap;
            BitMapHeader->Buffer = BitMapBuffer;
        }

        __forceinline VOID RtlClearBit(IN PRTL_BITMAP BitMapHeader, IN ULONG BitPos)
        {
            PCHAR ByteAddress;
            ULONG ShiftCount;

            ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitPos >> 3);
            ShiftCount = BitPos & 0x7;
            *ByteAddress &= (CHAR)(~(1 << ShiftCount));
        }

        VOID RtlClearBits(
            IN PRTL_BITMAP BitMapHeader,
            IN ULONG StartingIndex,
            IN ULONG NumberToClear)
        {
            if (NumberToClear == 0)
            {
                return;
            }
            PCHAR CurrentByte = ((PCHAR)BitMapHeader->Buffer) + (StartingIndex / 8);
            ULONG BitOffset = StartingIndex % 8;
            if ((BitOffset + NumberToClear) <= 8)
            {

                *CurrentByte &= ~(internal::FillMask[NumberToClear] << BitOffset);
            }
            else
            {
                if (BitOffset > 0)
                {
                    *CurrentByte &= internal::FillMask[BitOffset];
                    CurrentByte += 1;
                    NumberToClear -= 8 - BitOffset;
                }
                if (NumberToClear > 8)
                {
                    RtlZeroMemory(CurrentByte, NumberToClear / 8);
                    CurrentByte += NumberToClear / 8;
                    NumberToClear %= 8;
                }
                if (NumberToClear > 0)
                {
                    *CurrentByte &= internal::ZeroMask[NumberToClear];
                }
            }
        }

        __forceinline VOID RtlSetBit(IN PRTL_BITMAP BitMapHeader, IN ULONG BitNumber)
        {
            PCHAR ByteAddress;
            ULONG ShiftCount;
            ByteAddress = (PCHAR)BitMapHeader->Buffer + (BitNumber >> 3);
            ShiftCount = BitNumber & 0x7;
            *ByteAddress |= (CHAR)(1 << ShiftCount);
        }

        VOID RtlSetBits(
            IN PRTL_BITMAP BitMapHeader,
            IN ULONG StartingIndex,
            IN ULONG NumberToSet)
        {
            if (NumberToSet == 0)
            {
                return;
            }
            PCHAR CurrentByte = ((PCHAR)BitMapHeader->Buffer) + (StartingIndex / 8);
            ULONG BitOffset = StartingIndex % 8;
            if ((BitOffset + NumberToSet) <= 8)
            {
                *CurrentByte |= (internal::FillMask[NumberToSet] << BitOffset);
            }
            else
            {
                if (BitOffset > 0)
                {
                    *CurrentByte |= internal::ZeroMask[BitOffset];
                    CurrentByte += 1;
                    NumberToSet -= 8 - BitOffset;
                }
                if (NumberToSet > 8)
                {
                    RtlFillMemory(CurrentByte, NumberToSet / 8, 0xff);
                    CurrentByte += NumberToSet / 8;
                    NumberToSet %= 8;
                }
                if (NumberToSet > 0)
                {
                    *CurrentByte |= internal::FillMask[NumberToSet];
                }
            }
        }

        __forceinline ULONG RtlFindClearBitsAndSet(
            IN PRTL_BITMAP BitMapHeader,
            IN ULONG NumberToFind,
            IN ULONG HintIndex)
        {
            ULONG StartingIndex = RtlFindClearBits(BitMapHeader, NumberToFind, HintIndex);
            if (StartingIndex != 0xffffffff)
            {
                internal::RtlSetBits(BitMapHeader, StartingIndex, NumberToFind);
            }
            return StartingIndex;
        }

    } // namespace

} // namespace internal
