@echo off
cd /d "%~dp0"
echo LONEMOORE - EXPANDED CISTERN PROTOTYPE
echo.
echo 1. Continue prototype autosave
echo 2. Start a new random layout
echo 3. Start the reviewed layout
echo.
echo Separate prototype save slots. Your main campaign is preserved.
echo New layouts replace only the prototype autosave.
choice /c 123 /n /m "Choose: "
set "expansionChoice=%errorlevel%"
set "expansionArgs=-ExpansionPlaytest"
if "%expansionChoice%"=="2" set "expansionArgs=-ExpansionPlaytest -ExpansionFresh"
if "%expansionChoice%"=="3" set "expansionArgs=-ExpansionPlaytest -ExpansionFresh -ExpansionSeed=73519"
if exist "Builds\ExpansionPrototype\Windows\DungeonCrawler.exe" (
 start "" "%~dp0Builds\ExpansionPrototype\Windows\DungeonCrawler.exe" %expansionArgs% -windowed
) else (
 start "" "D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DungeonCrawler.uproject" /Game/Game/Maps/Boot -game %expansionArgs% -windowed
)
