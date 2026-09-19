"""Verify player data, intentional VSync changes, and identical promoted packages."""
from pathlib import Path
import hashlib,json,re
r=Path(__file__).resolve().parents[1];out=r/'Saved/FineTunePass'
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
unchanged=[];settings=[]
for row in json.loads((out/'protected_before.json').read_text()):
    p=r/row['path'];assert p.exists(),p
    if p.name=='GameUserSettings.ini':
        old=(out/'Backup'/row['path']).read_text();new=p.read_text()
        expected=re.sub(r'(?m)^bUseVSync=.*$','bUseVSync=True',old)
        assert new==expected,(p,'Unexpected preference change')
        settings.append(str(p.relative_to(r)))
    else:
        assert digest(p)==row['sha256'],(p,'Protected content changed')
        unchanged.append(str(p.relative_to(r)))
candidate=r/'Builds/FineTuneCandidate/Windows';packages=[]
files=[candidate/'DungeonCrawler.exe',candidate/'DungeonCrawler/Binaries/Win64/DungeonCrawler.exe']
files+=list((candidate/'DungeonCrawler/Content/Paks').glob('*'))
for p in files:
    if not p.is_file():continue
    h=digest(p);rel=p.relative_to(candidate)
    for target in ['Builds/Windows','Builds/ExpansionPrototype/Windows']:
        assert digest(r/target/rel)==h,(target,rel)
    packages.append(str(rel))
report=dict(status='PASS',unchanged_files=len(unchanged),unchanged_saves=sum(p.endswith('.sav') for p in unchanged),
            vsync_only_settings=settings,matching_runtime_files=packages)
(out/'preservation.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))
