@echo off
cd /d "%~dp0"

set "name=JobSystem"

cmake --build "%CD%\cmake-build-debug-visual-studio" --target %name% -j 22
if errorlevel 1 exit /b

cd /d "%CD%\cmake-build-debug-visual-studio"
%name%.exe
