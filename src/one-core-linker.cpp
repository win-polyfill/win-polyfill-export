#include <windows.h>
#include <netfw.h>

int main()
{
    DWORD x = NetworkIsolationEnumAppContainers(0, 0, nullptr);
    return 0;
}
