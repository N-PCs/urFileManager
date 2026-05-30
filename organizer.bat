@echo off
setlocal
set EXE_PATH="%~dp0organizer.exe"

if not exist %EXE_PATH% (
    echo [ERROR] organizer.exe not found. Please compile the C++ project first by running build.bat.
    exit /b 1
)

%EXE_PATH% %*
endlocal
