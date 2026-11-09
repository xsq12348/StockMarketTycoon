@echo off
rem 编译独立的行情推演后端（训练用），产物 sim.dll
setlocal
set OUT=%~dp0..\_archive
if not exist "%OUT%" mkdir "%OUT%"
cl /nologo /O2 /LD "%~dp0..\src\sim_core.c" /Fe"%OUT%\sim.dll" /Fo"%OUT%\"
if errorlevel 1 goto :fail
echo SIM OK: %OUT%\sim.dll
goto :eof
:fail
echo SIM BUILD FAILED
