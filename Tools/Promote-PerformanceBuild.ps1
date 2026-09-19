$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$sourceRoot=Join-Path $projectRoot 'Builds\PerformanceCandidate\Windows'
$reportRoot=Join-Path $projectRoot 'Saved\PerformancePass'
$campaign=Get-Content -LiteralPath "$reportRoot\campaign_review.log" -Raw
$prototype=Get-Content -LiteralPath "$reportRoot\prototype_review.log" -Raw
$tests=Get-Content -LiteralPath "$reportRoot\Automation\index.json" -Raw | ConvertFrom-Json
if($campaign -notmatch 'CAMPAIGN_EXPANSION_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0' -or $prototype -notmatch 'EXPANSION_REVIEW_COMPLETE failures=0' -or $tests.failed -ne 0 -or $tests.notRun -ne 0){throw 'Candidate validation is incomplete.'}
if(Get-Process -Name DungeonCrawler -ErrorAction SilentlyContinue){throw 'Close the running game before updating its runtime.'}
foreach($destination in @('Builds\Windows','Builds\ExpansionPrototype\Windows')){
 $targetRoot=Join-Path $projectRoot $destination
 # Update only runtime directories and top-level package files. Never touch Saved.
 foreach($relative in @('Engine','DungeonCrawler\Binaries','DungeonCrawler\Content')){
  $from=Join-Path $sourceRoot $relative
  $to=Join-Path $targetRoot $relative
  New-Item -ItemType Directory -Path $to -Force | Out-Null
  & robocopy $from $to /E /R:2 /W:1 /NFL /NDL /NJH /NJS /NP
  if($LASTEXITCODE -gt 7){throw "Runtime copy failed: $to"}
 }
 Get-ChildItem -LiteralPath $sourceRoot -File | ForEach-Object {Copy-Item -LiteralPath $_.FullName -Destination (Join-Path $targetRoot $_.Name) -Force}
 Write-Output "Updated $destination; Saved directory excluded."
}
exit 0
