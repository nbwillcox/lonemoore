param([string]$PythonExecutable='C:\Python313\python.exe')
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$projectRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $projectRoot 'Builds\RoomKitPolishCandidate\Windows'
$evidence=Join-Path $projectRoot 'Saved\RoomKitPolish'
$roomEvidence=Join-Path $projectRoot 'Saved\RoomKitPass'
$targets=@((Join-Path $projectRoot 'Builds\Windows'),(Join-Path $projectRoot 'Builds\ExpansionPrototype\Windows'),(Join-Path $projectRoot 'Builds\RoomKitCandidate\Windows'))
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
 foreach($entry in Get-ChildItem -LiteralPath $Path -Recurse -Force){
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
 & $PythonExecutable (Join-Path $PSScriptRoot 'verify_room_kit_polish_preservation.py') | Out-Host
 if($LASTEXITCODE -ne 0){throw "Protected saves, settings or original artwork changed ($Phase). Runtime promotion stopped."}
 $result=Read-RequiredJson (Join-Path $evidence 'preservation.json')
 if($result.status -ne 'PASS' -or $result.protected -ne 145 -or $result.unchanged -ne $result.protected -or $result.baselineRows -ne 149 -or $result.generatedFixtureCount -ne 4){throw "Incomplete preservation proof ($Phase)."}
 Copy-Item -LiteralPath (Join-Path $evidence 'preservation.json') -Destination (Join-Path $evidence "preservation_$Phase.json") -Force
 return $result
}
function Assert-FileHash([string]$Path,[string]$Expected){
 if($Expected -notmatch '^[0-9a-fA-F]{64}$' -or !(Test-Path -LiteralPath $Path -PathType Leaf) -or (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash -ne $Expected){throw "Evidence source is missing or has changed: $Path"}
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
$newestRuntimeUtc=(Get-RuntimeFiles $candidate | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1).LastWriteTimeUtc

$tests=Read-RequiredJson (Join-Path $evidence 'AutomationFinal\index.json')
$passed=[int]$tests.succeeded+[int]$tests.succeededWithWarnings
if($passed -ne 25 -or @($tests.tests).Count -ne 25 -or $tests.failed -ne 0 -or $tests.notRun -ne 0 -or $tests.inProcess -ne 0){throw 'All 25 final native automation suites must finish successfully before promotion.'}
foreach($test in $tests.tests){if($test.state -ne 'Success'){throw "Incomplete or failed final suite: $($test.fullTestPath)"}}
foreach($required in @('Dungeon.RoomKit.EncounterRoomCoverage','Dungeon.RoomKit.EncounterMigrationPreservesProgress','Dungeon.Story.CompleteRoutesAndEndings')){
 if($required -notin $tests.tests.fullTestPath){throw "Required final regression suite missing: $required"}
}
$geometry=Read-RequiredJson (Join-Path $roomEvidence 'geometry_import.json')
if(@($geometry.meshes).Count -ne 26 -or $geometry.roomCount -ne 25 -or $geometry.failures -ne 0){throw 'The 26-mesh import must contain 25 rooms and zero failures.'}
$materials=Read-RequiredJson (Join-Path $roomEvidence 'materials.json')
if($materials.status -ne 'PASS' -or @($materials.materials).Count -ne 45){throw 'All 45 regional material imports must pass before promotion.'}
$dressing=Read-RequiredJson (Join-Path $projectRoot 'Saved\RegionalDressingPass\import_report.json')
if($dressing.status -ne 'PASS' -or $dressing.failures -ne 0 -or @($dressing.meshes).Count -ne 24 -or @($dressing.materials).Count -ne 45){throw 'The new regional dressing import must contain 24 props, 45 materials and zero failures.'}
foreach($prop in $dressing.meshes){
 if($prop.id -notmatch '^[a-z0-9_]+$'){throw 'Invalid imported prop identifier.'}
 Assert-FileHash (Join-Path $projectRoot ('ArtSource\RegionalDressing\Meshes\SM_RD_'+$prop.id+'.fbx')) $prop.sourceFbxSha256
}
$kit=Read-RequiredJson (Join-Path $projectRoot 'Content\Game\Data\room_kit.json')
$roomIds=@($kit.templates.id | Sort-Object -Unique)
if($roomIds.Count -ne 25){throw 'Expected 25 unique authored room templates.'}
$walkways=Read-RequiredJson (Join-Path $evidence 'walkways.json')
if(!$walkways.passed -or !$walkways.manifestUnchanged -or $walkways.roomCount -ne 25 -or @($walkways.rooms).Count -ne 25 -or $walkways.failureCount -ne 0 -or $walkways.rayCount -lt 1 -or @($walkways.excludedObjects).Count -ne 0){throw 'All 25 authored room meshes must pass the walking-lane audit with no object exclusions.'}
Assert-FileHash (Join-Path $projectRoot 'Content\Game\Data\room_kit.json') $walkways.manifestSha256
if(@(Compare-Object $roomIds @($walkways.rooms.id | Sort-Object -Unique)).Count -ne 0){throw 'Walking-lane evidence does not cover the current room catalog.'}
foreach($room in $walkways.rooms){
 if(!$room.passed -or !$room.sourceUnchanged -or @($room.failures).Count -ne 0){throw "Walking-lane audit failed: $($room.id)"}
 Assert-FileHash (Join-Path $projectRoot ('ArtSource\RoomKit\Scenes\SM_RK_'+$room.id+'.blend')) $room.sceneSha256
}
$floorCoverage=Read-RequiredJson (Join-Path $evidence 'floor_coverage.json')
if(!$floorCoverage.passed -or !$floorCoverage.roundTrip -or $floorCoverage.roomCount -ne 25 -or @($floorCoverage.rooms).Count -ne 25 -or @($floorCoverage.failures).Count -ne 0){throw 'All 25 rooms must pass the source and FBX floor-coverage audit.'}
if(@(Compare-Object $roomIds @($floorCoverage.rooms.id | Sort-Object -Unique)).Count -ne 0){throw 'Floor-coverage evidence does not cover the current room catalog.'}
foreach($room in $floorCoverage.rooms){
 if(!$room.unchanged){throw "Floor audit source changed: $($room.id)"}
 Assert-FileHash (Join-Path $projectRoot ('ArtSource\RoomKit\Scenes\SM_RK_'+$room.id+'.blend')) $room.sourceSha256
 Assert-FileHash (Join-Path $projectRoot ('ArtSource\RoomKit\Meshes\SM_RK_'+$room.id+'.fbx')) $room.fbxSha256
}

$nativeResult=Join-Path $candidate 'DungeonCrawler\Saved\RoomKit\runtime_results.txt'
if(!(Test-Path -LiteralPath $nativeResult -PathType Leaf)){throw 'Candidate native review results are missing.'}
if((Get-Item -LiteralPath $nativeResult).LastWriteTimeUtc -lt $newestRuntimeUtc){throw 'Candidate runtime changed after its native review; repeat the packaged review.'}
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
if($completedLogs.Count -eq 0){throw 'No complete 18-floor review log identifies the packaged RoomKitPolishCandidate executable.'}
$supplementalReviews=@(
 [pscustomobject]@{Name='packaged_dressing_review';Marker='ROOM_KIT_DRESSING_REVIEW_COMPLETE floors=18 failures=0(?:\r?\n|$)'},
 [pscustomobject]@{Name='packaged_floor_circular_ossuary';Marker='ROOM_KIT_FLOOR_DIAGNOSTIC_COMPLETE floor=0 rooms=1 views=4 failures=0(?:\r?\n|$)'},
 [pscustomobject]@{Name='packaged_floor_rounded_turn';Marker='ROOM_KIT_FLOOR_DIAGNOSTIC_COMPLETE floor=0 rooms=1 views=4 failures=0(?:\r?\n|$)'},
 [pscustomobject]@{Name='packaged_floor_cavern_hall';Marker='ROOM_KIT_FLOOR_DIAGNOSTIC_COMPLETE floor=0 rooms=1 views=4 failures=0(?:\r?\n|$)'}
)
foreach($review in $supplementalReviews){
 $logPath=Join-Path $evidence ($review.Name+'.log')
 if(!(Test-Path -LiteralPath $logPath -PathType Leaf)){throw "Final native validation log is missing: $logPath"}
 if((Get-Item -LiteralPath $logPath).LastWriteTimeUtc -lt $newestRuntimeUtc){throw "Native validation predates the current candidate runtime: $logPath"}
 $content=Get-Content -LiteralPath $logPath -Raw
 if($content -notmatch $review.Marker -or !$content.Replace('\','/').Contains($candidateLogDirectory) -or $content -match 'ROOM_KIT_QA FAIL|ROOM_DRESSING_MISSING|Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_\w+_COMPLETE[^\r\n]*failures=[1-9]'){
  throw "Final candidate native validation did not pass: $logPath"
 }
}
for($floor=1;$floor -le 18;$floor++){
 foreach($scene in @('Entrance','BossApproach','SealedStairs')){
  $capture=Join-Path (Split-Path -Parent $nativeResult) ('Floor_{0:D2}_{1}.png' -f $floor,$scene)
  if(!(Test-Path -LiteralPath $capture -PathType Leaf)){throw "Required native evidence screenshot missing: $capture"}
  if((Get-Item -LiteralPath $capture).LastWriteTimeUtc -lt $newestRuntimeUtc){throw "Native screenshot predates the current candidate runtime: $capture"}
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
  $performanceFile=Join-Path $performanceDirectory ($result.scene+'.'+$extension)
  if(!(Test-Path -LiteralPath $performanceFile -PathType Leaf)){throw "Performance source evidence missing: $($result.scene).$extension"}
  if((Get-Item -LiteralPath $performanceFile).LastWriteTimeUtc -lt $newestRuntimeUtc){throw "Performance evidence predates the current candidate runtime: $performanceFile"}
 }
 $benchmarkText=Get-Content -LiteralPath (Join-Path $performanceDirectory ($result.scene+'.log')) -Raw
 if($benchmarkText -notmatch 'ROOM_KIT_BENCHMARK_READY' -or $benchmarkText -notmatch 'CSV finalize time' -or !$benchmarkText.Replace('\','/').Contains($candidateLogDirectory) -or $benchmarkText -match 'Fatal error:|Assertion failed:|GPU Crashed|ROOM_KIT_BENCHMARK_FAILED'){throw "Packaged candidate performance capture did not finish cleanly: $($result.scene)"}
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
try {
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
} finally {
 # Always recheck protected data, even if a runtime copy or hash check fails.
 $preservedAfter=Invoke-PreservationCheck 'after_promotion'
}
if($preservedBefore.baselineSha256 -ne $preservedAfter.baselineSha256){throw 'Preservation baseline changed during promotion.'}
$report=[ordered]@{
 Status='PASS';Candidate=$candidate;RuntimeFiles=$manifest.Count;Targets=$targetReports
 AutomatedSuites=$passed;ImportedMeshes=@($geometry.meshes).Count;RegionalMaterials=@($materials.materials).Count
 RegionalDressingProps=@($dressing.meshes).Count;RegionalDressingMaterials=@($dressing.materials).Count;FloorCoverageRooms=$floorCoverage.roomCount
 NativeFloors=18;PhysicalSweeps=[int]$nativeMatch.Groups[1].Value;NativeLogs=$completedLogs;PerformanceScenes=$performance.Count
 DressingReviewFloors=18;RepairedFloorRooms=3;RepairedFloorViewsPerRoom=4;SupplementalNativeReviews=$supplementalReviews.Name
 AuthoredWalkingLaneRays=$walkways.rayCount
 ProtectedFiles=$preservedAfter.protected;UnchangedProtectedFiles=$preservedAfter.unchanged
 PreservationBaselineRows=$preservedAfter.baselineRows;GeneratedPerformanceFixtures=$preservedAfter.generatedFixtures
 SavedDirectoriesExcluded=$true;PlayerSettingsModified=$false;PlayerSavesModified=$false;CompletedUtc=(Get-Date).ToUniversalTime().ToString('o')
}
$report | ConvertTo-Json -Depth 7 | Set-Content -LiteralPath (Join-Path $evidence 'promotion.json') -Encoding utf8
Write-Output "ROOM_KIT_POLISH_PROMOTED: all three launcher runtimes updated; $($manifest.Count) runtime files SHA256-verified per target; all $($preservedAfter.protected) protected files unchanged; every Saved directory excluded."
