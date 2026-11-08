@echo off
REM ===== Local Windows build -> dist\GanjLibrarySetup.exe =====
REM Requirements: Visual Studio 2022 (Desktop C++), CMake, Qt 6.5+ (MSVC 64-bit), Inno Setup 6
REM Change QT_DIR if Qt is installed somewhere else.
setlocal
set QT_DIR=C:\Qt\6.5.3\msvc2019_64

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH=%QT_DIR% || goto :fail
cmake --build build --config Release || goto :fail

if exist deploy rmdir /s /q deploy
mkdir deploy
copy build\Release\Ganj.exe deploy\ >nul
"%QT_DIR%\bin\windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw deploy\Ganj.exe || goto :fail
if exist resources\fonts xcopy resources\fonts deploy\fonts /E /I /Y >nul
curl -L -o deploy\vc_redist.x64.exe https://aka.ms/vs/17/release/vc_redist.x64.exe

"%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" installer\ganj.iss || goto :fail
echo.
echo Done: dist\GanjLibrarySetup.exe
exit /b 0

:fail
echo.
echo BUILD FAILED
exit /b 1
