$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$candidate=Join-Path $root 'Builds\FineTuneCandidate\Windows'
$evidence=Join-Path $root 'Saved\FineTunePass'
if(Get-Process DungeonCrawler* -ErrorAction SilentlyContinue){throw 'Close the running game before replacing its executable.'}
foreach($check in @(@('fine_review.log','FINETUNE_REVIEW_COMPLETE failures=0'),@('campaign_review.log','CAMPAIGN_EXPANSION_REVIEW_COMPLETE floors=18 sweeps=\d+ failures=0'))){
 if((Get-Content -LiteralPath (Join-Path $evidence $check[0]) -Raw) -notmatch $check[1]){throw "Incomplete packaged validation: $($check[0])"}
}
$tests=Get-Content -LiteralPath "$evidence\AutomationFinal\index.json" -Raw | ConvertFrom-Json
if($tests.failed -ne 0 -or ($tests.succeeded+$tests.succeededWithWarnings) -lt 20){throw 'Native tests did not pass.'}
foreach($target in @((Join-Path $root 'Builds\Windows'),(Join-Path $root 'Builds\ExpansionPrototype\Windows'))){
 foreach($sub in @('Engine','DungeonCrawler\Binaries','DungeonCrawler\Content')){
  & robocopy (Join-Path $candidate $sub) (Join-Path $target $sub) /E /COPY:DAT /DCOPY:DAT /R:2 /W:1 /NFL /NDL /NJH /NJS /NP
  if($LASTEXITCODE -ge 8){throw "Runtime copy failed: $target\$sub"}
 }
 Get-ChildItem -LiteralPath $candidate -File | Copy-Item -Destination $target -Force
 # The only intentional player-preference change: enable synchronization once.
 # Future choices in the graphics menu remain authoritative.
 $config=Join-Path $target 'DungeonCrawler\Saved\Config\Windows\GameUserSettings.ini'
 $text=Get-Content -LiteralPath $config -Raw
 if($text -notmatch 'bUseVSync='){throw 'Expected display preference missing.'}
 $text=[regex]::Replace($text,'(?m)^bUseVSync=.*$','bUseVSync=True')
 [IO.File]::WriteAllText($config,$text)
}
Write-Output 'FINETUNE_PROMOTED: both launchers updated; save directories excluded; VSync enabled.'

