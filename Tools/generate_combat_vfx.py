"""Author original, palette-limited combat sprites and compile them into Slate scanline runs.

No external game artwork is used. PNGs and GIFs are review artifacts; the generated
C++ data is the exact runtime artwork and needs no texture import or cook step.
"""
from pathlib import Path
import math
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'ArtReview' / 'CombatFX'
OUT.mkdir(parents=True, exist_ok=True)
SIZE, FRAMES = 96, 8
PALETTE = [(0,0,0,0),(59,48,75,255),(107,117,139,255),(210,226,231,255),(255,255,242,255),(255,224,83,255),(255,145,43,255),(237,64,48,255),(149,35,66,255),(104,226,255,255),(54,131,221,255),(175,123,252,255),(87,48,160,255),(114,232,155,255),(38,119,107,255),(247,149,211,255)]
CUES = ['hit','critical','fireball','ice_lance','shadow_bolt','defend','dodge','power_strike','cleave','shield_wall','chain_lightning','power_shot','multi_shot','hunters_mark','heal','holy_smite','resurrection','backstab','poison_blade','grenade','holy_strike','divine_shield','lay_on_hands','curse','soul_drain','arcane_bolt','status_bind','item','mana','escape']

def frame(cue, f):
    im = Image.new('P',(SIZE,SIZE),0)
    im.putpalette([v for p in PALETTE for v in p[:3]] + [0]*(768-48))
    d = ImageDraw.Draw(im)
    t=f/7
    def line(points,c,w=1): d.line(points,fill=c,width=w)
    def disc(x,y,r,c): d.ellipse((int(x-r),int(y-r),int(x+r),int(y+r)),fill=c)
    def star(x,y,r,c):
        d.polygon([(x,y-r),(x+2,y-2),(x+r,y),(x+2,y+2),(x,y+r),(x-2,y+2),(x-r,y),(x-2,y-2)],fill=c)
    def sparks(c,number=8):
        for k in range(number):
            a=k*math.tau/number+.31; r=8+f*4
            x,y=48+math.cos(a)*r,48+math.sin(a)*r
            star(int(x),int(y),max(1,4-f//2),c)
    def slash(c1,c2,offset=0,reverse=False):
        # A tapered arc sweeps down across the target, then leaves a short glint.
        if f<6:
            x=14+f*7; y=79-f*8
            pts=[(x-10,y+9+offset),(x+13,y-26+offset),(min(88,x+35),max(7,y-36)+offset),(x+17,y-13+offset)]
            if reverse: pts=[(96-a,b) for a,b in pts]
            d.polygon(pts,fill=c1)
            pts2=[(x-5,y+3+offset),(x+16,y-24+offset),(min(88,x+35),max(7,y-36)+offset),(x+15,y-17+offset)]
            if reverse: pts2=[(96-a,b) for a,b in pts2]
            d.polygon(pts2,fill=c2)
        if f>=3: sparks(c2,6)
    if cue in ('hit','critical','power_strike','cleave','backstab','poison_blade','holy_strike'):
        edge,core=(2,4) if cue=='hit' else (8,7) if cue in ('critical','backstab') else (14,13) if cue=='poison_blade' else (6,5)
        slash(edge,core)
        if cue in ('critical','cleave'): slash(7 if cue=='critical' else 3,4,6,True)
        if cue=='hit' and 1<=f<=5: slash(5,4,-7)
        if cue=='power_strike' and f>=3: sparks(5,12)
    elif cue in ('fireball','ice_lance','shadow_bolt','arcane_bolt','soul_drain'):
        x=16+min(f,4)*8; y=67-min(f,4)*5
        edge,mid,core=(7,6,5) if cue=='fireball' else (10,9,4) if cue=='ice_lance' else (12,11,15)
        if f<5:
            for k in range(6):
                xx=x-k*5; yy=y+k*3+((k+f)%3-1)*2
                disc(xx,yy,max(1,9-k),edge if k%2 else mid)
            if cue=='ice_lance':
                d.polygon([(x-24,y+18),(x-3,y-13),(x+22,y-22),(x+6,y+7)],fill=edge)
                d.polygon([(x-17,y+13),(x+22,y-22),(x-1,y+2)],fill=core)
                line([(x-1,y+2),(x+22,y-22)],mid,3)
            else:
                disc(x,y,12,edge);disc(x+2,y-2,9,mid);disc(x+3,y-3,5,core)
                if cue=='shadow_bolt':disc(x+2,y-2,4,1);star(x+6,y-6,4,15)
        else:
            if cue=='ice_lance':
                for k in range(8):
                    a=k*math.tau/8; r=(f-3)*8; xx=48+math.cos(a)*r; yy=47+math.sin(a)*r
                    d.polygon([(xx-3,yy+5),(xx+5,yy-9),(xx+2,yy+5)],fill=9 if k%2 else 4)
            else:
                r=10+(f-5)*8;d.ellipse((48-r,47-r,48+r,47+r),outline=mid,width=max(1,5-(f-5)*2));sparks(core)
    elif cue in ('defend','shield_wall','divine_shield'):
        c=9 if cue!='divine_shield' else 5; w=3 if f<6 else 1
        top=20+abs(3-f)*2
        d.polygon([(27,top),(48,top-5),(69,top),(66,56),(48,76),(30,56)],fill=1)
        line([(27,top),(48,top-5),(69,top),(66,56),(48,76),(30,56),(27,top)],c,w)
        line([(48,top+5),(48,62)],4,3);line([(35,39),(61,39)],4,3)
        if 2<=f<=5:star(68,top,7,4)
    elif cue in ('dodge','escape'):
        for k in range(3):
            x=22+f*6-k*12
            line([(x-10,28+k*9),(x+7,28+k*9),(x-2,36+k*9)],3 if k==0 else 2,2)
            line([(x-15,53+k*6),(x,53+k*6)],4 if k==0 else 2,2)
    elif cue=='chain_lightning':
        pts=[(48,8),(36+(f%2)*10,29),(58,25),(37,53),(62,46),(47,81)]
        line(pts,10,8);line(pts,9,4);line(pts,4,2)
        if f>=3:sparks(9)
    elif cue in ('power_shot','multi_shot'):
        for k in range(3 if cue=='multi_shot' else 1):
            y=36+k*14 if cue=='multi_shot' else 48; x=12+min(f,5)*11
            line([(max(2,x-29),y+10),(x,y)],5,2)
            d.polygon([(x+7,y-3),(x-4,y+9),(x-3,y+2),(x-10,y)],fill=4)
        if f>=5:sparks(5)
    elif cue in ('hunters_mark','curse','status_bind'):
        c=7 if cue=='hunters_mark' else 11
        r=22+abs(3-f)*2
        d.ellipse((48-r,48-r,48+r,48+r),outline=c,width=2)
        for k in range(4):
            a=k*math.pi/2+(.0 if cue=='hunters_mark' else f*.12)
            x,y=48+math.cos(a)*r,48+math.sin(a)*r;star(int(x),int(y),6,4 if cue=='hunters_mark' else 15)
        if cue=='hunters_mark':line([(40,48),(56,48)],7,2);line([(48,40),(48,56)],7,2)
        else:line([(30,30+f*2),(66,66-f*2)],12,3);line([(66,30+f*2),(30,66-f*2)],11,3)
    elif cue=='grenade':
        if f<3:
            x=25+f*10;y=67-f*9;disc(x,y,8,2);disc(x-2,y-3,3,3);star(x+6,y-10,4,5)
        else:
            r=12+(f-3)*5
            for k in range(7):
                a=k*math.tau/7;disc(48+math.cos(a)*r*.7,48+math.sin(a)*r*.7,r*.45,7 if f<6 else 2)
            if f<6:disc(48,48,r*.65,6);star(48,48,r*.7,5)
            sparks(5)
    else:
        c=9 if cue=='mana' else 5 if cue in ('holy_smite','resurrection','lay_on_hands') else 13
        if cue=='holy_smite':
            line([(48,8),(48,79)],5,13 if f<5 else 3);line([(48,8),(48,79)],4,5)
        elif cue=='resurrection':
            line([(48,68),(48,26),(36,39),(48,26),(60,39)],4,4)
        else:
            y=59-f*3;d.rectangle((44,y-12,51,y+12),fill=c);d.rectangle((36,y-4,59,y+4),fill=c)
            d.rectangle((47,y-9,49,y+9),fill=4)
        for k in range(6):
            x=24+k*10;y=72-((f*7+k*11)%52);star(x,y,3,c)
        if f>=4:sparks(c,6)
    return im

all_frames={cue:[frame(cue,f) for f in range(FRAMES)] for cue in CUES}
review=Image.new('RGB',(112+FRAMES*144,len(CUES)*160),(21,23,32));rd=ImageDraw.Draw(review)
records=[];runs=[]
for row,(cue,frames) in enumerate(all_frames.items()):
    sheet=Image.new('RGBA',(SIZE*FRAMES,SIZE))
    gif=[]
    rd.text((8,row*160+64),cue,fill='white')
    for f,im in enumerate(frames):
        rgba=im.convert('RGBA');rgba.putalpha(im.point(lambda p:255 if p else 0, 'L'))
        sheet.paste(rgba,(f*SIZE,0));scaled=rgba.resize((144,144),Image.Resampling.NEAREST)
        review.paste(scaled,(112+f*144,row*160+8),scaled)
        start=len(runs)
        for y in range(SIZE):
            x=0
            while x<SIZE:
                c=im.getpixel((x,y));end=x+1
                while end<SIZE and im.getpixel((end,y))==c:end+=1
                if c:runs.append((x,y,end-x,c))
                x=end
        records.append((start,len(runs)-start))
        bg=Image.new('RGBA',(288,288),(21,23,32,255));large=rgba.resize((288,288),Image.Resampling.NEAREST);bg.alpha_composite(large);gif.append(bg.convert('RGB'))
    sheet.save(OUT/f'{cue}.png')
    gif[0].save(OUT/f'{cue}.gif',save_all=True,append_images=gif[1:],duration=100,loop=0)
review.save(OUT/'contact-sheet.png')
review.crop((0,0,1264,7*160)).save(OUT/'core-effects.png')
data=['// Generated by Tools/generate_combat_vfx.py. Original pixel artwork.','static const FColor SpritePalette[] = {']
data += [' FColor(%d,%d,%d,%d),' % p for p in PALETTE];data+=['};','static const FPixelRun SpriteRuns[] = {']
data += [' {%d,%d,%d,%d},'%r for r in runs];data+=['};','static const FSpriteFrame SpriteFrames[] = {']
data += [' {%d,%d},'%r for r in records];data+=['};','static const TCHAR* SpriteCues[] = {']
data += [' TEXT("'+c+'"),' for c in CUES];data+=['};']
(ROOT/'Source/DungeonCrawler/DungeonCombatVFXData.inl').write_text('\n'.join(data)+'\n')
print(f'{len(CUES)} cues, {len(records)} frames, {len(runs)} scanline runs; review: {OUT}')
