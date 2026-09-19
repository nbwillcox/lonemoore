"""Copy read-only approved sewer materials into the isolated comparison project."""
from pathlib import Path
import shutil,hashlib,json
G=Path(r'J:\First Person Dungeon Crawler Game');R=Path(r'J:\Lonemoore_Regional_Identity')
src=G/'Content/OldCitySewers';dst=R/'UnrealTest/Content/OldCitySewers';rows=[]
for p in src.rglob('*.uasset'):
 q=dst/p.relative_to(src);q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
 assert hashlib.sha256(p.read_bytes()).digest()==hashlib.sha256(q.read_bytes()).digest();rows.append(str(p.relative_to(G)))
(R/'reports/benchmark_copy.json').write_text(json.dumps(dict(direction='main -> isolated test only',files=rows),indent=2))
print('READ_ONLY_BENCHMARK_STAGED',len(rows))
