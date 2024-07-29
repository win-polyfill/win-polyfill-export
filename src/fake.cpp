#include <windows.h>

#include <perflib.h>
#include <shellscalingapi.h>

extern "C" HRESULT APIENTRY
PathCchAddBackslash(_Inout_updates_(cchPath) PWSTR pszPath, _In_ size_t cchPath)
{
    return 0;
}

extern "C" HRESULT STDAPICALLTYPE SetProcessDpiAwareness(_In_ PROCESS_DPI_AWARENESS value)
{
    return 0;
}
