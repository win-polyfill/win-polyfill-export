#include "gen-exports-function-info.h"

std::vector<FunctionInfo> module_functions;

struct FunctionInfo *function_info_alloca(const char *module_name, const char *function_name, int CallingConventionId)
{
    FunctionInfo x;
    x.ModuleName = module_name;
    x.FunctionName = function_name;
    x.CallingConventionId = CallingConventionId;
    module_functions.push_back(x);
    return &module_functions.back();
}

void function_info_add_parameter(struct FunctionInfo *x, ArtTypeInfo *info)
{
    x->ArgTypes.push_back(*info);
}
