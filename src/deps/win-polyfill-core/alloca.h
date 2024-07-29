#pragma once

#include "nt.h"

namespace internal
{
    namespace
    {

        _Check_return_ _Ret_maybenull_ _Post_writable_byte_size_(_cbBytes) static void
            *__fastcall Alloc(_In_ size_t _cbBytes, DWORD _fFlags = 0)
        {
            return RtlAllocateHeap(
                ((TEB *)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap,
                _fFlags,
                _cbBytes);
        }

        _Check_return_ _Ret_maybenull_
        _Post_writable_byte_size_(_cbBytes) static void *__fastcall ReAlloc(
            _Pre_maybenull_ _Post_invalid_ void *_pAddress,
            _In_ size_t _cbBytes,
            DWORD _fFlags = 0)
        {
            auto _hProcessHeap =
                ((TEB *)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap;
            if (_pAddress)
            {
                return RtlReAllocateHeap(_hProcessHeap, _fFlags, _pAddress, _cbBytes);
            }
            else
            {
                return RtlAllocateHeap(_hProcessHeap, _fFlags, _cbBytes);
            }
        }

        static void __fastcall Free(
            _Pre_maybenull_ _Post_invalid_ void *_pAddress,
            DWORD _fFlags = 0)
        {
            if (_pAddress)
                RtlFreeHeap(
                    ((TEB *)NtCurrentTeb())->ProcessEnvironmentBlock->ProcessHeap,
                    _fFlags,
                    _pAddress);
        }

        template <typename Type>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New()
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type();

            return _pType;
        }

        template <typename Type, typename A>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New(A &&a)
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type(a);

            return _pType;
        }

        template <typename Type, typename A, typename B>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New(A &&a, B &&b)
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type(a, b);

            return _pType;
        }

        template <typename Type, typename A, typename B, typename C>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New(A &&a, B &&b, C &&c)
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type(a, b, c);

            return _pType;
        }

        template <typename Type, typename A, typename B, typename C, typename D>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New(A &&a, B &&b, C &&c, D &&d)
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type(a, b, c, d);

            return _pType;
        }

        template <
            typename Type,
            typename A,
            typename B,
            typename C,
            typename D,
            typename F>
        _Success_(return != NULL) _Check_return_ _Ret_maybenull_ _CRTALLOCATOR
            inline Type *__fastcall New(A &&a, B &&b, C &&c, D &&d, F &&f)
        {
            Type *_pType = (Type *)Alloc(sizeof(Type));
            if (_pType)
                new (_pType) Type(a, b, c, d, f);

            return _pType;
        }

        template <typename T>
        inline void __fastcall Delete(_Pre_maybenull_ _Post_invalid_ T *_p)
        {
            if (_p)
            {
                _p->~T();
                Free(_p);
            }
        }
    } // namespace

} // namespace internal
