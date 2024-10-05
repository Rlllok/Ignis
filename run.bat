@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%graphics_test%" == "1" graphics_test.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"
