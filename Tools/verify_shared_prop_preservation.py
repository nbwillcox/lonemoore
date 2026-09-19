"""Check original content, campaign and saves; optionally restore review display preferences."""
from pathlib import Path
import hashlib,json,shutil,sys
R=Path(__file__).resolve().parents[1];D=R/'Saved/SharedPropUpdate'
allowed={'Source/DungeonCrawler/DungeonGame.cpp','Source/DungeonCrawler/DungeonGame.h','Source/DungeonCrawler/DungeonEnvironment.cpp','Config/DefaultGame.ini'}
counts={'original_uassets':0,'original_art':0,'saves':0,'campaign':0};mismatches=[]
for row in json.loads((D/'baseline.json').read_text()):
 p=R/row['path'];rel=p.relative_to(R)
 if p.name=='GameUserSettings.ini':
  if '--restore-preferences' not in sys.argv:continue
  shutil.copy2(D/'Backup'/rel,p)
 if rel.as_posix() in allowed:continue
 if not p.exists() or hashlib.sha256(p.read_bytes()).hexdigest()!=row['sha256']:mismatches.append(str(rel))
 if p.suffix=='.uasset':counts['original_uassets']+=1
 elif p.suffix=='.sav':counts['saves']+=1
 elif rel.parts[0]=='.art':counts['original_art']+=1
 elif p.name=='campaign.json':counts['campaign']+=1
result={'counts':counts,'mismatches':mismatches,'preferences_restored':'--restore-preferences' in sys.argv};(D/'preservation_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2));assert not mismatches
