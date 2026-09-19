"""Apply the user-approved regional identity pack with a preservation record."""
from pathlib import Path
import hashlib
import json
import shutil

GAME = Path(r'J:\First Person Dungeon Crawler Game')
PACK = Path(r'J:\Lonemoore_Regional_Identity')
EVIDENCE = GAME / 'Saved/RegionalIdentity'
DESTINATION = GAME / 'Content/RegionalIdentity'

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

baseline = json.loads((EVIDENCE / 'baseline.json').read_text())
drift = [row['path'] for row in baseline if not (GAME / row['path']).is_file()
         or digest(GAME / row['path']) != row['sha256']]
assert not drift, f'Preservation baseline changed: {drift}'
assert not DESTINATION.exists(), 'Do not overwrite an existing integration.'
for name in ['unreal_textures', 'unreal_meshes']:
    report = json.loads((PACK / 'reports' / (name + '.json')).read_text())
    assert report.get('errors') == [], (name, report.get('errors'))
assert json.loads((PACK / 'reports/export_validation.json').read_text())['status'] == 'PASS'

sources = ['DungeonIdentityArt.h', 'DungeonIdentityArt.cpp', 'DungeonEnvironment.cpp',
           'DungeonRegionalArt.cpp', 'DungeonRegionalArtReview.cpp']
assert all((PACK / 'runtime' / name).is_file() for name in sources)
assets = [p for folder in ['Materials', 'Textures', 'Meshes']
          for p in (PACK / 'UnrealTest/Content/RegionalIdentity' / folder).rglob('*.uasset')]
assert len(assets) == 227, len(assets)
assert len({p.name for p in assets}) == len(assets)

# Preserve the currently accepted executable and cooked containers before rebuilding.
package_backup = EVIDENCE / 'Backup/AcceptedPackage'
assert not package_backup.exists(), 'An accepted-package backup already exists.'
package_rows = []
for path in (GAME / 'Builds/Windows').rglob('*'):
    if path.suffix.lower() not in ['.exe', '.pak', '.utoc', '.ucas']:
        continue
    relative = path.relative_to(GAME / 'Builds/Windows')
    target = package_backup / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(path, target)
    value = digest(path)
    assert digest(target) == value
    package_rows.append(dict(path=relative.as_posix(), sha256=value))
assert len(package_rows) >= 7
(EVIDENCE / 'accepted_package_backup.json').write_text(json.dumps(package_rows, indent=2))

installed = []
for path in assets:
    relative = path.relative_to(PACK / 'UnrealTest/Content/RegionalIdentity')
    target = DESTINATION / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(path, target)
    value = digest(path)
    assert digest(target) == value
    installed.append(dict(path=target.relative_to(GAME).as_posix(), sha256=value))
for name in sources:
    target = GAME / 'Source/DungeonCrawler' / name
    shutil.copy2(PACK / 'runtime' / name, target)
    installed.append(dict(path=target.relative_to(GAME).as_posix(), sha256=digest(target)))
config = GAME / 'Config/DefaultGame.ini'
text = config.read_text()
anchor = '+DirectoriesToAlwaysCook=(Path="/Game/RegionalArt")'
assert text.count(anchor) == 1 and '/Game/RegionalIdentity' not in text
config.write_text(text.replace(anchor, anchor + '\n+DirectoriesToAlwaysCook=(Path="/Game/RegionalIdentity")'))
installed.append(dict(path=config.relative_to(GAME).as_posix(), sha256=digest(config)))
(EVIDENCE / 'integration.json').write_text(json.dumps(dict(
    status='INTEGRATED_AWAITING_BUILD_AND_PACKAGED_VALIDATION',
    authorization='User explicitly approved main game integration of the new dungeon art.',
    runtime_assets=len(assets), package_backup=str(package_backup), files=installed), indent=2))
print('INTEGRATED', len(assets), 'new assets and', len(sources), 'rendering source files; original content preserved.')
