param(
 [ValidateSet('Release')][string]$Label='Release',
 [ValidateRange(-1,17)][int]$Floor=-1,
 [ValidateSet('Entrance','BossApproach','Bridge')][string[]]$Scenes=@('Entrance','BossApproach','Bridge'),
 [ValidateRange(640,7680)][int]$Width=3440,
 [ValidateRange(480,4320)][int]$Height=1369
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$projectRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $projectRoot 'Builds\AdventurePolishCandidate\Windows'
$exe=Join-Path $candidate 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
$out=Join-Path $projectRoot "Saved\AdventurePolish\Performance\$Label"
$saved=Join-Path $candidate 'DungeonCrawler\Saved'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'The isolated AdventurePolishCandidate executable is missing.'}
if(Get-Process -Name 'DungeonCrawler*','UnrealEditor*','Blender' -ErrorAction SilentlyContinue){throw 'Close other games, editors and Blender before isolated profiling.'}
$item=Get-Item -LiteralPath $candidate
if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw 'The candidate cannot be a filesystem link.'}
New-Item -ItemType Directory -Path $out -Force | Out-Null

function Installed-Settings {
 $result=@()
 foreach($relative in @('Builds\Windows','Builds\ExpansionPrototype\Windows','Builds\RoomKitCandidate\Windows')){
  $path=Join-Path (Join-Path $projectRoot $relative) 'DungeonCrawler\Saved\Config\Windows\GameUserSettings.ini'
  $exists=Test-Path -LiteralPath $path -PathType Leaf
  $hash=if($exists){(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash}else{$null}
  $result+=[pscustomobject]@{Path=$path;Exists=$exists;SHA256=$hash}
 }
 return $result
}
function Assert-SettingsUnchanged($Before){
 $after=@(Installed-Settings)
 for($i=0;$i -lt $Before.Count;$i++){
  if($Before[$i].Exists -ne $after[$i].Exists -or $Before[$i].SHA256 -ne $after[$i].SHA256){throw "Installed graphics settings changed during profiling: $($Before[$i].Path)"}
 }
}

$jobs=@()
if($Floor -ge 0){foreach($scene in $Scenes){$jobs+=[pscustomobject]@{Floor=$Floor;Scene=$scene}}}
else{
 foreach($scene in @('Entrance','BossApproach','Bridge')){$jobs+=[pscustomobject]@{Floor=6;Scene=$scene}}
 $jobs+=@(
  [pscustomobject]@{Floor=1;Scene='Entrance'},
  [pscustomobject]@{Floor=8;Scene='BossApproach'},
  [pscustomobject]@{Floor=12;Scene='Entrance'},
  [pscustomobject]@{Floor=16;Scene='Entrance'}
 )
}
$settingsBefore=@(Installed-Settings)
$runtimeHash=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
try {
 foreach($job in $jobs){
  $name=('Floor_{0:D2}_{1}' -f ($job.Floor+1),$job.Scene)
  $log=Join-Path $out ($name+'.log')
  # The native fixture has Testing=true and an isolated prefix. It neither loads
  # player saves nor changes installed settings; these overrides are process-only.
  $arguments=@('-RoomKitBenchmark',"-ReviewFirst=$($job.Floor)","-ReviewLast=$($job.Floor)","-RoomKitScene=$($job.Scene)",'-RenderOffscreen','-windowed',"-ResX=$Width","-ResY=$Height",'-ForceRes','-unattended','-nosplash','-nosound','-csvGpuStats','-ExitAfterCsvProfiling','-csvExecCmds="2350:Shot SHOWUI"','-ExecCmds="t.MaxFPS 0,r.VSync 0,r.TSR.History.ScreenPercentage,r.ScreenPercentage"',('-abslog="'+$log+'"'))
  $started=Get-Date
  $process=Start-Process -FilePath $exe -ArgumentList $arguments -WorkingDirectory $candidate -WindowStyle Hidden -PassThru
  $deadline=(Get-Date).AddMinutes(8)
  while(!$process.WaitForExit(10000)){
   if((Get-Date) -gt $deadline){Stop-Process -Id $process.Id;throw "Adventure benchmark timed out: $name"}
  }
  $process.Refresh()
  $text=Get-Content -LiteralPath $log -Raw
  $ready='ROOM_KIT_BENCHMARK_READY floor='+$job.Floor+' scene='+$job.Scene+' rooms=68 frames=2400'
  $binaryDirectory=(Split-Path -Parent $exe).Replace('\','/').TrimEnd('/')+'/'
  if($process.ExitCode -ne 0 -or $text -notmatch [regex]::Escape($ready) -or $text -notmatch 'CSV finalize time' -or !$text.Replace('\','/').Contains($binaryDirectory) -or $text -match 'Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_BENCHMARK_FAILED'){
   throw "Adventure benchmark did not finish cleanly: $name (exit $($process.ExitCode))"
  }
  $csv=Get-ChildItem -LiteralPath (Join-Path $saved 'Profiling\CSV') -Filter '*.csv' -ErrorAction SilentlyContinue | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  $shot=Get-ChildItem -LiteralPath (Join-Path $saved 'Screenshots') -Recurse -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object LastWriteTime -ge $started | Sort-Object LastWriteTime -Descending | Select-Object -First 1
  if(!$csv -or !$shot){throw "Fresh native CSV or screenshot missing: $name"}
  if((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $runtimeHash){throw 'The candidate executable changed during profiling.'}
  Assert-SettingsUnchanged $settingsBefore
  $csvPath=Join-Path $out ($name+'.csv');$pngPath=Join-Path $out ($name+'.png')
  Copy-Item -LiteralPath $csv.FullName -Destination $csvPath -Force
  Copy-Item -LiteralPath $shot.FullName -Destination $pngPath -Force
  $metadata=[ordered]@{
   status='PASS';label=$Label;scene=$name;floorIndex=$job.Floor;nativeScene=$job.Scene;authoredSections=68
   candidateExecutable=$exe;candidateExecutableSha256=$runtimeHash;outputResolution=@($Width,$Height)
   processExitCode=$process.ExitCode;startedUtc=$started.ToUniversalTime().ToString('o');completedUtc=(Get-Date).ToUniversalTime().ToString('o')
   installedSettingsUnchanged=$true;installedSettingsBefore=$settingsBefore
   sources=@{csv=@{path=$csvPath;sha256=(Get-FileHash -LiteralPath $csvPath -Algorithm SHA256).Hash};screenshot=@{path=$pngPath;sha256=(Get-FileHash -LiteralPath $pngPath -Algorithm SHA256).Hash};log=@{path=$log;sha256=(Get-FileHash -LiteralPath $log -Algorithm SHA256).Hash}}
  }
  $metadata | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out ($name+'.capture.json')) -Encoding utf8
  Write-Output "PASS $Label ${name}: native warmed CSV and screenshot captured; installed settings unchanged."
 }
} finally {
 Assert-SettingsUnchanged $settingsBefore
}
Write-Output "ADVENTURE_PROFILE_COMPLETE scenes=$($jobs.Count); run analyze_adventure_polish_performance.py after all seven scenes are present."
