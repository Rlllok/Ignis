@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%testApp%"=="1" testApp.exe
    if "%memoryTest%"=="1" memoryTest.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"