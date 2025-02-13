@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%renderer%" == "1" renderer.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"
