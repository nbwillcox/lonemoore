@echo off
cd /d "%~dp0"
echo LONEMOORE - REGIONAL ART TEST
echo.
echo 1. Last Dawn Cathedral
echo 2. Old City Sewers
echo 3. Forgotten Catacombs
echo 4. Goblin Warrens
echo 5. Ancient Crypts
echo 6. Buried Fortress
echo 7. The Deep
echo 8. Infernal Ruins
echo 9. Hell
echo.
echo Campaign saving is disabled in this test session.
choice /c 123456789 /n /m "Choose a region: "
set "artChoice=%errorlevel%"
set "artFloor=0"
if "%artChoice%"=="2" set "artFloor=1"
if "%artChoice%"=="3" set "artFloor=3"
if "%artChoice%"=="4" set "artFloor=5"
if "%artChoice%"=="5" set "artFloor=7"
if "%artChoice%"=="6" set "artFloor=9"
if "%artChoice%"=="7" set "artFloor=11"
if "%artChoice%"=="8" set "artFloor=13"
if "%artChoice%"=="9" set "artFloor=15"
if exist "Builds\Windows\DungeonCrawler.exe" (
 start "" "%~dp0Builds\Windows\DungeonCrawler.exe" -RegionalArtPlaytest -ArtFloor=%artFloor% -windowed
) else (
 start "" "D:\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DungeonCrawler.uproject" /Game/Game/Maps/Boot -game -RegionalArtPlaytest -ArtFloor=%artFloor% -windowed
)
