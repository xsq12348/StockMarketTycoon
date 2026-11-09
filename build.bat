@echo off
setlocal
rem 构建脚本：源码在 src\，编译产物在 build\，可直接分发的内容复制到 dist\
rem 引擎随项目附带（engine\），如需换回外部引擎，把下面这行改成对应路径
set ENGINE=%~dp0engine
set INC=%ENGINE%\include
set SRC=%~dp0src
set OUT=%~dp0build
set DIST=%~dp0dist
if not exist "%OUT%" mkdir "%OUT%"
if not exist "%DIST%" mkdir "%DIST%"

cl /nologo /W0 /O2 /MD /I"%INC%" /I"%INC%\GLAD" /I"%INC%\KHR" ^
   /Fe"%OUT%\StockMarketTycoon.exe" /Fo"%OUT%\\" ^
   "%SRC%\_Main.c" "%SRC%\GameState.c" "%SRC%\Resource.c" "%SRC%\Setting.c" "%SRC%\Logic.c" "%SRC%\Draw.c" ^
   "%INC%\Moon_Engine_Function.c" "%INC%\Moon_Drawing_Function.c" "%INC%\Moon_Utility_Function.c" ^
   /link "%ENGINE%\lib\glad.lib" "%ENGINE%\lib\SDL3.lib" "%ENGINE%\lib\glfw3.lib" ^
   user32.lib gdi32.lib shell32.lib winmm.lib opengl32.lib
if errorlevel 1 goto :fail

copy /y "%ENGINE%\lib\SDL3.dll" "%OUT%\" >nul
copy /y "%ENGINE%\lib\glfw3.dll" "%OUT%\" >nul
copy /y "%OUT%\StockMarketTycoon.exe" "%DIST%\" >nul
copy /y "%OUT%\SDL3.dll" "%DIST%\" >nul
copy /y "%OUT%\glfw3.dll" "%DIST%\" >nul
echo BUILD OK: %OUT%\StockMarketTycoon.exe
echo DIST   : %DIST%
goto :eof
:fail
echo BUILD FAILED
