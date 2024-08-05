#pragma once
#include <vector>

#include "gen-exports-api.h"

struct FunctionInfo
{
    const char* ModuleName;
    const char* FunctionName;
    // 0 cdecl 1 stdcall
    int CallingConventionId;

    // 0 is the return type, 1..N is arguments
    std::vector<ArtTypeInfo> ArgTypes;
};

extern std::vector<FunctionInfo> module_functions;
