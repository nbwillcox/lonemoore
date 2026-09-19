param([int]$Width=1600,[int]$Height=900)
$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$evidence=Join-Path $projectRoot "Saved\Storefront\Packaged-${Width}x${Height}"
$exe=Join-Path $projectRoot 'Builds\Windows\DungeonCrawler\Binaries\Win64\DungeonCrawler.exe'
New-Item -ItemType Directory -Force -Path $evidence | Out-Null
$reviewLog=Join-Path $evidence 'review.log'
$userDir=Join-Path $evidence 'ReviewUser'
$gameArgs=@('-StorefrontReview','-RenderOffscreen','-windowed',"-ResX=$Width","-ResY=$Height",'-ForceRes','-unattended','-nosplash','-nosound',"-UserDir=`"$userDir`"","-abslog=`"$reviewLog`"")
$process=Start-Process -FilePath $exe -ArgumentList $gameArgs -WindowStyle Hidden -PassThru
Write-Output "Storefront review started: PID $($process.Id), ${Width}x${Height}."
if(-not $process.WaitForExit(240000)){Stop-Process -Id $process.Id;throw 'Storefront review timed out.'}
$process.Refresh()
$log=Get-Content -LiteralPath $reviewLog -Raw
if($process.ExitCode -ne 0 -or $log -notmatch 'STOREFRONT_REVIEW_COMPLETE failures=0' -or $log -match 'STOREFRONT_REVIEW .* FAIL|Assertion failed:|Fatal error:'){throw "Storefront review failed. See $reviewLog"}
Write-Output "Storefront review passed: $reviewLog"
