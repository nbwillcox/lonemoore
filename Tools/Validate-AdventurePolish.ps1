param([switch]$SkipNative,[switch]$SkipPerformance)
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $projectRoot 'Builds\AdventurePolishCandidate\Windows'
$exe=Join-Path $candidate 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
$evidence=Join-Path $projectRoot 'Saved\AdventurePolish'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged candidate is missing.'}
if(Get-Process -Name 'DungeonCrawler*','UnrealEditor*' -ErrorAction SilentlyContinue){throw 'Another game or editor is running.'}
$config=Join-Path $candidate 'DungeonCrawler\Saved\Config\Windows'
New-Item -ItemType Directory -Path $config -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $projectRoot 'Builds\Windows\DungeonCrawler\Saved\Config\Windows\GameUserSettings.ini') -Destination (Join-Path $config 'GameUserSettings.ini') -Force
function Review([string]$Name,[string[]]$Extra,[string]$Complete){
 $log=Join-Path $evidence ($Name+'.log')
 $arguments=@('-RenderOffscreen','-windowed','-ResX=3440','-ResY=1369','-ForceRes','-unattended','-nosplash','-nosound',('-abslog="'+$log+'"'))+$Extra
 $p=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $candidate -WindowStyle Hidden -PassThru
 $deadline=(Get-Date).AddMinutes(20)
 while(!$p.WaitForExit(10000)){if((Get-Date) -gt $deadline){Stop-Process -Id $p.Id;throw "Timed out: $Name"}}
 $content=Get-Content -LiteralPath $log -Raw
 if($p.ExitCode -ne 0 -or $content -notmatch $Complete -or $content -match 'ADVENTURE_QA FAIL|ROOM_KIT_QA FAIL|Fatal error:|Assertion failed:|GPU Crashed|ROOM_DRESSING_MISSING'){throw "Native review failed: $Name (exit $($p.ExitCode))"}
 Write-Output "PASS $Name"
}
if(!$SkipNative){
 Review 'packaged_adventure' @('-AdventurePolishReview') 'ADVENTURE_POLISH_REVIEW_COMPLETE failures=0'
 Review 'packaged_all_floors' @('-RoomKitReview','-ReviewFirst=0','-ReviewLast=17') 'ROOM_KIT_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0'
}
if(!$SkipPerformance){
 & (Join-Path $PSScriptRoot 'Profile-AdventurePolish.ps1')
 & 'C:\Python313\python.exe' (Join-Path $PSScriptRoot 'analyze_adventure_polish_performance.py')
 if($LASTEXITCODE -ne 0){throw 'Performance validation failed.'}
}
