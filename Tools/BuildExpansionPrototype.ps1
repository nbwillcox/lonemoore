param([switch]$Package)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$engineRoot='D:\Epic Games\UE_5.8'
$projectFile=Join-Path $projectRoot 'DungeonCrawler.uproject'
$evidence=Join-Path $projectRoot 'Saved\ExpansionPrototype'
New-Item -ItemType Directory -Force -Path $evidence | Out-Null
& "$engineRoot\Engine\Build\BatchFiles\Build.bat" DungeonCrawlerEditor Win64 Development "-Project=$projectFile" -NoUBTMakefiles -NoLiveCoding -NoHotReloadFromIDE -UBANoDetour -nocache -NoPCH -MaxParallelActions=3
if($LASTEXITCODE -ne 0){throw 'Editor build failed'}
& "$engineRoot\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" $projectFile /Game/Game/Maps/Boot -unattended -nosplash -nosound -NullRHI '-ExecCmds=Automation RunTests Dungeon' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$evidence\Automation" "-abslog=$evidence\tests.log"
$results=Get-Content -LiteralPath "$evidence\tests.log" -Raw
if($LASTEXITCODE -ne 0 -or $results -match 'Result=\{Fail' -or $results -notmatch 'Automation Test Queue Empty \d+ tests performed'){throw 'Regression tests failed'}
if($Package){
 & "$engineRoot\Engine\Build\BatchFiles\Build.bat" DungeonCrawler Win64 Development "-Project=$projectFile" -NoUBTMakefiles -NoHotReloadFromIDE -UBANoDetour -nocache -NoPCH -MaxParallelActions=3
 if($LASTEXITCODE -ne 0){throw 'Game build failed'}
 & "$engineRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun "-project=$projectFile" -noP4 -platform=Win64 -clientconfig=Development -skipbuild -nocompileeditor -cook -stage -pak -archive "-archivedirectory=$projectRoot\Builds\ExpansionPrototype" -unattended -utf8output
 if($LASTEXITCODE -ne 0){throw 'Prototype packaging failed'}
}
