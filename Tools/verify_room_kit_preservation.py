"""Verify protected art, saves and settings without changing them."""
import hashlib, json
from pathlib import Path
r=Path(__file__).resolve().parents[1];out=r/'Saved/RoomKitPass'
before=json.loads((out/'protected_before.json').read_text(encoding='utf-8-sig'))
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for chunk in iter(lambda:f.read(1024*1024),b''):h.update(chunk)
 return h.hexdigest().upper()
items=[]
for b in before:
 p=r/b['Path'];ok=p.is_file() and p.stat().st_size==b['Length'] and sha(p)==b['SHA256']
 items.append(dict(path=b['Path'],unchanged=ok))
report=dict(status='PASS' if all(x['unchanged'] for x in items) else 'FAIL',protected=len(items),unchanged=sum(x['unchanged'] for x in items),files=items)
(out/'preservation.json').write_text(json.dumps(report,indent=2))
print(json.dumps({k:v for k,v in report.items() if k!='files'}))
if report['status']!='PASS':raise SystemExit(1)
