#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <vector>

#pragma once

struct ArtTypeInfo
{
    std::string name;
    std::string raw_name;
    size_t size;
    size_t alignment;
    bool is_pointer;
    int8_t function_id;
};

struct FunctionInfo
{
    std::string ModuleName;
    std::string FunctionName;
    // 0 cdecl 1 stdcall
    int CallingConventionId;

    // 0 is the return type, 1..N is arguments
    std::vector<ArtTypeInfo> ArgTypes;
};

extern std::vector<FunctionInfo> module_functions;

void gen_functions();
