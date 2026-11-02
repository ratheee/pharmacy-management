@echo off
rem Builds pharmacy.exe and starts it. Double-click this file in File Explorer.
rem Uses g++ from PATH, or else the WinLibs copy that winget installs in AppData.
cd /d "%~dp0"

where g++ >nul 2>nul
if errorlevel 1 (
    for /d %%D in ("%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.*") do (
        if exist "%%D\mingw64\bin\g++.exe" set "GPP_DIR=%%D\mingw64\bin"
    )
)
if defined GPP_DIR set "PATH=%GPP_DIR%;%PATH%"

where g++ >nul 2>nul
if errorlevel 1 (
    echo g++ was not found. Install it from PowerShell with:
    echo     winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
    pause
    exit /b 1
)

rem A running pharmacy.exe can't be replaced, so offer to close it first.
tasklist /fi "imagename eq pharmacy.exe" 2>nul | find /i "pharmacy.exe" >nul
if not errorlevel 1 (
    echo pharmacy.exe is already running in another window, so it can't be rebuilt.
    choice /c YN /m "Close the running copy now? Any unsaved input there will be lost"
    if errorlevel 2 (
        echo Close the other window first, then run this again.
        pause
        exit /b 1
    )
    taskkill /im pharmacy.exe /f >nul
    timeout /t 1 /nobreak >nul
)

echo Building pharmacy.exe ...
g++ -std=c++17 -Wall -Wextra -Iinclude src/Account.cpp src/Billing.cpp src/Inventory.cpp src/main.cpp src/Medicine.cpp src/Requests.cpp src/Utils.cpp -o pharmacy.exe
if errorlevel 1 (
    echo.
    echo Build failed. If pharmacy.exe is already open in another window, close it and try again.
    pause
    exit /b 1
)

cls
"%~dp0pharmacy.exe"
echo.
pause
