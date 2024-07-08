@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%game%"     == "1" game.exe
    if "%physics%"  == "1" physics.exe
    if "%sphere%"   == "1" sphere.exe
    if "%soa_test%" == "1" soa_test.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"
