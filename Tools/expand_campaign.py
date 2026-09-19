"""Fixed, authored floor layouts. This writes content; no runtime topology generation."""
import json,pathlib
root=pathlib.Path(__file__).resolve().parents[1]
p=root/'Content/Game/Data/campaign.json';data=json.loads(p.read_text(encoding='utf-8'))
data['floors']=data['floors'][:1]
data['floors'][0]['rows']=[row.replace('H','C') for row in data['floors'][0]['rows']]
layouts=[
'''###############
#S....#....C..#
#.##..#.###.#.#
#..K..+...#.#.#
###.#####.#.#.#
#...#C?...E#..#
#.###.###.###.#
#...E...#.....#
#.#####.#####.#
#...T...L...#.#
#.###.#####.#.#
#C#...D...E...#
#.###.#.#####.#
#..?$.G...V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#K#...#...+...#
#.#####.###.#.#
#...E...#...#.#
###.#.###.###.#
#...#...E.....#
#.#####.###.#.#
#C?...#...L.#.#
#####.###.###.#
#...T.D...B...#
#.###.#####.#.#
#$?..G....V.>.#
###############''',
'''###############
#S......#....C#
#.#####.#.###.#
#...K.#.+...#.#
###.#.#.###.#.#
#C?.#...E...#.#
###.###.#####.#
#...#R..#.....#
#.###.#.#.###.#
#.....#...T...#
#.#######.###.#
#...E.D...L...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#.#...#K..+...#
#.#####.###.###
#...E...#.....#
###.###.#.###.#
#...#C?.E...#.#
#.###.#####.#.#
#...T.....L...#
#.#######.###.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S....#....C..#
#.###.#.#####.#
#K..#.+.....#.#
#.#.#.###.#.#.#
#.#.E...#.#...#
#.#####.#.###.#
#...#C?.#...E.#
###.###.###.#.#
#...T...#...#.#
#.#####.#.###.#
#C....D...L...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#K#...#...+...#
#.###.#.###.#.#
#...E.#...#.#.#
###.#####.#.#.#
#C?.#...E...#.#
#.#.#.#####.#.#
#...T.....L...#
#.#######.###.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S......#....C#
#.#####.#.###.#
#...K.#.+...#.#
#.###.#.###.#.#
#...E...#...#.#
###.###.#.###.#
#C?...#...R...#
#####.###.###.#
#...T...E.L...#
#.###.#####.#.#
#.....D.......#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#.#...#K..+...#
#.#####.#.###.#
#.....E.#.....#
#.###.###.###.#
#...#C?.E...#.#
#.###.#####.#.#
#.....T...L...#
#####.###.###.#
#C....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S....#....C..#
#.###.#.###.#.#
#K..#.+...#.#.#
###.#.###.#.#.#
#...E...#...#.#
#.#####.#####.#
#...#R..#...E.#
#.#.#.#.#.###.#
#.#...#...T...#
#.#######.###.#
#C....D...L...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S......#....C#
#.#####.#.###.#
#...K.#.+...#.#
#.###.#.###.#.#
#...E...#.....#
###.#.###.###.#
#C?.#...E...#.#
#.#######.###.#
#.....T...L...#
#.###.#####.#.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#K#...#...+...#
#.#####.###.#.#
#...E...#...#.#
#.###.#.#.###.#
#...#C?.E.....#
###.#####.###.#
#...T.....L...#
#.###.#####.#.#
#.....D...E...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S....#....C..#
#.###.#.###.#.#
#K..#.+...#.#.#
#.#.#.###.#.#.#
#.#.E...#...#.#
#.#####.###.#.#
#C?...#...E...#
#####.###.###.#
#...T.....L...#
#.###.#####.#.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S......#....C#
#.#####.#.###.#
#...K.#.+...#.#
###.#.#.###.#.#
#C?.#...E.....#
#.###.#####.#.#
#...#...#...#.#
#.#.#.#.#.###.#
#.#...#R..T...#
#.###.#####.#.#
#...E.D...L...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S..#.....#..C#
#.#.#.###.#.#.#
#.#...#K..+...#
#.#####.###.#.#
#...E...#.....#
#.###.###.###.#
#C?.#...E...#.#
###.#####.###.#
#...T.....L...#
#.###.#####.#.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S....#....C..#
#.###.#.###.#.#
#K..#.+...#.#.#
#.#.#.###.#.#.#
#...E...#...#.#
###.###.###.#.#
#C?...#...E...#
#.###.#.###.#.#
#...T.....L...#
#.###.#####.#.#
#.....D...E...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S......#....C#
#.#####.#.###.#
#...K.#.+...#.#
#.###.#.###.#.#
#...E...#...#.#
###.###.###.#.#
#C?...#...E...#
#.###.#.###.#.#
#...T.....L...#
#.###.#####.#.#
#.....D...B...#
#.###.#.#####.#
#$?..G....V.>.#
###############''',
'''###############
#S....#....C..#
#.###.#.###.#.#
#K..#.+...#.#.#
#.#.#.###.#.#.#
#.#.E...#...#.#
#.#####.###.#.#
#C?...#...E...#
#####.###.###.#
#...T.....L...#
#.###.#####.#.#
#.....D.......#
#.###.#.#####.#
#$?..G....V.A.#
###############''']
regions=[
('Old City Sewers',['The Drowned Conduit','The Rat King\'s Cistern'],['fungal','beetle','slime'],'ratking','Rusted Key',[.21,.29,.20]),
('Forgotten Catacombs',['The Failed Chapel','The Ossuary'],['skeleton','archer','wraith'],'keeper','Crypt Key',[.34,.32,.28]),
('Goblin Warrens',['The Scavenger Roads','The Thorn Court'],['goblin','brute','shaman'],'matriarch','Rusted Key',[.30,.25,.15]),
('Ancient Crypts',['The Pilgrim\'s Vigil','The Crimson Sepulchre'],['knight','wraith','gargoyle','vampire'],'vampiress','Crypt Key',[.24,.24,.33]),
('Buried Fortress',['The Warden\'s Cells','The Hollow Keep'],['bandit','hound','knight','gargoyle'],'castellan','Warden Key',[.26,.29,.32]),
('The Deep',['The Chasm of Wings','The Drake\'s Heart'],['bat','spider','beetle','minotaur'],'agni','Drake Lair Key',[.15,.25,.29]),
('Infernal Ruins',['The Sealed Witness','The Ashen Compact'],['succubus','hellknight','witch'],'ashwarden','Master Key',[.32,.18,.16]),
('Hell',['The Ashen Threshold','The Hellforge','The Broken Halo'],['succubus','hellknight','minotaur'],'gatewarden','Hell Key',[.19,.12,.18])]
def alias(id,name,art,source,family):
    if any(e['id']==id for e in data['enemies']):return
    base=next(e.copy() for e in data['enemies'] if e['id']==source);base.update(id=id,name=name,art=art,family=family,hp=3.1);data['enemies'].append(base)
