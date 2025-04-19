@echo off

REM orx init
"%~dp0code\build\rebol\r3-windows.exe" -qs "%~dp0code\build\rebol\init.r" %*

REM keep the window around when double-clicked
echo %CMDCMDLINE% | findstr /C:"%~nx0">nul && pause
exit /B 0
