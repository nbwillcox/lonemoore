"""Create a new sibling art revision; never replace the approved source packs."""
from pathlib import Path
import json, hashlib, shutil
G=Path(r'J:\First Person Dungeon Crawler Game')
R=Path(r'J:\Lonemoore_Regional_Identity')
E=G/'Saved/RegionalIdentity'
for p in ['scripts','sources','textures','meshes','previews','reports','UnrealTest/Config','UnrealTest/Content']:(R/p).mkdir(parents=True,exist_ok=True)
E.mkdir(parents=True,exist_ok=True)
if not (E/'baseline.json').exists():
 rows=[]
 paths=list((G/'Source').rglob('*'))+list((G/'Config').glob('*.ini'))+list((G/'Content').rglob('*.uasset'))+list((G/'.art').rglob('*'))+[G/'Content/Game/Data/campaign.json']
 for base in [G/'Saved',G/'Builds/Windows/DungeonCrawler/Saved']:
  paths+=list(base.rglob('*.sav'))+list(base.rglob('GameUserSettings.ini'))
 for p in paths:
  if not p.is_file() or 'Backup' in p.parts:continue
  rel=p.relative_to(G);rows.append(dict(path=str(rel),sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
  if rel.parts[0] in ['Source','Config'] or p.suffix=='.sav' or p.name=='GameUserSettings.ini':
   q=E/'Backup'/rel;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
 (E/'baseline.json').write_text(json.dumps(rows,indent=2))
 for p in (G/'ArtReview/RegionalArt/Packaged').glob('*.png'):
  q=R/'previews/before'/p.name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
specs=[
 (0,'Cathedral','Chalk lime walls with exposed dressed foundations; pale processional inlay; plaster soffits and pointed limestone ribs.', [('LimeWall','lime',[.55,.50,.41]),('ProcessionalInlay','inlay',[.43,.40,.34]),('PlasterSoffit','plaster',[.48,.44,.36]),('CarvedLimestone','cutstone',[.42,.38,.30])]),
 (2,'Catacombs','Dry rubble and deep stacked ossuary compartments; chipped limestone flags; heavy shelf lintels and burial coffers.', [('BurialRubble','rubble',[.39,.35,.28]),('ChippedFlags','brokenflags',[.38,.35,.29]),('BurialSoffit','smallbrick',[.32,.28,.22]),('ChalkLimestone','chalk',[.49,.44,.33])]),
 (3,'Warrens','Excavated earth and shale with salvaged timber braces; trodden soil and small stones; roots and crude overhead planks.', [('EarthShale','shale',[.31,.29,.21]),('PackedEarth','earth',[.30,.24,.15]),('RootSoil','roots',[.29,.26,.18]),('ScavengedOak','wood',[.36,.24,.12])]),
 (4,'Crypts','Formal tall funerary panels, veined pale stone and dark borders; contrasting stone inlay floor; recessed stone coffers.', [('VigilMarble','marble',[.47,.48,.47]),('MortuaryInlay','checker',[.44,.44,.40]),('CofferedStone','darkmarble',[.24,.25,.26]),('SepulchralBorder','darkmarble',[.25,.27,.29])]),
 (5,'Fortress','Massive squared defensive blocks and iron straps; herringbone fired pavers; flat oak and iron supported ceilings.', [('DefenseBlocks','largeblocks',[.37,.38,.36]),('GarrisonPavers','herringbone',[.34,.27,.21]),('BarracksPlanks','wood',[.31,.25,.18]),('ForgedStraps','iron',[.24,.25,.25])]),
 (6,'Deep','Angular exposed rock faces with mineral inclusions; fractured bedrock underfoot; faceted hanging overburden and rock lintels.', [('ChasmRock','rock',[.29,.34,.35]),('FracturedBedrock','bedrock',[.31,.34,.33]),('Overburden','rock',[.25,.29,.30]),('MineralEdges','mineral',[.38,.43,.40])]),
 (7,'Infernal','Monolithic scorched ritual slabs with narrow seal joints; shattered ash covered pavements; tapered red stone ribs and dark ceiling slabs.', [('SealMonolith','monolith',[.38,.28,.22]),('ShatteredPavement','brokenflags',[.36,.30,.25]),('ScorchedSoffit','scorch',[.30,.24,.22]),('RitualEdges','seal',[.40,.29,.20])]),
 (8,'Hell','Fractured columnar basalt and angular iron bracing, cooled slag underfoot, hanging volcanic teeth and furnace ribs. Sparse embers stay within fractures.', [('ColumnBasalt','basalt',[.245,.235,.22]),('CooledSlag','lava',[.26,.24,.21]),('HangingBasalt','basaltroof',[.23,.225,.21]),('FurnaceIron','iron',[.255,.245,.22]),('HellforgePlates','forge',[.28,.26,.23]),('BrokenHaloStone','obsidian',[.29,.285,.265])])
]
campaign=json.loads((G/'Content/Game/Data/campaign.json').read_text())
regions=[];materials=[]
for idx,code,direction,defs in specs:
 fs=[f for f in campaign['floors'] if f['regionIndex']==idx]
 regions.append(dict(index=idx,code=code,name=fs[0]['name'].split(' — ')[0],floors=[f['name'] for f in fs],lore=fs[0]['lore'],direction=direction))
 for role,(name,kind,color) in zip(['wall','floor','ceiling','trim','forge_floor','halo_floor'],defs):
  materials.append(dict(id=code+'_'+name,region=idx,role=role,kind=kind,color=color,repeat_m=2,resolution=[2048,2048]))
manifest=dict(version=2,status='IN_PRODUCTION',regions=regions,materials=materials,decals=[],props=[],conventions=dict(scale='metres in Blender, centimetres in Unreal; 4 m grid retained',base_color='sRGB, unlit diffuse reflectance only',normal='DirectX +X -Y +Z, linear, BC5 in Unreal',roughness='linear grayscale',density='1024 pixels/metre; each texture repeats over 2 m',collision='original invisible collision components retained, new visual meshes have NoCollision'))
(R/'manifest.json').write_text(json.dumps(manifest,indent=2))
(R/'UnrealTest/RegionalIdentityTest.uproject').write_text(json.dumps(dict(FileVersion=3,EngineAssociation='5.8',Plugins=[dict(Name='PythonScriptPlugin',Enabled=True),dict(Name='EditorScriptingUtilities',Enabled=True)])))
(R/'UnrealTest/Config/DefaultEngine.ini').write_text('[/Script/Engine.RendererSettings]\nr.DefaultFeature.AutoExposure=False\nr.AllowStaticLighting=False\nr.DynamicGlobalIlluminationMethod=1\nr.ReflectionMethod=1\n')
(R/'DIRECTION.md').write_text('# Regional identity correction\n\nThe approved campaign and original concept images remain authoritative. Old City Sewers stays the approved benchmark. This revision changes the visible building fabric in the other eight regions. No room shape, routes, gate states, collision, progression, enemies, saves or lore change.\n\n'+'\n\n'.join('## '+r['name']+'\n'+r['direction']+'\n\nLore: '+r['lore'] for r in regions)+'\n\nImage generation is available but was not needed for these procedural surfaces and modeled architectural forms. No illustration is claimed.\n')
print('IDENTITY_WORKSPACE_READY',R,flush=True)
