@echo off
echo ============================================
echo Starting Applied Materials Capstone Project
echo ============================================

REM Create necessary directories
if not exist "C:\Capstone\logs" mkdir "C:\Capstone\logs"
if not exist "C:\Capstone\recipes" mkdir "C:\Capstone\recipes"

REM Copy recipes to C:\Capstone\recipes if they don't exist
if not exist "C:\Capstone\recipes\recipe1.lcp" copy "recipes\recipe1.lcp" "C:\Capstone\recipes\"
if not exist "C:\Capstone\recipes\recipe2.lcp" copy "recipes\recipe2.lcp" "C:\Capstone\recipes\"

echo.
echo Starting LLVacController (Server)...
start "LLVacController" LLVacController.exe

echo Waiting for server to initialize...
timeout /t 2 /nobreak > nul

echo.
echo Starting ATMController (Client) with 10 wafers...
start "ATMController" ATMController.exe 10

echo.
echo Both processes started!
echo Check the console windows for output.
echo Logs will be saved to C:\Capstone\logs\
echo.
pause
