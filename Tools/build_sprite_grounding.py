"""Measure original sprite transparency; never crop or change approved artwork."""
import hashlib
import json
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
campaign = json.loads((ROOT / 'Content/Game/Data/campaign.json').read_text())
manifest = json.loads((ROOT / 'Content/Game/Data/asset_manifest.json').read_text())
enemies = {entry['art'] for entry in campaign['enemies']} | {'final_boss'}
records = []
for entry in manifest:
    art = entry['asset'].split('.')[-1].removeprefix('T_')
    if art not in enemies:
        continue
    path = ROOT / entry['source']
    with Image.open(path) as original:
        alpha = original.convert('RGBA').getchannel('A')
        # Matches the ordinary one-third opacity-mask threshold.
        bounds = alpha.point(lambda value: 255 if value >= 85 else 0).getbbox()
        assert bounds, art
        padding = (original.height - bounds[3]) / original.height
        assert 0 <= padding < .35, (art, padding)
        records.append(dict(art=art, bottomPadding=padding, source=entry['source'],
                            sourceSha256=hashlib.sha256(path.read_bytes()).hexdigest(),
                            width=original.width, height=original.height, solidBounds=list(bounds)))
assert {record['art'] for record in records} == enemies
output = ROOT / 'Content/Game/Data/sprite_grounding.json'
output.write_text(json.dumps(dict(alphaThreshold=85, floorInsetCm=4, sprites=records), indent=2))
evidence = ROOT / 'Saved/AdventurePolish'
evidence.mkdir(parents=True, exist_ok=True)
(evidence / 'sprite_grounding.json').write_text(json.dumps(dict(status='PASS', count=len(records),
    sourceArtworkModified=False, dataSha256=hashlib.sha256(output.read_bytes()).hexdigest(), sprites=records), indent=2))
print(f'SPRITE_GROUNDING_PASS sprites={len(records)}; original artwork unchanged')
