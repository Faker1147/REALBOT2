@echo off
setlocal
title Counter-Strike 1.6 Server - REALBOT2

rem This script starts HLDS with the arguments it needs.
rem Starting hlds.exe by double-clicking it (without -game cstrike)
rem crashes this 8684 build immediately - that is the "flash and close" bug.

set "HLDIR=%~dp0"
if exist "%HLDIR%hlds.exe" goto found
set "HLDIR=%~dp0..\..\.."
if exist "%HLDIR%\hlds.exe" goto found

echo hlds.exe was not found.
echo Put this file in the Half-Life root folder (next to hlds.exe),
echo or keep it in cstrike\addons\realbot2\.
pause
exit /b 1

:found
cd /d "%HLDIR%"
set "MAP=%~1"
if "%MAP%"=="" set "MAP=de_dust2"

echo ============================================================
echo  Counter-Strike 1.6 dedicated server + REALBOT2
echo  Keep this window open while playing.
echo.
echo  Add bots in game with the numpad menu (KP_INS / numpad 0),
echo  or type in the server console:
echo      realbot2 add 1 5
echo      realbot2 add 2 5
echo ============================================================
echo.
echo Map: %MAP%
echo.
hlds.exe -game cstrike -console -insecure -norestart +sv_lan 1 +maxplayers 12 +map %MAP%
echo.
echo Server process has stopped. Press any key to close this window.
pause >nul
