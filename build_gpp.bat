@echo off
echo === Build with g++ / MinGW (no Visual Studio needed) ===
set SRC_DIR=%~dp0src
set OUT_DIR=%~dp0build
set EXE=%OUT_DIR%\obfuscator.exe
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] g++ not found.
    echo     Install one of:
    echo     1) MSYS2: https://www.msys2.org/ -> pacman -S mingw-w64-x86_64-gcc
    echo     2) MinGW-w64: winget install -e --id BrechtSanders.WinLibs.POSIX.UCRT
    echo     3) Build Tools: https://visualstudio.microsoft.com/visual-cpp-build-tools/
    echo     After install, restart terminal and run again.
    exit /b 1
)

echo Found g++:
g++ --version | findstr /R "g++"

echo Compiling...
g++ -std=c++17 -O2 -o "%EXE%" "%SRC_DIR%\main.cpp" "%SRC_DIR%\obfuscator.cpp" "%SRC_DIR%\language.cpp" "%SRC_DIR%\lexer.cpp" "%SRC_DIR%\encodings.cpp" -lstdc++fs 2>&1

if exist "%EXE%" (
    echo [OK] Built: %EXE%
    "%EXE%" --help
) else (
    echo [ERR] Build failed - trying without -lstdc++fs
    g++ -std=c++17 -O2 -o "%EXE%" "%SRC_DIR%\main.cpp" "%SRC_DIR%\obfuscator.cpp" "%SRC_DIR%\language.cpp" "%SRC_DIR%\lexer.cpp" "%SRC_DIR%\encodings.cpp"
    if exist "%EXE%" ( echo [OK] Built: %EXE% & "%EXE%" --help ) else ( echo [FAIL] )
)
