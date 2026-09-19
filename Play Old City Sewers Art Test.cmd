@echo off
cd /d "%~dp0"
if exist "Builds\Windows\DungeonCrawler.exe" (
 start "" "%~dp0Builds\Windows\DungeonCrawler.exe" -SewerArtPlaytest -windowed
) else (
 start "" "D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DungeonCrawler.uproject" /Game/Game/Maps/Boot -game -SewerArtPlaytest -windowed
)
