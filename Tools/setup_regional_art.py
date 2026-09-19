from pathlib import Path
import json,hashlib,shutil
G=Path(r'J:\First Person Dungeon Crawler Game');R=Path(r'J:\Lonemoore_Regional_Art')
for f in ['scripts','sources','textures','props','previews','reports','UnrealTest/Config','UnrealTest/Content']:(R/f).mkdir(parents=True,exist_ok=True)
E=G/'Saved/RegionalArt';E.mkdir(parents=True,exist_ok=True)
if not (E/'baseline.json').exists():
 paths=list((G/'Source').rglob('*'))+list((G/'Config').glob('*.ini'))+list((G/'Content').rglob('*.uasset'))+list((G/'.art').rglob('*'))+[G/'Content/Game/Data/campaign.json']
 for p in [G/'Saved',G/'Builds/Windows/DungeonCrawler/Saved']:
  paths+=list(p.rglob('*.sav'))+list(p.rglob('GameUserSettings.ini'))
 rows=[]
 for p in paths:
  if not p.is_file() or 'Backup' in p.parts:continue
  rel=p.relative_to(G);rows.append({'path':str(rel),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
  if rel.parts[0] in ['Source','Config'] or p.suffix=='.sav' or p.name=='GameUserSettings.ini':
   d=E/'Backup'/rel;d.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,d)
 (E/'baseline.json').write_text(json.dumps(rows,indent=2))
shutil.copy2(G/'DUNGEON_REGIONAL_ART_PLAN.md',R/'APPROVED_PLAN.md')
(R/'UnrealTest/RegionalArtTest.uproject').write_text(json.dumps({'FileVersion':3,'EngineAssociation':'5.8','Plugins':[{'Name':'PythonScriptPlugin','Enabled':True},{'Name':'EditorScriptingUtilities','Enabled':True}]}))
(R/'UnrealTest/Config/DefaultEngine.ini').write_text('[/Script/Engine.RendererSettings]\nr.DefaultFeature.AutoExposure=False\nr.AllowStaticLighting=False\nr.DynamicGlobalIlluminationMethod=1\nr.ReflectionMethod=1\n')
# Order: wall, secondary wall, floor, vault, trim, metal, accent, detail.
spec=[
(0,'Cathedral','Last Dawn Cathedral',[
 ('PilgrimLimestone','ashlar',[.38,.35,.29],3,5),('RuinedLimePlaster','plaster',[.46,.42,.34],0,0),('ProcessionalFlags','flags',[.34,.32,.28],4,4),('ChapelVault','brick',[.32,.28,.23],6,10),('BellCarving','engraved',[.46,.42,.32],2,4),('OxidizedIron','metal',[.20,.11,.055],0,0),('OldPewOak','wood',[.29,.19,.10],6,0),('AltarDressing','ashlar',[.45,.42,.34],2,3)],['WaxRun','PlasterLoss','SootFan','DampFoot'],['AltarFragment','BellAnchor','BrokenPew']),
(2,'Catacombs','Forgotten Catacombs',[
 ('BurialRubble','ashlar',[.32,.30,.25],5,7),('FuneraryPlaster','plaster',[.40,.37,.29],0,0),('SettledFlags','flags',[.30,.29,.25],3,5),('BurialVault','brick',[.29,.26,.21],8,12),('CrownMemorial','engraved',[.41,.36,.26],3,3),('AgedBronze','metal',[.28,.23,.105],0,0),('ChalkBone','bone',[.56,.51,.39],0,0),('NicheStone','ashlar',[.37,.34,.28],4,8)],['ChalkBloom','DustFall','WaxTrace','MemorialSpall'],['BoneShelf','MemorialPlaque','FuneraryUrn']),
(3,'Warrens','Goblin Warrens',[
 ('PatchedRubble','ashlar',[.29,.27,.19],4,7),('ScavengedPlanks','wood',[.26,.18,.095],9,0),('TroddenEarth','gravel',[.235,.20,.13],0,0),('RootStrata','strata',[.255,.245,.16],0,0),('TimberBracing','wood',[.24,.15,.075],4,0),('BatteredIron','metal',[.25,.13,.065],0,0),('RootMoss','growth',[.145,.20,.07],0,0),('WovenScraps','cloth',[.30,.25,.13],0,0)],['MudSplash','MossPatch','SalvageScuff','EarthFall'],['SalvageBundle','SealTray','ThornCluster']),
(4,'Crypts','Ancient Crypts',[
 ('SepulchralAshlar','ashlar',[.31,.32,.36],2,5),('ScrapedMemorial','engraved',[.39,.39,.42],2,2),('VigilSlabs','flags',[.29,.29,.32],2,3),('RibVaultStone','brick',[.27,.27,.32],4,12),('FuneraryCarving','engraved',[.42,.40,.39],4,3),('TarnishedSilver','metal',[.30,.29,.25],0,0),('CrimsonCloth','cloth',[.25,.055,.05],0,0),('CoffinStone','veins',[.37,.36,.38],0,0)],['ErasedName','CandleWax','CrimsonDust','ColdSeep'],['Nameplate','CoffinCorner','VotiveStand']),
(5,'Fortress','Buried Fortress',[
 ('GarrisonBlocks','ashlar',[.285,.31,.33],3,4),('DefenseMasonry','ashlar',[.255,.28,.30],4,6),('GarrisonFlags','flags',[.27,.29,.30],4,3),('UtilityVault','brick',[.26,.275,.285],5,8),('MilitaryTrim','engraved',[.33,.35,.35],3,4),('WardenIron','metal',[.21,.13,.08],0,0),('BarracksOak','wood',[.24,.17,.11],5,0),('BarredPlate','plate',[.26,.275,.285],2,3)],['RustRun','BootScuff','SootStain','SaltSeep'],['ShackleFixture','ShieldRack','ChainGuide']),
(6,'Deep','The Deep',[
 ('ChasmStrata','strata',[.225,.275,.285],0,0),('FracturedCave','rock',[.215,.25,.27],0,0),('CaveGravel','gravel',[.23,.255,.25],0,0),('OverhangRock','strata',[.20,.24,.25],0,0),('MineralCrust','veins',[.31,.37,.35],0,0),('AncientIron','metal',[.16,.23,.22],0,0),('ScorchedDrakeRock','slag',[.22,.16,.13],0,0),('DarkSediment','gravel',[.17,.205,.21],0,0)],['MineralBloom','ClawScars','SootScorch','CaveSeep'],['MineralCluster','DrakeScales','RockSpur']),
(7,'Infernal','Infernal Ruins',[
 ('SealMasonry','ashlar',[.29,.235,.21],3,6),('SealCarvedSlabs','engraved',[.34,.27,.22],2,3),('AshFlags','flags',[.285,.26,.24],3,4),('FracturedVault','brick',[.27,.21,.19],5,9),('RitualTrim','engraved',[.36,.29,.22],3,5),('HeatIron','metal',[.25,.12,.075],0,0),('ScorchedPlaster','plaster',[.28,.235,.21],0,0),('HeatFissures','heat',[.20,.14,.12],0,0)],['AshFall','BrokenSeal','HeatScorch','RitualResidue'],['SealFragment','RitualClamp','OfferingVessel']),
(8,'Hell','Hell',[
 ('BasaltRampart','ashlar',[.23,.205,.22],2,4),('FracturedBasalt','rock',[.215,.185,.22],0,0),('ForgePlates','plate',[.245,.225,.23],3,3),('VaultBasalt','strata',[.23,.18,.215],0,0),('ObsidianCarving','engraved',[.27,.245,.30],2,4),('InfernalMetal','metal',[.28,.13,.09],0,0),('ForgeSlag','slag',[.265,.19,.15],0,0),('EmberSeams','heat',[.19,.135,.15],0,0)],['AshDrift','ForgeScorch','BrokenSigil','SlagSpatter'],['SlagCluster','ChainAnchor','BrokenHalo'])]
camp=json.loads((G/'Content/Game/Data/campaign.json').read_text());regions=[]
for idx,code,name,mats,decs,props in spec:
 floors=[f for f in camp['floors'] if f['regionIndex']==idx]
 regions.append({'index':idx,'code':code,'name':name,'floors':[f['name'] for f in floors],'lore':floors[0]['lore'],'materials':[dict(id=code+'_'+a,kind=k,color=c,nx=nx,ny=ny,role=role) for (a,k,c,nx,ny),role in zip(mats,['wall','secondary','floor','vault','trim','metal','accent','detail'])],'decals':[code+'_'+d for d in decs],'props':[code+'_'+p for p in props]})
(R/'regions.json').write_text(json.dumps(regions,indent=2));print('REGIONAL_WORKSPACE_READY',R)
