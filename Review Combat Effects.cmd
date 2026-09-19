@echo off
cd /d "%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0Tools\Review-CombatFX.ps1" -Visible
if errorlevel 1 pause
