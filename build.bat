@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Prepare Build Directory ---
if not exist build mkdir build
@echo Copy data.
xcopy /y /q /s /e /i data .\build\data

rem --- Build Settings ---
set compiler=clang -std=c11
set clang_turnoff_warnings=-Wno-deprecated-declarations -Wno-gnu-anonymous-struct -Wno-unused-function -Wno-gnu-zero-variadic-macro-arguments -Wno-missing-braces -Wno-strict-prototypes
rem set clang_flags=-Wall -Wconversion %clang_turnoff_warnings% -pedantic -g -I..\code\ -L..\code\
set include_flags=-I..\code\ -I..\code\third_party\include\
set link_flags=-L..\code\third_party\lib\ -lwinmm.lib -luser32.lib -lvulkan\vulkan-1.lib -lglslang\GenericCodeGen.lib -lglslang\glslang.lib -lglslang\glslang-default-resource-limits.lib -lglslang\MachineIndependent.lib -lglslang\OSDependent.lib -lglslang\SPIRV.lib -lglslang\SPIRV-Tools.lib -lglslang\SPIRV-Tools-opt.lib -lglslang\SPIRV-tools-opt.lib -lglslang\SPVRemapper.lib
set clang_flags=-DIGNIS_DEBUG -Wall %clang_turnoff_warnings% -fuse-ld=lld -pedantic -fms-runtime-lib=dll_dbg -g
set build_flags=-DIGNIS_PLATFORM_WIN32

rem --- Build ---
@echo.
@echo **************************************************
@echo Compilation started.
@echo **************************************************
@echo.

pushd build
    if "%main%" == "1" %compiler% %build_flags% %clang_flags% %include_flags% %link_flags% ..\code\app\main.c -o main.exe
    if "%config%" == "1" %compiler% %build_flags% %clang_flags% %include_flags% %link_flags% ..\code\app\config.c -o config.exe
popd

@echo.
@echo **************************************************
@echo Compilation ended.
@echo **************************************************
@echo.

rem --- Unset Flags for Arguments ---
for %%a in (%*) do set "%%a=0"
