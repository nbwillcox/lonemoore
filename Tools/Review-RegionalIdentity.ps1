$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$evidenceRoot = Join-Path $projectRoot 'Saved\RegionalIdentity'
$gameExe = Join-Path $projectRoot 'Builds\Windows\DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
$packagedSaved = Join-Path $projectRoot 'Builds\Windows\DungeonCrawler\Saved'
if (Get-Process -Name DungeonCrawler -ErrorAction SilentlyContinue) {
    throw 'A game session is already running; leave it untouched.'
}
foreach ($review in @('RegionalArtReview', 'SharedPropReview')) {
    $logName = if ($review -eq 'RegionalArtReview') { 'packaged_review.log' } else { 'shared_packaged_review.log' }
    $reviewLog = Join-Path $evidenceRoot $logName
    $gameArgs = @("-$review", '-RenderOffscreen', '-unattended', '-NoSound', '-windowed', '-ResX=1600', '-ResY=900', "-abslog=`"$reviewLog`"")
    $process = Start-Process -FilePath $gameExe -ArgumentList $gameArgs -WindowStyle Hidden -PassThru
    Write-Output "Started $review, process $($process.Id)"
    $process.WaitForExit()
    if ($process.ExitCode -ne 0) { throw "$review exited with code $($process.ExitCode)" }
    $log = Get-Content -LiteralPath $reviewLog -Raw
    $marker = if ($review -eq 'RegionalArtReview') { 'REGIONAL_REVIEW_COMPLETE floors=18 sweeps=6882 failures=0' } else { 'SHARED_PROP_REVIEW_COMPLETE failures=0' }
    if (-not $log.Contains($marker)) { throw "$review did not pass: inspect $reviewLog" }
    if ($log -match '(?m)\b(?:Error|Fatal):|REGIONAL_QA FAIL|SHARED_PROP FAIL|Assertion failed:') {
        throw "$review reported a runtime error: inspect $reviewLog"
    }
    $folder = if ($review -eq 'RegionalArtReview') { 'RegionalIdentity' } else { 'SharedPropUpdate' }
    $target = Join-Path $evidenceRoot $folder
    New-Item -ItemType Directory -Path $target -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $packagedSaved "$folder\runtime_results.txt") -Destination $target
    Write-Output "$review passed"
}