alias('keeper','The Ossuary Keeper','cryptGuardian1','castellan','Undead')
alias('vampiress','Enraged Vampiress','enragedVampiress1','vampire','Undead')
alias('ashwarden','Warden of Ash [dev presentation]','minotaurJailer1','minotaur','Demon')
alias('gatewarden','The Last Gatekeeper [dev presentation]','ArmoredFemaleSuccubus1','succubus','Demon')
barks={3:'Cleric: I came to bury my brothers. The graves opened before I could begin. Let me help you end this.',5:'Paladin: My expedition is gone. But an oath does not end because its witnesses are dead.',4:'Rogue: Treasure brought me here. A locked cell changed my priorities. You have my blades.',6:'Warlock: This is no curse leaking from a tomb. It is a door. And something on the other side knows your name.'}
lore=[
'The old channels carry no rain. Every drop seeps upward from below.',
'The crowns in these graves predate the city. Lonemoore was built on a warning.',
'The goblins trade in broken seals. They have mistaken the locks for treasure.',
'Holy names were scraped from every coffin. The last inscription reads: Do not answer her.',
'The garrison fought inward, not outward. This fortress was built to keep something underneath it.',
'Beyond the chasm, the stars appear below your feet. The descent has left the mortal world.',
'Seven generations fed the seals. The last keeper chose power instead. The breach remembers.',
'Astra wears the Hell Key. The door is bound to her will. Killing her may break its master, but not its hunger.']
index=0
for regionIndex,(region,names,enemies,boss,key,tint) in enumerate(regions,1):
    for sub,name in enumerate(names,1):
        rows=layouts[index].splitlines();recruit={3:3,7:5,9:4,13:6}.get(index+1,-1)
        if regionIndex==8 and sub==3:floorboss=''
        elif sub==len(names) or (regionIndex==8 and sub==2):floorboss=boss
        else:floorboss=''
        # An optional encounter occupies the center of each first floor.
        data['floors'].append(dict(name=f'{region} — Level {sub}: {name}',region=region,regionIndex=regionIndex,rank=index+2,rows=rows,enemies=enemies,boss=floorboss,key=key,recruit=recruit,recruitBark=barks.get(recruit,''),lore=lore[regionIndex-1],tint=tint))
        index+=1
assert len(data['floors'])==18
for floor in data['floors']:
    # Each authored floor has a fixed optional bounty alcove.
    for y,row in enumerate(floor['rows']):
        if 'C' in row:
            floor['rows'][y]=row.replace('C','H',1)
            break
    if floor['region']=='Hell':floor['key']='Master Key (Hell)'
    elif floor['region']=='Infernal Ruins':floor['key']='Master Key (Infernal Ruins)'
p.write_text(json.dumps(data,indent=2),encoding='utf-8')
print('Wrote 18 fixed campaign floors.')
