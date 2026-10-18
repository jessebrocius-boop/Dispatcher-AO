@echo off
set "VSVARS="
for /f "delims=" %%i in ('
    where /r "C:\Program Files\Microsoft Visual Studio\2022" vcvarsall.bat 2^>nul
') do set "VSVARS=%%i"

if not defined VSVARS (
    for /f "delims=" %%i in ('
        where /r "C:\Program Files (x86)\Microsoft Visual Studio\2022" vcvarsall.bat 2^>nul
    ') do set "VSVARS=%%i"
)

if not defined VSVARS (
    echo [ERROR] vcvarsall.bat not found – please install VS2022 Build Tools.
    exit /b 1
)

echo [ENV] Initializing MSVC environment via: %VSVARS%
call "%VSVARS%" x64

echo [BUILD] Initializing Dispatcher AO MSVC build sequence...
cl /O2 /std:c++17 /openmp:experimental /I include src\scheduler_kernel.cpp tests\test_modulation.cpp /Fe:dispatcher_node.exe
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    exit /b %errorlevel%
)

echo [SUCCESS] Build completed. Running executable...
.\dispatcher_node.exe
