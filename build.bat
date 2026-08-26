@echo off
echo === Compilation Obfuscator Universel ===
set SRC_DIR=%~dp0src
set OUT_DIR=%~dp0build
set EXE=%OUT_DIR%\obfuscator.exe

if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo Utilisation de MSVC cl.exe...
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1

cl /EHsc /std:c++17 /O2 /Fe:"%EXE%" "%SRC_DIR%\main.cpp" "%SRC_DIR%\obfuscator.cpp" "%SRC_DIR%\language.cpp" "%SRC_DIR%\lexer.cpp" "%SRC_DIR%\encodings.cpp" >nul

if exist "%EXE%" (
    echo [OK] Compilation reussie: %EXE%
    "%EXE%" --help
) else (
    echo [ERREUR] Compilation echouee
    cl /EHsc /std:c++17 /Fe:"%EXE%" "%SRC_DIR%\main.cpp" "%SRC_DIR%\obfuscator.cpp" "%SRC_DIR%\language.cpp" "%SRC_DIR%\lexer.cpp" "%SRC_DIR%\encodings.cpp"
)
