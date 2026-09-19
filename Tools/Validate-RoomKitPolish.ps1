param([switch]$SkipNativeReview,[switch]$SkipPerformance)
$ErrorActionPreference='Stop'
$taskRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $taskRoot 'Builds\RoomKitPolishCandidate\Windows'
$exe=Join-Path $candidate 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
$evidence=Join-Path $taskRoot 'Saved\RoomKitPolish'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Final candidate is missing.'}
if(Get-Process -Name 'DungeonCrawler*','UnrealEditor*','Blender' -ErrorAction SilentlyContinue){throw 'Close other game/editor/Blender processes before isolated validation.'}
# Copy preferences only into the isolated candidate; never write installed settings.
$settings=Join-Path $taskRoot 'Builds\Windows\DungeonCrawler\Saved\Config\Windows\GameUserSettings.ini'
$candidateConfig=Join-Path $candidate 'DungeonCrawler\Saved\Config\Windows'
New-Item -ItemType Directory -Path $candidateConfig -Force | Out-Null
Copy-Item -LiteralPath $settings -Destination (Join-Path $candidateConfig 'GameUserSettings.ini') -Force
function Invoke-NativeReview([string]$Name,[string[]]$Extra,[string]$Marker){
 $log=Join-Path $evidence ($Name+'.log')
 $arguments=@('-RoomKitReview','-RenderOffscreen','-windowed','-ResX=3440','-ResY=1369','-ForceRes','-unattended','-nosplash','-nosound',('-abslog="'+$log+'"'))+$Extra
 $job=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $candidate -WindowStyle Hidden -PassThru
 $deadline=(Get-Date).AddMinutes(20)
 while(!$job.WaitForExit(10000)){if((Get-Date) -gt $deadline){Stop-Process -Id $job.Id;throw "Native review timed out: $Name"}}
 $content=Get-Content -LiteralPath $log -Raw
 if($job.ExitCode -ne 0 -or $content -notmatch $Marker -or $content -match 'ROOM_KIT_QA FAIL|Fatal error:|Assertion failed:|GPU Crashed|ROOM_DRESSING_MISSING'){throw "Native validation failed: $Name exit=$($job.ExitCode)"}
 Write-Output "PASS $Name"
}
if(!$SkipNativeReview){
 foreach($room in @('circular_ossuary','rounded_turn','cavern_hall')){
  Invoke-NativeReview ('packaged_floor_'+$room) @('-RoomKitFloorDiagnostic',('-DiagnosticRoom='+$room),'-ReviewFirst=0','-ReviewLast=0') 'ROOM_KIT_FLOOR_DIAGNOSTIC_COMPLETE floor=0 rooms=1 views=4 failures=0'
 }
 Invoke-NativeReview 'packaged_review' @('-ReviewFirst=0','-ReviewLast=17') 'ROOM_KIT_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0'
 Invoke-NativeReview 'packaged_dressing_review' @('-RoomKitDressingReview','-ReviewFirst=0','-ReviewLast=17') 'ROOM_KIT_DRESSING_REVIEW_COMPLETE floors=18 failures=0'
}
if(!$SkipPerformance){
 & (Join-Path $PSScriptRoot 'Profile-RoomKitPolish.ps1') -Label Release -Floor 6
 & (Join-Path $PSScriptRoot 'Profile-RoomKitPolish.ps1') -Label Release -Floor 1 -Scenes Entrance
 & (Join-Path $PSScriptRoot 'Profile-RoomKitPolish.ps1') -Label Release -Floor 8 -Scenes BossApproach
 # Cover the newly added skull geometry and emissive lava in addition to the
 # five scenes retained from the previous release's comparison.
 & (Join-Path $PSScriptRoot 'Profile-RoomKitPolish.ps1') -Label Release -Floor 12 -Scenes Entrance
 & (Join-Path $PSScriptRoot 'Profile-RoomKitPolish.ps1') -Label Release -Floor 16 -Scenes Entrance
 & 'C:\Python313\python.exe' (Join-Path $PSScriptRoot 'analyze_room_kit_polish_performance.py') --label Release
 if($LASTEXITCODE -ne 0){throw 'Performance analysis failed.'}
}
Write-Output 'Final packaged native validation and requested performance captures completed.'
