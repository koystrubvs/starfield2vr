@echo off
setlocal
set "GAME_DIR=C:\Zona Downloads\Starfield.Digital.Premium.Edition-InsaneRamZes"
cd /d "%GAME_DIR%"

:: Disable VR mod
if exist dxgi.dll (
    ren dxgi.dll dxgi.dll.vr
    if errorlevel 1 (
        echo Failed to rename dxgi.dll
        pause
        exit /b 1
    )
) else if exist dxgi.dll.vr (
    echo VR mod already disabled
) else (
    echo dxgi.dll not found - nothing to disable
)

:: Launch game
start "" "Starfield.exe"

:: Wait for game to START (up to 30 seconds)
echo Waiting for Starfield.exe to start...
set /a count=0
:wait_start
tasklist /fi "imagename eq Starfield.exe" 2>nul | find /i "Starfield.exe" >nul
if %errorlevel%==0 goto game_running
set /a count+=1
if %count% geq 30 (
    echo Game did not start in 30 seconds, restoring VR mod
    goto restore
)
timeout /t 1 /nobreak >nul
goto wait_start

:game_running
echo Starfield is running. Waiting for it to close...
:wait_close
timeout /t 3 /nobreak >nul
tasklist /fi "imagename eq Starfield.exe" 2>nul | find /i "Starfield.exe" >nul
if %errorlevel%==0 goto wait_close

:restore
:: Small delay to ensure file locks are released
timeout /t 2 /nobreak >nul

if exist dxgi.dll.vr (
    ren dxgi.dll.vr dxgi.dll
    if errorlevel 1 (
        echo WARNING: Failed to restore dxgi.dll! Run as admin or restore manually.
        pause
        exit /b 1
    )
    echo VR mod restored.
) else (
    echo WARNING: dxgi.dll.vr not found - manual check needed
    pause
)
endlocal
