#include "gen-exports-api.h"

void write(FILE *f, const std::string &str) { fwrite(str.data(), 1, str.size(), f); }

void write_argument(FILE *f, const FunctionInfo &func, int i)
{
    char buffer[128];
    auto &arg = func.ArgTypes[i];
    if (arg.function_id == 0 || arg.function_id == 1)
    {
        write(f, "/* ");
        write(f, arg.name);
        write(f, " */");
        write(f, " void *");
    }
    else
    {
        const std::string enum_prefix = "enum ";
        auto enum_prefix_size = enum_prefix.size();
        bool is_enum = false;
        if (arg.name.size() >= enum_prefix_size)
        {
            is_enum = memcmp(arg.name.data(), enum_prefix.data(), enum_prefix_size) == 0;
        }
        if (is_enum)
        {
            write(f, arg.name.c_str() + enum_prefix_size);
        }
        else
        {
            write(f, arg.name);
        }
    }
    if (i == 0)
    {
        // this is return value;
    }
    else
    {
        if (arg.size > 4 && arg.size % 4 != 0)
        {
            fprintf(stderr, "Something is wrong\n");
        }
        write(f, " ");
        write(f, "a");
        write(f, itoa(i, buffer, 10));
    }
}

const char *get_call_conv(int conv_id)
{
    if (conv_id == 0)
        return "__cdecl";
    if (conv_id == 1)
        return "__stdcall";
    if (conv_id == 2)
        return "__fastcall";
    return "__unknow_conv";
}

int main(void)
{
    gen_functions();
    auto f_thunks = fopen("override-api-thunks.h", "w");
    auto f = fopen("override-api.cpp", "w");
    auto f_def = fopen("override-api.def", "w");
    write(f, "#include \"override-api.h\"\n");
    fprintf(f_def, "LIBRARY \"override-api.dll\"\n");
    fprintf(f_def, "EXPORTS\n");
    for (int i = 0; i < module_functions.size(); ++i)
    {
        auto &func = module_functions[i];

        write(f, "extern \"C\" ");

        write_argument(f, func, 0);

        fprintf(f, " %s ", get_call_conv(func.CallingConventionId));

        int total_size = 0;
        for (int i = 1; i < func.ArgTypes.size(); ++i)
        {
            auto &arg = func.ArgTypes[i];
            if (arg.size <= 4)
            {
                total_size += 4;
            }
            else
            {
                total_size += arg.size;
            }
        }

        fprintf(
            f_thunks,
            "__DEFINE_THUNKS_FUNC_SIZE(%s, %s, %d)\n",
            get_call_conv(func.CallingConventionId),
            func.FunctionName.c_str(),
            total_size);

        fprintf(f, "wp_%s", func.FunctionName.c_str());
        fprintf(f_def, "    wp_%s\n", func.FunctionName.c_str());
        auto sz = func.ArgTypes.size();
        write(f, "(");
        switch (sz)
        {
        case 1:
            write(f, "void");
            break;
        default:
            for (int i = 1; i < func.ArgTypes.size(); ++i)
            {
                if (i > 1)
                    write(f, ", ");
                write_argument(f, func, i);
            }
            break;
        }
        write(f, ")");
        write(f, "\n");
        write(f, "{\n");
        fprintf(
            f,
            "    auto pfn = (decltype(&wp_%s))get_function(&functions.%s);\n",
            func.FunctionName.c_str(),
            func.FunctionName.c_str());
        fprintf(f, "    return pfn(");
        switch (sz)
        {
        case 1:
            write(f, "");
            break;
        default:
            for (int i = 1; i < func.ArgTypes.size(); ++i)
            {
                char buffer[128];
                if (i > 1)
                    write(f, ", ");
                write(f, "a");
                write(f, itoa(i, buffer, 10));
            }
            break;
        }
        write(f, ");\n");
        write(f, "}\n");
    }
    fclose(f);
    fclose(f_def);
    fclose(f_thunks);
    printf("gen exports finished\n");
    return 0;
}
