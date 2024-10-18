@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%graphics_test%" == "1" graphics_test.exe
    if "%physics_app%" == "1" physics_app.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"
