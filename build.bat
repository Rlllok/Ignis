@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Compile Shaders ---
pushd data\shaders
    CALL compileGLSL.bat
popd

rem --- Prepare Build Directory ---
if not exist build mkdir build
@echo Copy data.
xcopy /y /q /s /e /i data .\build\data

rem --- Build Settings ---
set compiler=clang
set clang_turnoff_warnings=-Wno-deprecated-declarations -Wno-gnu-anonymous-struct
set clang_flags=-Wall %clang_turnoff_warnings% -pedantic -g -I..\code\ -L..\code\

rem --- Build ---
@echo.
@echo **************************************************
@echo Compilation started.
@echo **************************************************
@echo.

pushd build
    if "%testApp%" == "1" %compiler% %clang_flags% ..\code\app\testApp.cpp -o testApp.exe
    if "%memoryTest%" == "1" %compiler% %clang_flags% ..\code\app\memoryTest.cpp -o memoryTest.exe
popd

@echo.
@echo **************************************************
@echo Compilation ended.
@echo **************************************************
@echo.

rem --- Unset Flags for Arguments ---
for %%a in (%*) do set "%%a=0"