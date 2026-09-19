"""Verify original saves/art/campaign and unrelated source; optionally restore review display preferences."""
from pathlib import Path
import json,hashlib,shutil,sys
R=Path(__file__).resolve().parents[1];E=R/'Saved/SewerArtIntegration';rows=json.loads((E/'baseline.json').read_text())
allowed={'Source/DungeonCrawler/DungeonEnvironment.cpp','Source/DungeonCrawler/DungeonGame.cpp','Source/DungeonCrawler/DungeonGame.h','Config/DefaultGame.ini'}
restore='--restore-preferences' in sys.argv
result={'art':0,'saves':0,'campaign':0,'unrelated_source':0,'preferences_restored':[],'expected_changes':[],'mismatches':[]}
for row in rows:
 rel=Path(row['relative']);p=Path(row['path']);parts=rel.parts
 if 'Saved' in parts and 'Config' in parts:
  if restore:
   shutil.copy2(E/'Backup'/rel,p);result['preferences_restored'].append(str(rel))
  else:continue
 current=hashlib.sha256(p.read_bytes()).hexdigest() if p.exists() else None
 key=rel.as_posix()
 if key in allowed:result['expected_changes'].append(key);continue
 if current!=row['sha256']:result['mismatches'].append(key)
 if '.art' in parts:result['art']+=1
 elif p.suffix.lower()=='.sav':result['saves']+=1
 elif key=='Content/Game/Data/campaign.json':result['campaign']+=1
 elif parts[0]=='Source':result['unrelated_source']+=1
(E/'preservation_results.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2));assert not result['mismatches']
