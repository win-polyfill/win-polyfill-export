#include <stdint.h>

#pragma once

struct ArtTypeInfo
{
    const char *name;
    const char *raw_name;
    size_t size;
    size_t alignment;
    bool is_pointer;
    int8_t function_id;
};

struct FunctionInfo;

struct FunctionInfo *function_info_alloca(
    const char *module_name,
    const char *function_name,
    int CallingConventionId);

void function_info_add_parameter(struct FunctionInfo *, ArtTypeInfo *info);

void gen_functions();
