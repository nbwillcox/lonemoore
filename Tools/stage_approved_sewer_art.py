from pathlib import Path
import json,hashlib,shutil
r=Path.cwd();e=r/'Saved/SewerArtIntegration';snap=[]
paths=list((r/'.art').rglob('*'))+list((r/'Source').rglob('*'))+list((r/'Config').glob('*.ini'))+[r/'Content/Game/Data/campaign.json']
for folder in [r/'Saved/SaveGames',r/'Builds/Windows/DungeonCrawler/Saved/SaveGames',r/'Saved/Config',r/'Builds/Windows/DungeonCrawler/Saved/Config']:
 if folder.exists():paths+=list(folder.rglob('*'))
for p in paths:
 if not p.is_file():continue
 rel=p.relative_to(r);snap.append({'path':str(p),'relative':str(rel),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
 if '.art' not in rel.parts:
  dest=e/'Backup'/rel;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,dest)
(e/'baseline.json').write_text(json.dumps(snap,indent=2))
stage=Path(r'J:\Lonemoore_Art_OldCitySewers');dest=r/'Content/OldCitySewers';assert not dest.exists(),'Review existing regional content before overwriting'
for sub in ['Textures','Materials','Meshes']:
 for p in (stage/'UnrealTest/Content/OldCitySewers'/sub).glob('*.uasset'):
  if p.stem in ['ShowcaseCorridor','M_ReviewEmber','M_OCS_ReviewDecalCard'] or 'ReviewCard' in p.stem:continue
  out=dest/sub/p.name;out.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,out)
for name in ['manifest.json','VISUAL_SPEC.md','INTEGRATION_HANDOFF.md']:
 shutil.copy2(stage/name,r/'ArtReview/OldCitySewers'/name)
(e/'approval.txt').write_text('Owner approved the first pass and authorized main-game integration and real in-game testing on 2026-09-16. Staging-only gate is superseded for this regional pack. Preserve gameplay, other regions, saves and source artwork.\n')
print('BACKUP_AND_REGIONAL_COPY',len(snap),'tracked files')
