param([switch]$Visible)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$evidence = Join-Path $projectRoot 'Saved\CombatFXPass'
$exe = Join-Path $projectRoot 'Builds\Windows\DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
if (-not (Test-Path -LiteralPath $exe)) { throw 'Build the Windows game first with Tools\Build.ps1 -Package.' }
if (Get-Process -Name DungeonCrawler -ErrorAction SilentlyContinue) { throw 'A game session is already open. Close it before running the effects review.' }
New-Item -ItemType Directory -Force -Path $evidence | Out-Null
$reviewLog = Join-Path $evidence 'packaged-review.log'
$userDir = Join-Path $evidence 'ReviewUser'
$gameArgs = @('-CombatFXReview', '-windowed', '-ResX=1600', '-ResY=900', '-unattended', "-UserDir=`"$userDir`"", "-abslog=`"$reviewLog`"")
if (-not $Visible) { $gameArgs += '-RenderOffscreen' }
$style = if ($Visible) { 'Normal' } else { 'Hidden' }
$process = Start-Process -FilePath $exe -ArgumentList $gameArgs -WindowStyle $style -PassThru
Write-Output "Combat effects review started (PID $($process.Id)); sound is enabled."
if (-not $process.WaitForExit(300000)) {
    Stop-Process -Id $process.Id
    throw 'Combat effects review exceeded five minutes; inspect Saved\CombatFXPass\packaged-review.log.'
}
$process.Refresh()
$log = Get-Content -LiteralPath $reviewLog -Raw
if ($process.ExitCode -ne 0 -or $log -notmatch 'COMBAT_FX_REVIEW_COMPLETE' -or $log -match 'COMBAT_FX_REVIEW_FAIL|Assertion failed:|Fatal error:') {
    throw "Combat effects review did not pass. Inspect $reviewLog"
}
Write-Output "Combat effects review passed. Log: $reviewLog"
