@echo off

REM orx setup
code\build\rebol\r3-windows.exe -qs code/build/rebol/setup.r %*

REM keep the window around when double-clicked
echo %CMDCMDLINE% | findstr /C:"/c">nul && pause
