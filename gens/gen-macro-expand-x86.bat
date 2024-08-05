call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsamd64_x86.bat"
:: /C means preserve comment
:: /EP Preprocesses C and C++ source files and copies the preprocessed files to the standard output device.
pushd x86
cl /EP ../gen-exports-decls.c -I../../src/deps/phnt -I../../src/ -I../../src/deps -I. -DJET_VERSION=0x0B00 -DUNICODE -D_UNICODE -D_CONST_RETURN=const >gen-exports-decls.h
cl /EP ../gen-exports-decls-cpp.cpp -I../../src/deps/phnt -I../../src/ -I../../src/deps -DJET_VERSION=0x0B00 -DUNICODE -D_UNICODE -D_CONST_RETURN=const -I. >>gen-exports-decls.h

..\..\tools\win-polyfill-export-gen-sa.exe gen-exports-decls.h gen-exports-decls.h.index.txt
pause
