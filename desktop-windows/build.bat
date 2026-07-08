@echo off
echo.
echo =======================================================
echo   urFileManager - Windows C++ (GUI + CLI) Builder
echo =======================================================
echo.

echo [1/4] Compiling resource script using windres...
windres -O coff ufmgr.rc -o ufmgr.o
if %errorlevel% neq 0 (
    echo [ERROR] Failed to compile resources. Ensure MinGW-w64 is installed.
    pause
    exit /b %errorlevel%
)

echo [2/4] Compiling shared core (config, PDF report, revert)...
g++ -std=c++17 -O3 -c urfm_common.cpp -o urfm_common.o
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %errorlevel%
)

echo [3/4] Building GUI launcher (ufmgr.exe)...
g++ -std=c++17 -O3 -mwindows -o ufmgr.exe gui_app.cpp urfm_common.o ufmgr.o -lshlwapi -lole32 -lcomctl32 -ldwmapi -luuid -lcomctl32
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %errorlevel%
)

echo [4/4] Building CLI (ufmgr-cli.exe)...
g++ -std=c++17 -O3 -mconsole -o ufmgr-cli.exe cli.cpp urfm_common.o ufmgr.o -lshlwapi -lole32 -lcomctl32 -ldwmapi -luuid -lcomctl32
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b %errorlevel%
)

echo Build successful! Created 'ufmgr.exe' (GUI) and 'ufmgr-cli.exe' (CLI).
echo.
echo GUI launcher:
echo   ufmgr.exe
echo CLI examples:
echo   ufmgr-cli.exe "C:\Users\You\Downloads" --no-dry-run
echo   ufmgr-cli.exe --revert "C:\Users\You\Downloads"
echo   ufmgr-cli.exe -h
echo See windows_usage.md for full details.
echo =======================================================
pause
