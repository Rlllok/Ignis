@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%testApp%"=="1" testApp.exe
    if "%memoryTest%"=="1" memoryTest.exe
    if "%game%"=="1" game.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"