"""Verify protected files and the two delivered runtime packages."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
evidence = root / 'Saved/CampaignExpansion'

def digest(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest().upper()

protected = json.loads((evidence / 'protected_before.json').read_text(encoding='utf-8-sig'))
changed = [entry['Path'] for entry in protected
           if not Path(entry['Path']).is_file() or digest(Path(entry['Path'])) != entry['Hash']]
main = root / 'Builds/Windows'
prototype = root / 'Builds/ExpansionPrototype/Windows'
runtime_paths = [Path('DungeonCrawler.exe'), Path('DungeonCrawler/Binaries/Win64/DungeonCrawler.exe')]
runtime_paths += [p.relative_to(main) for p in (main / 'DungeonCrawler/Content/Paks').iterdir() if p.is_file()]
runtime = []
for relative in runtime_paths:
    campaign_hash = digest(main / relative)
    prototype_hash = digest(prototype / relative)
    runtime.append({'path': str(relative), 'sha256': campaign_hash,
                    'prototype_matches_campaign': campaign_hash == prototype_hash})
report = {'protected_files': len(protected), 'changed_protected_files': changed, 'runtime': runtime}
(evidence / 'preservation.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps(report, indent=2))
assert not changed, 'Protected files changed; review before delivery.'
assert all(p['prototype_matches_campaign'] for p in runtime), 'Runtime packages differ.'
