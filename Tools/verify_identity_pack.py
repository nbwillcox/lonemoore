"""Validate exported maps and the current main-project preservation baseline."""
from pathlib import Path
from PIL import Image
import numpy as np,json,hashlib,sys,shutil
G=Path(r'J:\First Person Dungeon Crawler Game');R=Path(r'J:\Lonemoore_Regional_Identity');M=json.loads((R/'manifest.json').read_text());E=G/'Saved/RegionalIdentity'
maps=[]
for a in M['materials']:
 assert set(['base_color','normal','roughness'])<=set(a['maps'])
 for role,path in a['maps'].items():
  p=R/path;im=Image.open(p);assert im.size==(2048,2048),(path,im.size)
  ar=np.asarray(im).astype(np.float32)/255
  assert np.isfinite(ar).all()
  if role=='normal':
   lengths=np.linalg.norm(ar*2-1,axis=-1);assert np.max(np.abs(lengths-1))<.008
  if role=='roughness':assert ar.min()>=.41 and ar.max()<=.99
  maps.append(dict(asset=a['id'],map=role,resolution=list(im.size),minimum=float(ar.min()),maximum=float(ar.max()),sha256=hashlib.sha256(p.read_bytes()).hexdigest(),seams=[dict(axis=axis,mean_boundary_delta=float(np.mean(np.abs(np.take(ar,0,axis)-np.take(ar,-1,axis))))) for axis in [0,1]]))
for p in M['props']:
 assert (R/p['fbx']).exists() and p['triangles']>0
 assert len(p['materials'])>0 and max(p['size_m'])<6
for code in [r['code'] for r in M['regions']]:assert (R/'sources'/(code+'_Architecture.blend')).exists()
report=dict(status='PASS',textures=len(maps),materials=len(M['materials']),meshes=len(M['props']),editable_blend_sources=8,maps=maps)
(R/'reports/export_validation.json').write_text(json.dumps(report,indent=2))
allowed=set()
if '--integrated' in sys.argv:allowed={'Source/DungeonCrawler/DungeonEnvironment.cpp','Source/DungeonCrawler/DungeonRegionalArt.cpp','Source/DungeonCrawler/DungeonRegionalArtReview.cpp','Config/DefaultGame.ini'}
bad=[];counts=dict(original_assets=0,reference_art=0,saves=0,campaign=0,source=0,config=0)
for row in json.loads((E/'baseline.json').read_text()):
 p=G/row['path'];rel=p.relative_to(G)
 if p.name=='GameUserSettings.ini' and '--restore-preferences' in sys.argv:shutil.copy2(E/'Backup'/rel,p)
 if rel.as_posix() in allowed:continue
 if not p.exists() or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:bad.append(str(rel))
 if p.suffix=='.uasset':counts['original_assets']+=1
 elif p.suffix=='.sav':counts['saves']+=1
 elif rel.parts[0]=='.art':counts['reference_art']+=1
 elif p.name=='campaign.json':counts['campaign']+=1
 elif rel.parts[0]=='Source':counts['source']+=1
 elif rel.parts[0]=='Config':counts['config']+=1
pres=dict(counts=counts,mismatches=bad,source_changes_allowed=sorted(allowed),preferences_restored='--restore-preferences' in sys.argv)
(E/'preservation.json').write_text(json.dumps(pres,indent=2));(R/'reports/preservation.json').write_text(json.dumps(pres,indent=2));print(json.dumps(dict(exports={k:v for k,v in report.items() if k!='maps'},preservation=pres),indent=2));assert not bad
