$ErrorActionPreference='Stop'
$projectRoot=Split-Path -Parent $PSScriptRoot
$evidence=Join-Path $projectRoot 'Saved\IntegrityUpdate'
$result=[ordered]@{}
foreach($kind in @('art','saves')) {
    $baseline=Get-Content -LiteralPath (Join-Path $evidence ($kind+'_before.json')) -Raw | ConvertFrom-Json
    $failures=@()
    foreach($entry in $baseline) {
        if(!(Test-Path -LiteralPath $entry.Path) -or (Get-FileHash -LiteralPath $entry.Path -Algorithm SHA256).Hash -ne $entry.Hash) { $failures+=$entry.Path }
    }
    $result[$kind]=[ordered]@{checked=$baseline.Count; mismatches=$failures}
}
$result['campaignUnchanged']=(Get-FileHash -LiteralPath (Join-Path $projectRoot 'Content\Game\Data\campaign.json')).Hash -eq (Get-FileHash -LiteralPath (Join-Path $evidence 'Backup\campaign.json')).Hash
$result | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'preservation_results.json') -Encoding utf8
if($result.art.mismatches.Count -or $result.saves.mismatches.Count -or !$result.campaignUnchanged){throw 'Preservation mismatch. Inspect Saved\IntegrityUpdate\preservation_results.json.'}
Write-Output "Verified $($result.art.checked) source art files, $($result.saves.checked) original saves, and unchanged campaign data."
