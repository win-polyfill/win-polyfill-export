
#include "win-polyfill-core/crt-macros.h"

//չ�����к���������
#define DEFINE_THUNK(_CONVENTION, _FUNCTION, _SIZE)                         \
    void _CONVENTION _FUNCTION(_CRT_PARAMS_EXPAND(_SIZE)) {}

#include "ntdll-basic.h"

#undef DEFINE_THUNK
