@echo off
cd /d "%~dp0"

set "name=SceneRenderer"

cmake --build "%CD%\cmake-build-debug" --target %name% -j 22
if errorlevel 1 exit /b

cd /d "%CD%\cmake-build-debug"
%name%.exe
