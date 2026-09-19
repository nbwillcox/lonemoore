"""Author deterministic content data. No source art is modified."""
import json, pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1]
out=ROOT/'Content/Game/Data'
out.mkdir(parents=True,exist_ok=True)
classes=[]
def cls(name,stats,primary,skills,passive,weapons,armor):
    classes.append(dict(name=name,art=name.lower(),stats=stats,primary=primary,skills=skills,passive=passive,weapons=weapons.split(','),armor=armor.split(',')))
cls('Warrior',[9,5,9,3,4,4],0,['Power Strike','Cleave','Shield Wall'],'Iron Resolve: +4 physical armor','dagger,sword,broadsword,axe,mace,spear','leather,studded,chain,scale,scaleplate,plate,ornate')
cls('Mage',[3,5,5,10,7,4],3,['Fireball','Ice Lance','Chain Lightning'],'Arcane Blood: +20% spell power','dagger,magestaff','cloth,magerobes')
cls('Ranger',[6,10,6,4,5,6],1,['Power Shot','Multi-Shot',"Hunter\'s Mark"],'Keen Eye: improved accuracy and critical chance','dagger,sword,spear,bow,crossbow','leather,studded,chain,scale')
cls('Cleric',[5,4,7,6,10,4],4,['Heal','Holy Smite','Resurrection'],'Mercy: +25% healing','mace,holystaff','cloth,priestrobes,chain,scale')
cls('Rogue',[6,11,5,4,4,9],1,['Backstab','Poison Blade','Small Grenade'],'Shadowcraft: critical chance and trap detection','dagger,sword,bow,crossbow','cloth,leather,studded')
cls('Paladin',[8,4,9,4,8,4],0,['Holy Strike','Divine Shield','Lay on Hands'],'Consecrated: armor and holy resistance','sword,broadsword,mace,spear,holystaff','chain,scale,scaleplate,plate,ornate')
cls('Warlock',[3,6,6,10,7,5],3,['Shadow Bolt','Curse','Soul Drain'],'Void Pact: +20% dark damage','dagger,magestaff','cloth,magerobes')
skills=[]
def sk(name,cost,power,typ='Physical',effect='',target='enemy',unlock=1):
    skills.append(dict(name=name,cost=cost,power=power,type=typ,effect=effect,target=target,unlock=unlock))
sk('Power Strike',3,1.7); sk('Cleave',6,1.1,target='all',unlock=5); sk('Shield Wall',8,1,effect='Guard',target='party',unlock=10)
sk('Fireball',4,1.5,'Fire','Burn');sk('Ice Lance',6,1.65,'Ice','Stun',unlock=5);sk('Chain Lightning',10,1.3,'Lightning',target='all',unlock=10)
sk('Power Shot',3,1.7);sk('Multi-Shot',6,1.1,target='all',unlock=5);sk("Hunter's Mark",7,1,effect='Mark',unlock=10)
sk('Heal',4,2,'Holy',target='heal');sk('Holy Smite',6,1.8,'Holy',unlock=5);sk('Resurrection',15,1,'Holy',target='revive',unlock=10)
sk('Backstab',4,1.9);sk('Poison Blade',5,1.2,'Physical','Poison',unlock=5);sk('Small Grenade',8,1.15,target='all',unlock=10)
sk('Holy Strike',4,1.5,'Holy');sk('Divine Shield',7,1,effect='Guard',target='party',unlock=5);sk('Lay on Hands',10,3,'Holy',target='heal',unlock=10)
sk('Shadow Bolt',4,1.6,'Dark');sk('Curse',6,.8,'Dark','Curse',unlock=5);sk('Soul Drain',10,1.8,'Dark','Drain',unlock=10)
items=[]
def it(id,name,family,slot,power,value,stack=1,effect=''):
    items.append(dict(id=id,name=name,family=family,slot=slot,power=power,value=value,stack=stack,effect=effect))
for id,name,power,cost,effect in [('health','Health Potion',45,18,'hp'),('greater_health','Greater Health Potion',150,55,'hp'),('mana','Mana Potion',25,22,'mp'),('greater_mana','Greater Mana Potion',90,65,'mp'),('food','Travel Bread',25,9,'hp'),('remedy','Cleansing Tonic',0,20,'cleanse')]:it(id,name,'consumable','',power,cost,5,effect)
for i,(id,name) in enumerate([('dagger','Dagger'),('sword','Sword'),('broadsword','Broadsword'),('axe','Axe'),('mace','Hammer / Mace'),('spear','Spear'),('bow','Bow'),('crossbow','Crossbow'),('holystaff','Holy Staff'),('magestaff','Mage Staff')]):it(id,name,id,'Weapon',5+i%4,45+i*5)
for i,(id,name) in enumerate([('cloth','Cloth'),('leather','Leather'),('studded','Studded Leather'),('chain','Chain'),('scale','Scale'),('scaleplate','Scale and Plate'),('plate','Plate'),('ornate','Ornate Plate'),('priestrobes','Priest Robes'),('magerobes','Mage Robes')]):it(id,name+' Armor',id,'Body',2+i%8,35+i*10)
for id,name,slot,power in [('shield','Shield','Off Hand',4),('hood','Travel Hood','Head',1),('gloves','Travel Gloves','Hands',1),('boots','Travel Boots','Feet',1),('ring','Silver Ring','Accessory 1',2),('charm','Dawn Charm','Accessory 2',2)]:it(id,name,'universal',slot,power,40)
it('antiquity','Sealed Antiquity','treasure','',0,180)
enemies=[]
def en(id,name,art,family,hp=1,attack=1,speed=1,typ='Physical',status='',resist=None):
    enemies.append(dict(id=id,name=name,art=art,family=family,hp=hp,attack=attack,speed=speed,type=typ,status=status,resist=resist or {}))
