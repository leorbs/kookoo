@echo off
REM ==========================================================
REM Git Repository Auto-Update Script (with confirmation)
REM This script force-updates the local repo to match remote.
REM ==========================================================

echo.
echo ------------------------------------------
echo   Git Repository Auto-Update Utility
echo ------------------------------------------
echo.
echo This will:
echo   - Discard ALL local changes
echo   - Overwrite files with the latest from remote
echo   - Clean untracked files and folders
echo.
echo Are you sure you want to continue? (Y/N)

choice /c YN /n /m "> "
if errorlevel 2 (
    echo.
    echo Update cancelled.
    pause
    exit /b 0
)

echo.
echo Proceeding with repository update...
echo.

REM Navigate to the directory where this script resides
cd /d "%~dp0"

REM Make sure git is available
git --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Git is not installed or not in PATH.
    pause
    exit /b 1
)

echo [Step 1] Fetching latest changes...
git fetch --all

REM Detect current branch automatically
for /f "tokens=*" %%b in ('git rev-parse --abbrev-ref HEAD') do set CURRENT_BRANCH=%%b

echo [Step 2] Resetting local files to match origin/%CURRENT_BRANCH%...
git reset --hard origin/%CURRENT_BRANCH%

echo [Step 3] Cleaning untracked files and directories...
git clean -fd

echo.
echo [SUCCESS] Repository has been updated to the latest version of %CURRENT_BRANCH%.
echo.

pause
exit /b 0
