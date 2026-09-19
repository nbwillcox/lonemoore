param([Parameter(Mandatory=$true)][string]$BuildDirectory,[Parameter(Mandatory=$true)][string]$Label)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$buildRoot=(Resolve-Path -LiteralPath $BuildDirectory).Path
# Benchmarks only run in the explicitly isolated packages, never player save roots.
$allowed=@((Join-Path $projectRoot 'Builds\FineTuneCandidate\Windows'),(Join-Path $projectRoot 'Builds\PerformanceCandidate\Windows'))
if($buildRoot -notin $allowed){throw 'Benchmark requires an isolated baseline or candidate package.'}
$out=Join-Path $projectRoot "Saved\FineTunePass\$Label"
New-Item -ItemType Directory -Path $out -Force | Out-Null
$saved=Join-Path $buildRoot 'DungeonCrawler\Saved'
New-Item -ItemType Directory -Path "$saved\SaveGames" -Force | Out-Null
foreach($name in @('Warrens','Explored')){
 Copy-Item -LiteralPath (Join-Path $projectRoot "Saved\FineTunePass\Fixtures\$name.sav") -Destination "$saved\SaveGames\ExpansionPrototype_Auto.sav" -Force
 $log=Join-Path $out "$name.log"
 $args=@('-ExpansionPlaytest','-RenderOffscreen','-windowed','-ResX=3440','-ResY=1369','-ForceRes','-unattended','-nosplash','-nosound','-csvGpuStats','-csvCaptureFrames=2400','-ExitAfterCsvProfiling','-csvExecCmds="2350:Shot SHOWUI"','-ExecCmds="t.MaxFPS 0,r.VSync 0,r.TSR.History.ScreenPercentage,r.ScreenPercentage"',('-abslog="'+$log+'"'))
 $started=Get-Date
 $p=Start-Process -FilePath "$buildRoot\DungeonCrawler\Binaries\Win64\DungeonCrawler.exe" -ArgumentList $args -WorkingDirectory $buildRoot -WindowStyle Hidden -PassThru -Wait
 $csv=Get-ChildItem -LiteralPath "$saved\Profiling\CSV" -Filter '*.csv' | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if(!$csv){throw "No CSV capture for $name"}
 $text=Get-Content -LiteralPath $log -Raw
 if($text -match 'Fatal error:|Assertion failed:|GPU Crashed' -or $text -notmatch 'CSV finalize time'){throw "Benchmark failed: $log"}
 Copy-Item -LiteralPath $csv.FullName -Destination (Join-Path $out "$name.csv") -Force
 $shot=Get-ChildItem -LiteralPath "$saved\Screenshots" -Recurse -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
 if(!$shot){throw "No fresh screenshot for $name"}
 Copy-Item -LiteralPath $shot.FullName -Destination (Join-Path $out "$name.png") -Force
 Write-Output "$Label $name capture complete (exit $($p.ExitCode))."
}