en('rat','Cursed Rat','rat1','Vermin',.7,.7,1.1);en('spider','Cave Spider','cavespider1','Vermin',.9,.8,1,'Physical','Poison')
en('cultist','Cult Enforcer','cultenforcer1','Cult',1.2,1,1);en('skeleton','Skeleton Warrior','skeleton1','Undead',1,1,.8,resist={'Holy':1.5,'Dark':.6})
en('fungal','Fungal Rat','fungalrat1','Vermin',1,.8,1,status='Poison');en('beetle','Sewer Beetle','beetle1','Vermin',1.3,.8,.6)
en('slime','Slime','slime1','Vermin',1.2,.7,.6,resist={'Physical':.7,'Fire':1.5});en('wraith','Wraith','wraith1','Undead',1.1,1,1.1,'Dark','Fear',{'Holy':1.5,'Dark':.5})
en('archer','Skeleton Archer','skeletonarcher1','Undead',.85,1.1,1.2);en('goblin','Goblin Scavenger','goblinscavenger1','Goblin',.9,1,1.1)
en('brute','Goblin Brute','goblinbrute1','Goblin',1.4,1.2,.65);en('shaman','Goblin Shaman','goblinshaman1','Goblin',1,1,1,'Fire','Burn')
en('knight','Restless Knight','restlessknight1','Undead',1.5,1.1,.7);en('gargoyle','Stone Gargoyle','stonegargoyle1','Construct',1.5,1,.8,resist={'Physical':.8,'Lightning':1.4})
en('vampire','Vampire Duelist','vampireDuelist1','Undead',1.2,1.2,1.3,status='Bleed');en('bandit','Bandit Cutthroat','banditCutthroat1','Soldier',1,1.2,1.2,status='Blind')
en('hound','Grave Hound','gravehound1','Beast',1.2,1.2,1.3);en('minotaur','Minotaur Jailer','minotaurJailer1','Beast',1.8,1.2,.6)
en('bat','Dungeon Bat','dungeonbat1','Beast',.7,.9,1.5);en('witch','Dark Witch','darkwitch1','Cult',1,1.2,1,'Dark','Curse')
en('succubus','Armored Succubus','ArmoredFemaleSuccubus1','Demon',1.3,1.2,1.1,'Dark','Curse',{'Dark':.6,'Holy':1.4})
en('hellknight','Infernal Enforcer [dev variant]','cultenforcer1','Demon',1.6,1.3,.8,'Fire','Burn',{'Fire':.5,'Ice':1.4})
en('ratking','The Rat King','TheRatKing','Vermin',3,1.2,.9,status='Poison');en('widow','The Widow Queen','TheWidowQueen','Vermin',3,1.2,1.2,status='Poison')
en('castellan','The Hollow Castellan','TheHollowCastellan','Undead',3.2,1.4,.8,status='Fear');en('matriarch','The Blackthorn Matriarch','TheBlackthornMatriarch','Cult',2.8,1.3,1.2,'Dark','Curse')
en('agni','The Grand Agni Drake','TheGrandAgniDrake','Drake',3.4,1.4,1,'Fire','Burn',{'Fire':.5,'Ice':1.5})
en('reaper','The Death Reaper Drake','TheDeathReaperDrake','Drake',3.4,1.4,1,'Poison','Poison',{'Poison':0,'Holy':1.4})
en('astra','Astra, Cosmic Chaos Nephilim Queen','final_boss','Demon',4.2,1.5,1.3,'Dark','Curse',{'Dark':.65,'Holy':.8})
# Explicitly authored first floor. North is upward. E is a visible encounter;
# B a unique boss, K key, D keyed door, + ordinary door, ? secret wall,
# $ super-secret, T trap, L lever, G gate, R guaranteed recruit, ! lore.
cathedral=[
'###############',
'#S....#...C...#',
'#.##..#.#####.#',
'#..!..+.......#',
'###.###.###.#.#',
'#K..#...#C#.#.#',
'#.###.###?#.#.#',
'#...E...#...#.#',
'#.###.#.###.#.#',
'#...T.#...L.#.#',
'#.#.###.#####.#',
'#C#...D...B...#',
'#######.#####.#',
'#$?..G....V.>.#',
'###############']
# Correct intentional alcove width to the canonical 15-column grid.
cathedral[1]='#S....#...C...#'
data=dict(classes=classes,skills=skills,items=items,enemies=enemies,floors=[dict(name='Last Dawn Cathedral — Level 1: The Unanswered Bell',region='Last Dawn Cathedral',regionIndex=0,rank=1,rows=cathedral,enemies=['rat','spider','skeleton'],boss='cultist',key='Rusted Key',recruit=-1,recruitBark='',lore='The bell has not rung for a century. Beneath its altar, something still answers.',tint=[.38,.32,.23])])
(out/'campaign.json').write_text(json.dumps(data,indent=2),encoding='utf-8')
assert all(len(r)==15 for r in cathedral), [(i,len(r)) for i,r in enumerate(cathedral)]
print('Content written:',len(classes),'classes,',len(skills),'skills,',len(items),'items,',len(enemies),'enemy definitions')
