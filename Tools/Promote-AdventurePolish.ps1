$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$projectRoot=(Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$candidate=Join-Path $projectRoot 'Builds\AdventurePolishCandidate\Windows'
$evidence=Join-Path $projectRoot 'Saved\AdventurePolish'
$targets=@('Builds\Windows','Builds\ExpansionPrototype\Windows','Builds\RoomKitCandidate\Windows')
$runtimeDirs=@('Engine','DungeonCrawler\Binaries','DungeonCrawler\Content')
function ReadJson([string]$Path){return Get-Content -LiteralPath $Path -Raw | ConvertFrom-Json}
function RuntimeFiles([string]$Base){
 $files=@(Get-ChildItem -LiteralPath $Base -File)
 foreach($dir in $runtimeDirs){$files+=Get-ChildItem -LiteralPath (Join-Path $Base $dir) -Recurse -File | Where-Object {$_.FullName -notmatch '[\\/]Saved[\\/]'}}
 return @($files | Sort-Object FullName)
}
function AssertTree([string]$Base){
 $full=[IO.Path]::GetFullPath($Base);$allowed=[IO.Path]::GetFullPath((Join-Path $projectRoot 'Builds'))+'\'
 if(!$full.StartsWith($allowed,[StringComparison]::OrdinalIgnoreCase)){throw "Runtime outside Builds: $full"}
 if((Get-Item -LiteralPath $Base).Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Linked runtime: $Base"}
 foreach($dir in (@('DungeonCrawler')+$runtimeDirs)){if((Get-Item -LiteralPath (Join-Path $Base $dir)).Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Linked runtime root: $Base/$dir"}}
 foreach($file in RuntimeFiles $Base){if($file.Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Linked runtime file: $($file.FullName)"}}
 foreach($dir in $runtimeDirs){foreach($item in Get-ChildItem -LiteralPath (Join-Path $Base $dir) -Recurse -Directory){if($item.Attributes -band [IO.FileAttributes]::ReparsePoint){throw "Linked runtime directory: $($item.FullName)"}}}
}
if(Get-Process -Name 'DungeonCrawler*','UnrealEditor*' -ErrorAction SilentlyContinue){throw 'Close running game/editor before installing.'}
$tests=ReadJson (Join-Path $evidence 'AutomationFinal\index.json')
if($tests.failed -ne 0 -or $tests.notRun -ne 0 -or $tests.inProcess -ne 0 -or ($tests.succeeded+$tests.succeededWithWarnings) -lt 35){throw 'Final complete regression suite has not passed.'}
foreach($test in $tests.tests){if($test.state -ne 'Success'){throw "Unsuccessful test: $($test.fullTestPath)"}}
foreach($required in @('Dungeon.Profiles.InitialSaveAndCreationRollback','Dungeon.Profiles.IsolatedSaveBrowserAndHardcore','Dungeon.Profiles.NamesAndSevenClasses','Dungeon.Audio.ImportedEffects','Dungeon.Adventure.DescentCheckpointAndGates','Dungeon.Adventure.ChestCatalogAndDepthQuality','Dungeon.Story.CompleteRoutesAndEndings','Dungeon.RoomKit.EncounterRoomCoverage')){if($required -notin $tests.tests.fullTestPath){throw "Missing required suite: $required"}}
$audio=ReadJson (Join-Path $evidence 'combat_audio_import.json')
if($audio.status -ne 'PASS' -or $audio.imported.Count -ne 27 -or $audio.failures -ne 0){throw 'Combat audio import incomplete.'}
$performance=ReadJson (Join-Path $evidence 'performance_release_evidence.json')
if($performance.status -ne 'PASS'){throw 'Performance evidence incomplete.'}
$exe=Join-Path $candidate 'DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
$hash=(Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
if($performance.sceneCount -ne 7 -or $performance.candidateExecutableSha256 -ne $hash){throw 'Performance evidence belongs to another executable or lacks scenes.'}
foreach($name in @('packaged_adventure','packaged_all_floors')){
 $text=Get-Content -LiteralPath (Join-Path $evidence ($name+'.log')) -Raw
 $marker=if($name -eq 'packaged_adventure'){'ADVENTURE_POLISH_REVIEW_COMPLETE failures=0'}else{'ROOM_KIT_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0'}
 if($text -notmatch $marker -or $text -match 'ADVENTURE_QA FAIL|ROOM_KIT_QA FAIL|Fatal error:|Assertion failed:|GPU Crashed'){throw "Native validation incomplete: $name"}
 if(!$text.Replace('\','/').Contains($candidate.Replace('\','/')+'/DungeonCrawler/Binaries/Win64/')){throw "Review used a different runtime: $name"}
 if((Get-Item -LiteralPath (Join-Path $evidence ($name+'.log'))).LastWriteTimeUtc -lt (Get-Item -LiteralPath $exe).LastWriteTimeUtc){throw 'Runtime changed after native checks.'}
}
AssertTree $candidate
$source=@(RuntimeFiles $candidate);$sourceManifest=@($source | ForEach-Object {[pscustomobject]@{Path=$_.FullName.Substring($candidate.Length+1);SHA256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash}})
$protected=@()
foreach($relative in $targets){
 $target=Join-Path $projectRoot $relative;AssertTree $target
 $saved=Join-Path $target 'DungeonCrawler\Saved'
 if(Test-Path -LiteralPath $saved){foreach($file in Get-ChildItem -LiteralPath $saved -File -Recurse | Where-Object {$_.FullName -match '[\\/](SaveGames|Config)[\\/]'}){$protected+=[pscustomobject]@{Path=$file.FullName;SHA256=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash}}}
}
$record=[ordered]@{status='RUNNING';candidate=$candidate;executableSha256=$hash;nativeFloors=18;automationSuites=$tests.tests.Count;targets=@();runtime=$sourceManifest;protectedFiles=$protected.Count}
$report=Join-Path $evidence 'promotion.json'
$record | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $report -Encoding utf8
foreach($relative in $targets){
 $target=Join-Path $projectRoot $relative
 foreach($row in $sourceManifest){$destination=Join-Path $target $row.Path;New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null;Copy-Item -LiteralPath (Join-Path $candidate $row.Path) -Destination $destination -Force}
 foreach($row in $sourceManifest){if((Get-FileHash -LiteralPath (Join-Path $target $row.Path) -Algorithm SHA256).Hash -ne $row.SHA256){throw "Installed runtime mismatch: $relative/$($row.Path)"}}
 $record.targets+=@{path=$target;files=$sourceManifest.Count;status='PASS'}
}
foreach($row in $protected){if((Get-FileHash -LiteralPath $row.Path -Algorithm SHA256).Hash -ne $row.SHA256){throw "Saved game or preference changed: $($row.Path)"}}
$record.status='PASS';$record.completedUtc=[DateTime]::UtcNow.ToString('o');$record | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $report -Encoding utf8
Write-Output "Installed $($sourceManifest.Count) verified runtime files in $($targets.Count) launch locations; $($protected.Count) existing save/settings files untouched."
