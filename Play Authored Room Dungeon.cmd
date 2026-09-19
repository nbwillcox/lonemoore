@echo off
cd /d "%~dp0"
echo LONEMOORE - AUTHORED ROOM DUNGEON
echo.
echo 1. Continue room-kit playtest
echo 2. Start a new random dungeon
echo 3. Start the reviewed crypt floor
echo.
echo This playtest has separate saves from your main journey.
choice /c 123 /n /m "Choose: "
set "roomChoice=%errorlevel%"
set "roomArgs=-RoomKitPlaytest -RoomKitFloor=8"
if "%roomChoice%"=="2" set "roomArgs=%roomArgs% -RoomKitFresh"
if "%roomChoice%"=="3" set "roomArgs=%roomArgs% -RoomKitFresh -RoomKitSeed=136871"
if exist "Builds\RoomKitCandidate\Windows\DungeonCrawler.exe" (
 start "" "%~dp0Builds\RoomKitCandidate\Windows\DungeonCrawler.exe" %roomArgs% -windowed
) else (
 echo The authored-room package is not installed yet.
 pause
)
