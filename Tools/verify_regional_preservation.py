from pathlib import Path
import json,hashlib,shutil,sys
G=Path(__file__).resolve().parents[1];R=G/'Saved/RegionalArt'
allowed={'Source/DungeonCrawler/DungeonEnvironment.cpp','Source/DungeonCrawler/DungeonGame.cpp','Source/DungeonCrawler/DungeonGame.h','Source/DungeonCrawler/DungeonSharedPropReview.cpp','Config/DefaultGame.ini'}
counts={'original_uassets':0,'art':0,'saves':0,'campaign':0,'unrelated_source':0};bad=[]
for row in json.loads((R/'baseline.json').read_text()):
 p=G/row['path'];rel=p.relative_to(G)
 if p.name=='GameUserSettings.ini':
  if '--restore-preferences' not in sys.argv:continue
  shutil.copy2(R/'Backup'/rel,p)
 if rel.as_posix() in allowed:continue
 if not p.exists() or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:bad.append(str(rel))
 if p.suffix=='.uasset':counts['original_uassets']+=1
 elif p.suffix=='.sav':counts['saves']+=1
 elif rel.parts[0]=='.art':counts['art']+=1
 elif p.name=='campaign.json':counts['campaign']+=1
 elif rel.parts[0]=='Source':counts['unrelated_source']+=1
report={'counts':counts,'mismatches':bad,'preferences_restored':'--restore-preferences' in sys.argv};(R/'preservation.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2));assert not bad
