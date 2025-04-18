@echo off

REM orx setup
"%~dp0code\build\rebol\r3-windows.exe" -qs "%~dp0code\build\rebol\setup.r" %*

REM keep the window around when double-clicked
echo %CMDCMDLINE% | findstr /C:"/c">nul && pause
