@echo off
rem SUBDESTRUCKT S0.1 - build Windows (MSVC). Usage : build_msvc.bat  (depuis spikes\s01)
setlocal
if not defined VCINSTALLDIR call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if not exist build mkdir build
set FLAGS=/nologo /std:c++17 /O2 /fp:precise /EHsc /W4 /wd4100 /D_CRT_SECURE_NO_WARNINGS /Iinclude /I..\s0\include
cl %FLAGS% tests\s01_tests.cpp /Fobuild\ /Febuild\s01_tests.exe || exit /b 1
cl %FLAGS% src\s01_cli.cpp /Fobuild\ /Febuild\s01_cli.exe || exit /b 1
cl %FLAGS% bench\s01_bench.cpp /Fobuild\ /Febuild\s01_bench.exe || exit /b 1
endlocal
