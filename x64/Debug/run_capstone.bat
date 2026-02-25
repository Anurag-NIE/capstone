@echo off
echo ============================================
echo Starting Applied Materials Capstone Project
echo ============================================
echo.

cd /d "%~dp0"

echo Current directory: %CD%
echo.

if not exist "HardwareIO.dll" (
    echo ERROR: HardwareIO.dll not found!
    pause
    exit /b 1
)

echo Starting LLVacController (Server)...
start "LLVacController" cmd /k "LLVacController.exe"

echo Waiting for server to initialize...
timeout /t 2 /nobreak > nul

echo.
echo Starting ATMController (Client) with 10 wafers...
start "ATMController" cmd /k "ATMController.exe 10"

echo.
echo Both processes started!
echo Check the console windows for output.
echo Logs will be saved to C:\Capstone\logs\
echo.
echo Press any key to exit this window...
pause > nul
