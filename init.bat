@echo off

REM orx init
"%~dp0code\build\rebol\r3-windows.exe" -qs %~dp0code\build\rebol\init.r %*
