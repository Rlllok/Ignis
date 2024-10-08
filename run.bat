@echo off

rem --- Unpack Arguments ---
for %%a in (%*) do set "%%a=1"

rem --- Run App ---
pushd build
    if "%game%"         == "1" game.exe
    if "%physics%"      == "1" physics.exe
    if "%sphere%"       == "1" sphere.exe
    if "%soa_test%"     == "1" soa_test.exe
    if "%fullquad%"     == "1" fullquad.exe
    if "%ecs%"          == "1" ecs.exe
    if "%string_test%"  == "1" string_test.exe
    if "%memory%"       == "1" memory.exe
popd

rem --- Unset Arguments ---
for %%a in (%*) do set "%%a=0"
