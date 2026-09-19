param([string]$PythonExecutable='C:\Python313\python.exe')
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $projectRoot 'Builds\RoomKitCandidate\Windows'
$evidence=Join-Path $projectRoot 'Saved\RoomKitPass'
$targets=@((Join-Path $projectRoot 'Builds\Windows'),(Join-Path $projectRoot 'Builds\ExpansionPrototype\Windows'))
$runtimeDirectories=@('Engine','DungeonCrawler\Binaries','DungeonCrawler\Content')

function Assert-ContainedPath([string]$Path,[string]$Parent){
 $full=[IO.Path]::GetFullPath($Path)
 $parentFull=[IO.Path]::GetFullPath($Parent).TrimEnd([char[]]'\/')
 if($full -ne $parentFull -and !$full.StartsWith($parentFull+'\',[StringComparison]::OrdinalIgnoreCase)){throw "Path is outside its intended directory: $full"}
 return $full
}
function Assert-NoRunningGame {
 if(Get-Process -Name 'DungeonCrawler*' -ErrorAction SilentlyContinue){throw 'Close the running Dungeon Crawler game before replacing its runtime files.'}
}
function Read-RequiredJson([string]$Path){
 if(!(Test-Path -LiteralPath $Path -PathType Leaf)){throw "Required release evidence is missing: $Path"}
 return Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json
}
function Assert-NoLinks([string]$Path){
 if(!(Test-Path -LiteralPath $Path)){return}
 $item=Get-Item -LiteralPath $Path -Force
 if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Runtime root may not be a filesystem link: $Path"}
 foreach($entry in Get-ChildItem -LiteralPath $Path -Directory -Recurse -Force){
  if($entry.Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Runtime tree contains a filesystem link: $($entry.FullName)"}
 }
}
function Get-RuntimeFiles([string]$Base){
 $files=@(Get-ChildItem -LiteralPath $Base -File -Force)
 foreach($directory in $runtimeDirectories){
  $path=Assert-ContainedPath (Join-Path $Base $directory) $Base
  if(!(Test-Path -LiteralPath $path -PathType Container)){throw "Missing packaged runtime directory: $path"}
  $files+=Get-ChildItem -LiteralPath $path -File -Recurse -Force | Where-Object { $_.FullName.Substring($Base.Length) -notmatch '(?i)[\\/]Saved[\\/]' }
 }
 return $files | Sort-Object FullName
}
function Invoke-PreservationCheck([string]$Phase){
 & $PythonExecutable (Join-Path $PSScriptRoot 'verify_room_kit_preservation.py') | Out-Host
 if($LASTEXITCODE -ne 0){throw "Protected saves, settings or original artwork changed ($Phase). Runtime promotion stopped."}
 $result=Read-RequiredJson (Join-Path $evidence 'preservation.json')
 if($result.status -ne 'PASS' -or $result.protected -lt 1 -or $result.unchanged -ne $result.protected){throw "Incomplete preservation proof ($Phase)."}
 Copy-Item -LiteralPath (Join-Path $evidence 'preservation.json') -Destination (Join-Path $evidence "preservation_$Phase.json") -Force
 return $result
}

Assert-NoRunningGame
$candidate=(Resolve-Path -LiteralPath (Assert-ContainedPath $candidate (Join-Path $projectRoot 'Builds'))).Path
Assert-NoLinks $candidate
foreach($target in $targets){
 $checked=Assert-ContainedPath $target (Join-Path $projectRoot 'Builds')
 if($checked -eq $candidate){throw 'A release target may not be the candidate directory.'}
 if(!(Test-Path -LiteralPath $checked -PathType Container)){throw "Existing launcher directory missing: $checked"}
 Assert-NoLinks $checked
}
if(!(Test-Path -LiteralPath $PythonExecutable -PathType Leaf)){throw "Python interpreter not found: $PythonExecutable"}
if(!(Test-Path -LiteralPath (Join-Path $candidate 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe') -PathType Leaf)){throw 'Packaged candidate executable is missing.'}

$tests=Read-RequiredJson (Join-Path $evidence 'AutomationFinal\index.json')
$passed=[int]$tests.succeeded+[int]$tests.succeededWithWarnings
if($passed -ne 23 -or $tests.failed -ne 0 -or $tests.notRun -ne 0 -or $tests.inProcess -ne 0){throw 'All 23 final native automation suites must finish successfully before promotion.'}
$geometry=Read-RequiredJson (Join-Path $evidence 'geometry_import.json')
if(@($geometry.meshes).Count -ne 26 -or $geometry.roomCount -ne 25 -or $geometry.failures -ne 0){throw 'The 26-mesh import must contain 25 rooms and zero failures.'}
$materials=Read-RequiredJson (Join-Path $evidence 'materials.json')
if($materials.status -ne 'PASS' -or @($materials.materials).Count -ne 45){throw 'All 45 regional material imports must pass before promotion.'}
$walkways=Read-RequiredJson (Join-Path $evidence 'walkways.json')
if(!$walkways.passed -or $walkways.roomCount -ne 25 -or $walkways.failureCount -ne 0){throw 'All 25 authored room meshes must pass the walking-lane audit.'}

$nativeResult=Join-Path $candidate 'DungeonCrawler\Saved\RoomKit\runtime_results.txt'
if(!(Test-Path -LiteralPath $nativeResult -PathType Leaf)){throw 'Candidate native review results are missing.'}
$nativeText=Get-Content -LiteralPath $nativeResult -Raw
$nativeMatch=[regex]::Match($nativeText,'Floors=18; physical sweeps=(\d+); captured_room_types=\d+; failures=0(?:\r?\n|$)')
if(!$nativeMatch.Success -or [int]$nativeMatch.Groups[1].Value -lt 1 -or $nativeText -match '(?m)^FAIL '){throw 'The packaged candidate must pass its complete 18-floor native review.'}
# Reject an editor-only result: the completion log must identify the candidate's native binary directory.
$candidateLogDirectory=(Join-Path $candidate 'DungeonCrawler\Binaries\Win64').Replace('\','/').TrimEnd('/')+'/'
$completedLogs=@()
foreach($log in Get-ChildItem -LiteralPath $evidence -File -Filter '*.log'){
 $content=Get-Content -LiteralPath $log.FullName -Raw
 if($content -match 'ROOM_KIT_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0' -and $content.Replace('\','/').Contains($candidateLogDirectory)){
  if($content -match 'Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_QA FAIL'){throw "Native review log contains a failure: $($log.FullName)"}
  $completedLogs+=$log.FullName
 }
}
if($completedLogs.Count -eq 0){throw 'No complete 18-floor review log identifies the packaged RoomKitCandidate executable.'}
for($floor=1;$floor -le 18;$floor++){
 foreach($scene in @('Entrance','BossApproach','SealedStairs')){
  $capture=Join-Path (Split-Path -Parent $nativeResult) ('Floor_{0:D2}_{1}.png' -f $floor,$scene)
  if(!(Test-Path -LiteralPath $capture -PathType Leaf)){throw "Required native evidence screenshot missing: $capture"}
 }
}
$performance=@(Read-RequiredJson (Join-Path $evidence 'performance_results.json'))
if($performance.Count -lt 1){throw 'Warmed native performance results are missing.'}
foreach($result in $performance){
 if($result.samples -ne 1300 -or $result.label -notmatch '^[A-Za-z0-9_-]+$' -or $result.scene -notmatch '^[A-Za-z0-9_-]+$'){throw 'Invalid native performance sample metadata.'}
 foreach($metric in @('fps','mean_ms','p99_ms','p99_fps')){
  $value=[double]$result.$metric
  if($value -le 0 -or [double]::IsNaN($value) -or [double]::IsInfinity($value)){throw "Invalid performance metric $metric in $($result.scene)."}
 }
 $performanceDirectory=Assert-ContainedPath (Join-Path (Join-Path $evidence 'Performance') $result.label) $evidence
 foreach($extension in @('csv','png','log')){
  if(!(Test-Path -LiteralPath (Join-Path $performanceDirectory ($result.scene+'.'+$extension)) -PathType Leaf)){throw "Performance source evidence missing: $($result.scene).$extension"}
 }
 $benchmarkText=Get-Content -LiteralPath (Join-Path $performanceDirectory ($result.scene+'.log')) -Raw
 if($benchmarkText -notmatch 'ROOM_KIT_BENCHMARK_READY' -or $benchmarkText -notmatch 'CSV finalize time' -or $benchmarkText -match 'Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_BENCHMARK_FAILED'){throw "Performance capture did not finish cleanly: $($result.scene)"}
}

$preservedBefore=Invoke-PreservationCheck 'before_promotion'
$manifest=@()
foreach($file in Get-RuntimeFiles $candidate){
 $relative=$file.FullName.Substring($candidate.Length).TrimStart([char[]]'\/')
 if($relative -match '(?i)(^|[\\/])Saved([\\/]|$)'){throw 'A Saved directory entered the runtime inventory.'}
 $manifest+=[pscustomobject]@{Path=$relative;Length=$file.Length;SHA256=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash}
}
if($manifest.Count -lt 4){throw 'Candidate runtime inventory is unexpectedly incomplete.'}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'promotion_candidate_manifest.json') -Encoding utf8

# Unexpected leftover content containers could override the newly validated package.
# Do not delete them automatically: stop before touching either installed runtime.
foreach($target in $targets){
 $pakRoot=Join-Path $target 'DungeonCrawler\Content\Paks'
 if(Test-Path -LiteralPath $pakRoot){
  foreach($container in Get-ChildItem -LiteralPath $pakRoot -File -Recurse){
   if($container.Extension -notin @('.pak','.ucas','.utoc')){continue}
   $relative=$container.FullName.Substring($target.Length).TrimStart([char[]]'\/')
   if($relative -notin $manifest.Path){throw "Unexpected installed content container requires review before promotion: $($container.FullName)"}
  }
 }
}
Assert-NoRunningGame
$targetReports=@()
foreach($target in $targets){
 Assert-NoRunningGame
 foreach($directory in $runtimeDirectories){
  $source=Assert-ContainedPath (Join-Path $candidate $directory) $candidate
  $destination=Assert-ContainedPath (Join-Path $target $directory) $target
  & robocopy $source $destination /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /XD Saved /XJ /NFL /NDL /NJH /NJS /NP
  if($LASTEXITCODE -ge 8){throw "Runtime copy failed: $destination"}
 }
 foreach($file in Get-ChildItem -LiteralPath $candidate -File -Force){Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $target $file.Name) -Force}
 $matched=0
 foreach($entry in $manifest){
  $installed=Assert-ContainedPath (Join-Path $target $entry.Path) $target
  if(!(Test-Path -LiteralPath $installed -PathType Leaf)){throw "Installed runtime file is missing: $installed"}
  if((Get-Item -LiteralPath $installed).Length -ne $entry.Length -or (Get-FileHash -LiteralPath $installed -Algorithm SHA256).Hash -ne $entry.SHA256){throw "Installed runtime hash mismatch: $installed"}
  ++$matched
 }
 $targetReports+=[pscustomobject]@{Path=$target;MatchedRuntimeFiles=$matched;SHA256Verified=$true}
}
# Detect a candidate being rebuilt during promotion rather than bless a moving target.
foreach($entry in $manifest){
 $source=Join-Path $candidate $entry.Path
 if((Get-Item -LiteralPath $source).Length -ne $entry.Length -or (Get-FileHash -LiteralPath $source -Algorithm SHA256).Hash -ne $entry.SHA256){throw "Candidate changed during promotion: $source"}
}
$preservedAfter=Invoke-PreservationCheck 'after_promotion'
$report=[ordered]@{
 Status='PASS';Candidate=$candidate;RuntimeFiles=$manifest.Count;Targets=$targetReports
 AutomatedSuites=$passed;ImportedMeshes=@($geometry.meshes).Count;RegionalMaterials=@($materials.materials).Count
 NativeFloors=18;PhysicalSweeps=[int]$nativeMatch.Groups[1].Value;NativeLogs=$completedLogs;PerformanceScenes=$performance.Count
 AuthoredWalkingLaneRays=$walkways.rayCount
 ProtectedFiles=$preservedAfter.protected;UnchangedProtectedFiles=$preservedAfter.unchanged
 SavedDirectoriesExcluded=$true;PlayerSettingsModified=$false;PlayerSavesModified=$false;CompletedUtc=(Get-Date).ToUniversalTime().ToString('o')
}
$report | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $evidence 'promotion.json') -Encoding utf8
Write-Output "ROOM_KIT_PROMOTED: both launchers updated; $($manifest.Count) runtime files SHA256-verified per target; all $($preservedAfter.protected) protected files unchanged; every Saved directory excluded."
