@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Prepare Build Directory ---
if not exist build mkdir build
@echo Copy data.
xcopy /y /q /s /e /i data .\build\data

rem --- Build Settings ---
set compiler=clang
set clang_turnoff_warnings=-Wno-deprecated-declarations -Wno-gnu-anonymous-struct -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments
rem set clang_flags=-Wall -Wconversion %clang_turnoff_warnings% -pedantic -g -I..\code\ -L..\code\
set clang_flags=-Wall %clang_turnoff_warnings% -fuse-ld=lld -pedantic -fms-runtime-lib=dll_dbg -g -I..\code\ -L..\code\

rem --- Build ---
@echo.
@echo **************************************************
@echo Compilation started.
@echo **************************************************
@echo.

pushd build
    if "%graphics_test%"         == "1" %compiler% %clang_flags% ..\code\app\graphics_test.cpp -o graphics_test.exe
    if "%sphere%"         == "1" %compiler% %clang_flags% ..\code\app\sphere.cpp -o sphere.exe
popd

@echo.
@echo **************************************************
@echo Compilation ended.
@echo **************************************************
@echo.

rem --- Unset Flags for Arguments ---
for %%a in (%*) do set "%%a=0"
