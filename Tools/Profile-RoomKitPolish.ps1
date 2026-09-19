param(
 [string]$BuildDirectory,
 [ValidatePattern('^[A-Za-z0-9_-]+$')][string]$Label='Candidate',
 [ValidateRange(0,17)][int]$Floor=6,
 [ValidateSet('Entrance','BossApproach','Bridge')][string[]]$Scenes=@('Entrance','BossApproach','Bridge'),
 [ValidateRange(640,7680)][int]$Width=3440,
 [ValidateRange(480,4320)][int]$Height=1369
)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$candidate=Join-Path $projectRoot 'Builds\RoomKitPolishCandidate\Windows'
if(!$BuildDirectory){$BuildDirectory=$candidate}
$buildRoot=(Resolve-Path -LiteralPath $BuildDirectory).Path
if($buildRoot -ne $candidate){throw 'Room kit benchmark requires the isolated RoomKitPolishCandidate package.'}
$exe=Join-Path $buildRoot 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw "Missing candidate executable: $exe"}
$out=Join-Path $projectRoot "Saved\RoomKitPolish\Performance\$Label"
New-Item -ItemType Directory -Path $out -Force | Out-Null
$saved=Join-Path $buildRoot 'DungeonCrawler\Saved'
# The native fixture creates a deterministic new game with Testing=true and an isolated prefix.
# It never loads, copies or writes a player save. Display overrides apply only to this process.
foreach($scene in $Scenes){
 $name=('Floor_{0:D2}_{1}' -f ($Floor+1),$scene)
 $log=Join-Path $out "$name.log"
 $arguments=@('-RoomKitBenchmark',"-ReviewFirst=$Floor","-ReviewLast=$Floor","-RoomKitScene=$scene",'-RenderOffscreen','-windowed',"-ResX=$Width","-ResY=$Height",'-ForceRes','-unattended','-nosplash','-nosound','-csvGpuStats','-ExitAfterCsvProfiling','-csvExecCmds="2350:Shot SHOWUI"','-ExecCmds="t.MaxFPS 0,r.VSync 0,r.TSR.History.ScreenPercentage,r.ScreenPercentage"',('-abslog="'+$log+'"'))
 $started=Get-Date
 $process=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $buildRoot -WindowStyle Hidden -PassThru
 # Wait in bounded intervals and enforce a fixture timeout instead of leaving a hung game behind.
 $deadline=(Get-Date).AddMinutes(8)
 while(!$process.WaitForExit(10000)){
  if((Get-Date) -gt $deadline){Stop-Process -Id $process.Id;throw "Room kit benchmark timed out: $log"}
 }
 $capture=Get-ChildItem -LiteralPath (Join-Path $saved 'Profiling\CSV') -Filter '*.csv' -ErrorAction SilentlyContinue | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if(!$capture){throw "No fresh CSV capture: $log"}
 $text=Get-Content -LiteralPath $log -Raw
 if($text -match 'Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_BENCHMARK_FAILED' -or $text -notmatch 'CSV finalize time' -or $text -notmatch 'ROOM_KIT_BENCHMARK_READY'){throw "Benchmark failed: $log"}
 Copy-Item -LiteralPath $capture.FullName -Destination (Join-Path $out "$name.csv") -Force
 $shot=Get-ChildItem -LiteralPath (Join-Path $saved 'Screenshots') -Recurse -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if(!$shot){throw "No fresh scene screenshot: $log"}
 Copy-Item -LiteralPath $shot.FullName -Destination (Join-Path $out "$name.png") -Force
 Write-Output "$Label $name captured after native scene warm-up (process exit $($process.ExitCode))."
}
