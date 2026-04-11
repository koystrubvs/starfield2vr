@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

set CMAKE="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
set NINJA="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

cd /d "C:\Zona Downloads\starfield2vr"

if not exist "build\build-release-msvc\build.ninja" (
    echo === Configuring CMake ===
    %CMAKE% --preset build-release-msvc -DCMAKE_MAKE_PROGRAM=%NINJA%
    if errorlevel 1 (
        echo CMake configure FAILED
        pause
        exit /b 1
    )
) else (
    echo === Skipping configure (already configured) ===
)

echo === Building ===
%CMAKE% --build build/build-release-msvc --config RelWithDebInfo -j%NUMBER_OF_PROCESSORS%
if errorlevel 1 (
    echo Build FAILED
    pause
    exit /b 1
)

echo === BUILD SUCCESS ===
echo Output: build\build-release-msvc\dxgi.dll
pause
