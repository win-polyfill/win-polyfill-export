// C++ program for building suffix array of a given text
#include <algorithm>
#include <cstring>
#include <iostream>

#include <windows.h>

#include <pathcch.h>
#include <perflib.h>
#include <shellscalingapi.h>

// Driver program to test above functions
int main()
{
    wchar_t c[128] = L"abc";
    PathCchAddBackslash(c, sizeof(c));
    PROCESS_DPI_AWARENESS v = PROCESS_DPI_UNAWARE;
    SetProcessDpiAwareness(v);
    return 0;
}
