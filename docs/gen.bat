set path=C:\CI-Tools\Auxillary\Dependencies_x64_Release;%PATH%

mkdir gens
mkdir gens\x86
mkdir gens\x64

for %%x in (
ntdll
kernel32
advapi32
cfgmgr32
crypt32
dbghelp
esent
gdi32
iphlpapi
netapi32
ole32
pdh
powrprof
psapi
setupapi
shell32
shlwapi
user32
userenv
version
winhttp
ws2_32

bcrypt
bcryptprimitives
bluetoothapis
d3d11
d3d12
d3d9
dwmapi
dwrite
dxgi
dxva2
mf
mfplat
mfreadwrite
ncrypt
ndfapi
propsys
shcore
uiautomationcore
uxtheme
wevtapi
winusb
zipfldr
) do (
Dependencies -json C:\work\win-polyfill\04-Windows-Server2003-64-RTM\Dll\%%x.dll -exports > gens\x64\%%x-nt-5.2.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.0-Dll\x64\%%x.dll -exports > gens\x64\%%x-nt-6.0.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.1-Dll\x64\%%x.dll -exports > gens\x64\%%x-nt-6.1.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.2-Dll\x64\%%x.dll -exports > gens\x64\%%x-nt-6.2.json
Dependencies -json C:\Windows\System32\%%x.dll -exports > gens\x64\%%x-nt-10.0.19045.json

Dependencies -json C:\work\win-polyfill\00-Windows2000-SP4-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-5.0.json
Dependencies -json C:\work\win-polyfill\01-WindowsXp-RTM-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-5.1.json
Dependencies -json C:\work\win-polyfill\03-Windows-Server2003-RTM-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-5.2.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.0-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-6.0.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.1-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-6.1.json
Dependencies -json C:\work\win-polyfill\08-Windows-NT-6.2-Dll\x86\%%x.dll -exports > gens\x86\%%x-nt-6.2.json
Dependencies -json C:\Windows\SysWow64\%%x.dll -exports > gens\x86\%%x-nt-10.0.19045.json
)

pause
