"""Verify this optimization pass preserved original art, player saves and settings."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
evidence = root / 'Saved/PerformancePass'

def digest(path):
    with path.open('rb') as source:
        return hashlib.file_digest(source, 'sha256').hexdigest().upper()

protected = json.loads((evidence / 'protected_before.json').read_text(encoding='utf-8-sig'))
changed = [p['Path'] for p in protected
           if not Path(p['Path']).is_file() or digest(Path(p['Path'])) != p['Hash']]
candidate = root / 'Builds/PerformanceCandidate/Windows'
main = root / 'Builds/Windows'
prototype = root / 'Builds/ExpansionPrototype/Windows'
paths = [Path('DungeonCrawler.exe'), Path('DungeonCrawler/Binaries/Win64/DungeonCrawler.exe')]
paths += [p.relative_to(candidate) for p in (candidate / 'DungeonCrawler/Content/Paks').iterdir() if p.is_file()]
runtime = []
for relative in paths:
    expected = digest(candidate / relative)
    runtime.append({'path': str(relative), 'sha256': expected,
                    'campaign_matches': digest(main / relative) == expected,
                    'prototype_matches': digest(prototype / relative) == expected})
report = {'protected_files': len(protected), 'changed_protected_files': changed, 'runtime': runtime}
(evidence / 'preservation.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps(report, indent=2))
assert not changed, 'Protected files changed; review before delivery.'
assert all(p['campaign_matches'] and p['prototype_matches'] for p in runtime), 'Packages differ.'
