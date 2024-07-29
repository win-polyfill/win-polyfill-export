set path=C:\CI-Tools\Auxillary\Dependencies_x64_Release;%PATH%

mkdir gens
mkdir gens\apisetschema

Dependencies -json -apisetsdll C:\work\win-polyfill\08-Windows-NT-6.1-Dll\x64\apisetschema.dll > gens\apisetschema\apisetschema-nt-6.1.json

Dependencies -json -apisetsdll C:\work\win-polyfill\08-Windows-NT-6.2-Dll\x64\apisetschema.dll > gens\apisetschema\apisetschema-nt-6.2.json

Dependencies -json -apisetsdll C:\work\win-polyfill\11-Windows10-RTM-Dll\x64\apisetschema.dll > gens\apisetschema\apisetschema-nt-10.0.0.RTM.json

Dependencies -json -apisetsdll C:\Windows\System32\apisetschema.dll > gens\apisetschema\apisetschema-nt-10.0.19045.json

pause
