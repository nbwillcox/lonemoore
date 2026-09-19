import json,pathlib,collections,sys
root=pathlib.Path(__file__).resolve().parents[1]
data=json.loads((root/'Content/Game/Data/campaign.json').read_text(encoding='utf-8'))
errors=[];checks=0
def check(ok,message):
    global checks
    checks+=1
    if not ok:errors.append(message)
enemy={e['id'] for e in data['enemies']}
for floor in data['floors']:
    rows=floor['rows'];width=len(rows[0]);name=floor['name']
    check(all(len(r)==width for r in rows),name+': uneven rows')
    starts=[(x,y) for y,r in enumerate(rows) for x,t in enumerate(r) if t=='S']
    check(len(starts)==1,name+': exactly one entrance shrine')
    check(sum(r.count('V') for r in rows)==1,name+': one descent shrine')
    check(any('>' in r or 'A' in r for r in rows),name+': exit or final choice')
    check(floor['boss'] in enemy or not floor['boss'],name+': valid boss')
    check(all(e in enemy for e in floor['enemies']),name+': valid enemies')
    def reach(blocked):
        seen=set(starts);q=collections.deque(starts)
        while q:
            x,y=q.popleft()
            for a,b in [(x+1,y),(x-1,y),(x,y+1),(x,y-1)]:
                if 0<=b<len(rows) and 0<=a<width and rows[b][a] not in blocked and (a,b) not in seen:
                    seen.add((a,b));q.append((a,b))
        return seen
    seen=reach('#')
    for y,r in enumerate(rows):
        for x,t in enumerate(r):
            if t!='#':check((x,y) in seen,f'{name}: isolated {t} at {x},{y}')
    unlocked=reach('#DG')
    for y,r in enumerate(rows):
        for x,t in enumerate(r):
            if t=='K':check((x,y) in unlocked,name+': key behind its own lock')
    check(any('?' in r for r in rows),name+': needs a secret')
    if len(data['floors'])>1:check(sum(r.count('H') for r in rows)==1,name+': one fixed bounty alcove')
for c in data['classes']:check(len(c['skills'])==3,c['name']+': exactly 3 skills')
check(len(data['classes'])==7,'seven classes')
check(len(data['skills'])==21,'21 active skills')
result=dict(checks=checks,passed=checks-len(errors),failed=len(errors),errors=errors,floors=len(data['floors']))
(root/'Saved/Validation/content.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result,indent=2))
sys.exit(bool(errors))
