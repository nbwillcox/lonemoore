"""Blender 5.2: 24 grounded regional props, reusable UVs and opaque materials.

Run Blender --background --threads 4 --python this_file.
Run the bundled Pillow Python with --contact-only to assemble the preview sheet.
Original artwork is referenced read-only. Output is isolated in ArtSource/RegionalDressing.
"""
from pathlib import Path
import json, math, random, sys, hashlib

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'ArtSource/RegionalDressing'
ROLES=['Wall','Trim','Iron','Bone','Accent']
REGIONS=['Cathedral','Sewer','Catacombs','Warrens','Crypts','Fortress','Deep','Infernal','Hell']

if '--contact-only' in sys.argv:
    from PIL import Image,ImageDraw,ImageFont,ImageOps
    data=json.loads((OUT/'manifest.json').read_text());font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',21)
    board=Image.new('RGB',(2000,6*430),(17,21,24));draw=ImageDraw.Draw(board)
    for i,prop in enumerate(data['props']):
        p=OUT/'Renders'/(prop['id']+'.png');x=i%4*500;y=i//4*430
        with Image.open(p) as source:im=ImageOps.contain(source.convert('RGB'),(496,380),Image.Resampling.LANCZOS)
        board.paste(im,(x+(500-im.width)//2,y));draw.text((x+10,y+384),prop['name'],font=font,fill=(231,218,190))
        draw.text((x+10,y+410),prop['region']+' / Blender asset preview',font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14),fill=(163,180,175))
    board.save(OUT/'CONTACT_SHEET.jpg',quality=93);print(OUT/'CONTACT_SHEET.jpg');sys.exit(0)

import bpy
from mathutils import Vector,Matrix
for sub in ('Meshes','Scenes','Renders'):(OUT/sub).mkdir(parents=True,exist_ok=True)
V=[];F=[];M=[];UV=[];REGION='Cathedral'
def face(points,role='Trim',uv=None):
    points=[Vector(p) for p in points]
    area=sum((points[i]-points[0]).cross(points[i+1]-points[0]).length for i in range(1,len(points)-1))
    if area<1e-9:return
    start=len(V);V.extend(tuple(p) for p in points);F.append(tuple(range(start,start+len(points))));M.append(ROLES.index(role))
    if uv is None:
        normal=(points[1]-points[0]).cross(points[2]-points[0]);axis=max(range(3),key=lambda j:abs(normal[j]));axes=[j for j in range(3) if j!=axis]
        uv=[(p[axes[0]]/1.2,p[axes[1]]/1.2) for p in points]
    UV.append(uv)

def box(center,size,role='Trim',rotate=0):
    rot=Matrix.Rotation(rotate,3,'Z');c=Vector(center)
    p=[c+rot@Vector((sx*size[0]/2,sy*size[1]/2,sz*size[2]/2)) for sx,sy,sz in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
    for ids in ((0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)):face([p[i] for i in ids],role)

def tube(points,radius,role='Bone',sides=10,hollow=0,caps=True):
    points=[Vector(p) for p in points];radii=radius if isinstance(radius,(list,tuple)) else [radius]*len(points)
    rings=[];inners=[]
    for i,p in enumerate(points):
        tangent=(points[min(i+1,len(points)-1)]-points[max(i-1,0)]).normalized();a=tangent.cross(Vector((0,0,1)))
        if a.length<.1:a=tangent.cross(Vector((0,1,0)))
        a.normalize();b=tangent.cross(a).normalized()
        rings.append([p+(a*math.cos(j*2*math.pi/sides)+b*math.sin(j*2*math.pi/sides))*radii[i] for j in range(sides)])
        if hollow:inners.append([p+(a*math.cos(j*2*math.pi/sides)+b*math.sin(j*2*math.pi/sides))*max(.001,radii[i]-hollow) for j in range(sides)])
    for i in range(len(points)-1):
        for j in range(sides):
            k=(j+1)%sides;face([rings[i][j],rings[i][k],rings[i+1][k],rings[i+1][j]],role)
            if hollow:face([inners[i][k],inners[i][j],inners[i+1][j],inners[i+1][k]],role)
    if caps:
        for i,reverse in ((0,True),(-1,False)):
            if hollow:
                for j in range(sides):
                    k=(j+1)%sides;coords=[rings[i][j],inners[i][j],inners[i][k],rings[i][k]]
                    face(coords if reverse else list(reversed(coords)),role)
            else:face(list(reversed(rings[i])) if reverse else rings[i],role)

def rod(a,b,r=.04,role='Iron',sides=10):tube([a,b],r,role,sides)
def cone(a,b,r=.08,role='Bone',sides=10):tube([a,b],[r,.002],role,sides)
def ellipsoid(center,scale,role='Bone',segments=16,rings=10,cut=None,phase=0):
    c=Vector(center)
    pts=[]
    for j in range(rings+1):
        latitude=-math.pi/2+j*math.pi/rings
        pts.append([c+Vector((math.cos(i*2*math.pi/segments+phase)*math.cos(latitude)*scale[0],math.sin(i*2*math.pi/segments+phase)*math.cos(latitude)*scale[1],math.sin(latitude)*scale[2])) for i in range(segments)])
    for j in range(rings):
        for i in range(segments):
            k=(i+1)%segments;corners=[pts[j][i],pts[j][k],pts[j+1][k],pts[j+1][i]]
            mid=sum(corners,Vector())/4
            if cut and cut(mid):continue
            face(corners,role)

def ring(center,radius,thick=.035,role='Iron',normal=(0,0,1),steps=24):
    q=Vector(normal).to_track_quat('Z','Y');c=Vector(center)
    path=[c+q@Vector((radius*math.cos(i*2*math.pi/steps),radius*math.sin(i*2*math.pi/steps),0)) for i in range(steps+1)]
    tube(path,thick,role,8,caps=False)

def lathe(center,profile,role='Trim',steps=24):
    c=Vector(center)
    for j in range(len(profile)-1):
        z,r=profile[j];zz,rr=profile[j+1]
        for i in range(steps):
            a=i*2*math.pi/steps;b=(i+1)*2*math.pi/steps
            face([c+Vector((r*math.cos(a),r*math.sin(a),z)),c+Vector((r*math.cos(b),r*math.sin(b),z)),c+Vector((rr*math.cos(b),rr*math.sin(b),zz)),c+Vector((rr*math.cos(a),rr*math.sin(a),zz))],role)

def patch(center,radii,height=.07,role='Accent',seed=1):
    rand=random.Random(seed);c=Vector(center);edge=[]
    for i in range(28):
        a=i*2*math.pi/28;r=rand.uniform(.86,1.13);edge.append(c+Vector((math.cos(a)*r*radii[0],math.sin(a)*r*radii[1],.012)))
    inner=[c+(p-c)*.62+Vector((0,0,height)) for p in edge]
    for i in range(28):
        j=(i+1)%28;face([edge[i],edge[j],inner[j],inner[i]],role);face([inner[i],inner[j],c+Vector((0,0,height*1.1))],role)

def rock(center,scale,seed=1,role='Wall'):
    rand=random.Random(seed);base=len(V);ellipsoid(center,scale,role,segments=9,rings=5,phase=seed*.29)
    c=Vector(center)
    # Shared geometric points use the same displacement, avoiding cracks between faces.
    displacements={}
    for i in range(base,len(V)):
        key=tuple(round(v,6) for v in V[i]);f=displacements.setdefault(key,rand.uniform(.87,1.1));V[i]=tuple(c+(Vector(V[i])-c)*f)

def bone(a,b,r=.045):
    rod(a,b,r,'Bone',8);v=(Vector(b)-Vector(a)).normalized();side=v.cross(Vector((0,0,1)))
    if side.length<.1:side=v.cross(Vector((0,1,0)))
    side.normalize()
    for p in (Vector(a),Vector(b)):
        for s in (-1,1):ellipsoid(p+side*r*.65,(r*1.25,r*1.05,r*1.20),'Bone',10,6)

def skull(center=(0,0,.28),scale=1):
    c=Vector(center)
    def p(x,y,z):return c+Vector((x,y,z))*scale
    ellipsoid(p(0,.025,.085),(.20*scale,.18*scale,.245*scale),'Bone',32,22,cut=lambda v:v.y<c.y-.08*scale and ((abs(v.x-c.x)/scale-.095)/.073)**2+(((v.z-c.z)/scale-.106)/.075)**2<1.18)
    # Recessed asymmetric orbital openings, not decorative circular toruses.
    for side in (-1,1):
        outline=[(.025,.139),(.062,.170),(.144,.163),(.179,.114),(.150,.061),(.077,.045),(.034,.075)]
        rim=[p(side*x,-.145-(.009 if z>.14 else 0),z) for x,z in outline]
        tube(rim+[rim[0]],[.020*scale,.026*scale,.031*scale,.019*scale,.017*scale,.016*scale,.020*scale,.020*scale],'Bone',7,caps=False)
        face([p(side*x,-.071,z) for x,z in reversed(outline)],'Iron')
        tube([p(side*.17,-.10,.077),p(side*.177,-.108,-.005),p(side*.119,-.159,-.04)],[.026*scale,.032*scale,.024*scale],'Bone',7)
    face([p(-.031,-.145,.034),p(.029,-.145,.032),p(.016,-.163,-.026),p(-.020,-.163,-.029)],'Iron')
    for side in (-1,1):rod(p(side*.018,-.14,.048),p(side*.034,-.171,-.037),.012*scale,'Bone',6)
    tube([p(-.15,-.07,-.085),p(-.105,-.18,-.12),p(0,-.207,-.125),p(.105,-.18,-.12),p(.15,-.07,-.085)],.038*scale,'Bone',8)
    for x in range(-3,4):box(p(x*.033,-.184,-.041),(.027*scale,.05*scale,.073*scale),'Bone')

def candle(x,y,z,height=.42,radius=.055):
    lathe((x,y,z),[(0,radius*1.25),(.035,radius*1.17),(height-.05,radius),(height,radius*.85),(height-.02,radius*.35)],'Bone',14)
    rod((x,y,z+height-.02),(x,y,z+height+.05),.009,'Iron',6)
    tube([(x,y,z+height+.02),(x+.015,y,z+height+.13),(x-.02,y,z+height+.27)],[radius*.5,radius*.7,.002],'Accent',8)
    for angle in (.3,2.5,4.5):
        a=radius*math.cos(angle);b=radius*math.sin(angle);tube([(x+a,y+b,z+height*.88),(x+a,y+b,z+height*.45)],[.018,.028],'Bone',6)

def flame(center,height=.7,r=.15):
    c=Vector(center)
    for i in range(4):
        a=i*2*math.pi/4;offset=Vector((math.cos(a)*r*.7,math.sin(a)*r*.7,0));tube([c+offset,c+offset*.5+Vector((r*.2,0,height*.35)),c-offset*.7+Vector((-.04,.06,height*.72)),c+Vector((.05,-.02,height))],[r*.50,r*.7,r*.32,.002],'Accent',8)

def chain(a,b,links=7,r=.08):
    a=Vector(a);b=Vector(b)
    for i in range(links):
        c=a.lerp(b,i/max(1,links-1));ring(c,r,.022,'Iron',normal=(1,0,0) if i%2 else (0,1,0),steps=16)

def drake(center=(0,0,0),scale=1,charred=False):
    c=Vector(center)
    def p(x,y,z):return c+Vector((x,y,z))*scale
    ellipsoid(p(0,.27,.81),(.54*scale,.55*scale,.48*scale),'Bone',22,14,
        cut=lambda v:abs(v.x-c.x)>.30*scale and c.y-.13*scale<v.y<c.y+.28*scale and c.z+.74*scale<v.z<c.z+1.10*scale)
    # Wide temporal openings, pronounced brow and a long open reptilian muzzle.
    for side in (-1,1):
        socket=[p(side*.455,.18,1.07),p(side*.48,-.035,1.11),p(side*.45,-.205,.94),p(side*.41,-.11,.77),p(side*.43,.16,.78),p(side*.455,.18,1.07)]
        tube(socket,[.049*scale,.075*scale,.047*scale,.04*scale,.054*scale,.049*scale],'Bone',8,caps=False)
        tube([p(side*.47,.30,1.10),p(side*.54,-.04,1.20),p(side*.33,-.53,.98)],[.085*scale,.092*scale,.055*scale],'Bone',9)
        tube([p(side*.45,.25,.58),p(side*.36,-.48,.64),p(side*.23,-1.14,.50)],[.09*scale,.076*scale,.073*scale],'Bone',9)
        tube([p(side*.53,.33,.45),p(side*.46,-.24,.23),p(side*.34,-.74,.14),p(side*.20,-1.24,.22)],[.10*scale,.076*scale,.070*scale,.071*scale],'Bone',9)
        # Swept horn silhouette rather than an unmodified spherical skull.
        tube([p(side*.40,.47,1.02),p(side*.66,.68,1.32),p(side*.80,.86,1.64),p(side*.84,1.03,1.86)],[.17*scale,.12*scale,.06*scale,.002],'Bone',11)
        cone(p(side*.49,.35,.74),p(side*.79,.70,.55),.11*scale,'Bone')
        for i in range(7):
            yy=-.28-i*.127;xx=side*(.39-i*.020);zz=.63-i*.017
            cone(p(xx,yy,zz),p(xx*.96,yy-.01,zz-(.21 if i<3 else .14)),(.042 if i<3 else .028)*scale,'Bone',8)
            if i<6:cone(p(xx*1.06,yy-.03,.20),p(xx,yy-.03,.32),.028*scale,'Bone',8)
    rod(p(-.21,-1.15,.50),p(.21,-1.15,.50),.071*scale,'Bone',10)
    rod(p(-.20,-1.24,.22),p(.20,-1.24,.22),.06*scale,'Bone',10)
    # Nasal bridge and two real apertures on the snout.
    tube([p(0,.30,1.17),p(0,-.25,1.04),p(0,-.86,.70)],[.25*scale,.19*scale,.11*scale],'Bone',12)
    for side in (-1,1):
        nasal=[p(side*.047,-1.071,.622),p(side*.128,-1.058,.648),p(side*.175,-1.081,.571),p(side*.061,-1.103,.542),p(side*.047,-1.071,.622)]
        tube(nasal,.018*scale,'Bone',7,caps=False)
    for y,z in ((.35,1.25),(.66,1.15)):cone(p(0,y,z),p(0,y+.1,z+.20),.085*scale,'Bone')

def cathedral_votive():
    lathe((0,0,0),[(0,.40),(.1,.43),(.2,.31),(.30,.16),(1.45,.10),(1.56,.17)],'Trim')
    for z in (.23,1.38):ring((0,0,z),.18,.035,'Iron')
    for i in range(6):
        a=i*math.pi/3;x=.50*math.cos(a);y=.50*math.sin(a)
        tube([(0,0,1.15),(x*.7,y*.7,1.02),(x,y,1.33)],.035,'Iron',8)
        lathe((x,y,1.28),[(0,.10),(.05,.13),(.08,.12)],'Iron',16);candle(x,y,1.36,.34+(i%2)*.10)
    candle(0,0,1.57,.56,.075)

def cathedral_bell():
    lathe((0,0,.10),[(0,.68),(.10,.75),(.18,.69),(.27,.56),(.55,.43),(.80,.29),(.96,.15),(1.0,.12),(1.0,.07),(.90,.095),(.73,.20),(.43,.33),(.10,.60),(0,.62)],'Iron',32)
    for z,r in ((.15,.74),(.31,.58),(.95,.16)):ring((0,0,z),r,.027,'Trim')
    ring((0,0,1.19),.14,.045,'Iron',normal=(0,1,0))
    rod((0,0,.96),(0,-.09,.31),.04,'Iron');ellipsoid((0,-.09,.29),(.10,.10,.14),'Iron')
    for x,y in ((-.63,.4),(.6,.33)):rock((x,y,.1),(.28,.25,.15),4,'Wall')

def cathedral_lectern():
    box((0,0,.08),(.85,.75,.16));box((0,0,.24),(.67,.58,.18));lathe((0,0,.29),[(0,.25),(.12,.18),(.86,.12),(.98,.30)],'Trim',8)
    # Pitched book cradle, thick covers and two engraved page blocks.
    for side in (-1,1):
        pts=[(0,-.36,1.31),(side*.46,-.36,1.34),(side*.46,.34,1.43),(0,.34,1.40)]
        if side<0:pts.reverse()
        face(list(reversed(pts)),'Iron')
        top=[(x,y,z+.022) for x,y,z in pts];face(top,'Bone')
        for i in range(4):
            j=(i+1)%4;face([pts[i],pts[j],top[j],top[i]],'Bone')
        for line in range(7):
            yy=-.26+line*.077;zz=1.336+(yy+.36)*(.09/.70)
            rod((side*.08,yy,zz+.08/.46*.03),(side*.37,yy,zz+.37/.46*.03),.003,'Iron',4)
    rod((0,-.39,1.333),(0,.36,1.424),.012,'Trim')

def sewer_pipe():
    path=[(0,.44,.20),(0,.44,.70),(0,.38,1.0),(0,.17,1.18),(0,-.17,1.19),(0,-.49,1.19)]
    tube(path,.35,'Iron',24,hollow=.068)
    for y in (-.43,-.20):ring((0,y,1.19),.40,.045,'Iron',normal=(0,1,0))
    for i in range(8):
        a=i*math.pi/4;rod((math.cos(a)*.41,-.50,1.19+math.sin(a)*.41),(math.cos(a)*.41,-.35,1.19+math.sin(a)*.41),.032,'Trim',6)
    tube([(0,-.46,.99),(0,-.63,.87),(.06,-.72,.60),(.09,-.77,.12)],[.16,.14,.11,.23],'Accent',14)
    patch((.05,-.56,.025),(.91,.94),.055,'Accent',12)
    for x,y in ((-.52,-.53),(.48,-.80),(.26,-1.1)):ellipsoid((x,y,.11),(.12,.10,.055),'Accent',12,6)
    box((0,.46,.05),(.94,.72,.10),'Wall')

def sewer_drain():
    lathe((0,0,.02),[(0,.82),(.11,.82),(.16,.73),(.16,.60),(.03,.60)],'Wall',32)
    ring((0,0,.135),.66,.035,'Iron')
    for x in (-.48,-.32,-.16,0,.16,.32,.48):
        length=math.sqrt(.6*.6-x*x);box((x,0,.095),(.06,length*2,.045),'Iron')
    for y in (-.24,.24):box((0,y,.112),(1.1,.045,.035),'Iron')
    for x,y in ((-.75,.35),(.47,-.72),(.76,.1)):patch((x,y,.025),(.38,.23),.035,'Accent',int((x+2)*8))
    bone((.33,.40,.20),(.62,.12,.15),.026)

def sewer_sludge():
    patch((0,0,.03),(1.15,.67),.12,'Accent',19)
    for i in range(7):
        a=i*2.4;x=math.sin(a)*.82;y=math.cos(a)*.41
        ellipsoid((x,y,.11),(.13+.025*(i%3),.11,.055),'Accent',12,6)
    tube([(-.65,.08,.14),(-.54,.02,.34),(-.33,.01,.49)],.11,'Iron',16,hollow=.025)
    box((.27,.06,.18),(.39,.40,.10),'Trim',.31);bone((-.20,-.39,.15),(.31,-.45,.13),.038)
    for x,y in ((.6,.25),(-.7,-.1),(.12,.46)):
        tube([(x,y,.10),(x,y,.29)],.024,'Bone',7);ellipsoid((x,y,.30),(.12,.09,.07),'Accent',14,7)

def catacombs_pile():
    rand=random.Random(808)
    for i in range(14):
        x=rand.uniform(-.58,.58);y=rand.uniform(-.37,.37);z=.07+rand.random()*.13;a=rand.random()*math.pi
        bone((x-.20*math.cos(a),y-.20*math.sin(a),z),(x+.20*math.cos(a),y+.20*math.sin(a),z+.04),.034)
    for x,y,z,s in ((-.31,.10,.31,1),(.34,-.07,.29,.9),(.10,.30,.43,1.05)):skull((x,y,z),s)

def catacombs_ossuary():
    box((0,0,.10),(1.65,.75,.20),'Wall')
    for x in (-.76,.76):box((x,0,.75),(.16,.71,1.25),'Wall')
    for z in (.24,.75,1.29):box((0,0,z),(1.63,.79,.11),'Trim')
    box((0,.30,.77),(1.62,.09,1.20),'Wall')
    for z in (.5,1.03):
        for x in (-.48,0,.48):skull((x,-.05,z),.78)
    for x in (-.50,.36):bone((x-.2,-.27,.35),(x+.2,.14,.33),.031)

def warrens_barricade():
    for i in range(7):
        x=-.84+i*.28;h=1.04+(i%3)*.18
        box((x,0,h/2),(.19,.12,h),'Trim',(i%3-1)*.07);cone((x,0,h),(x+.04,0,h+.26),.12,'Trim',4)
    for z in (.35,.79):rod((-.98,-.11,z),(.98,-.11,z+.10),.058,'Trim',8)
    for x in (-.61,.61):
        rod((x,.0,.73),(x,.5,.06),.07,'Trim',8);ring((x,-.08,.50),.10,.019,'Iron',normal=(1,0,0))
    for x in (-.36,.36):skull((x,-.12,1.11),.6)

def warrens_cache():
    lathe((-.37,0,.03),[(0,.32),(.08,.36),(.48,.40),(.82,.33),(.88,.32),(.88,.26),(.80,.27)],'Trim',14)
    for z in (.14,.64,.83):ring((-.37,0,z),.34 if z>.8 else .37,.027,'Iron')
    for i in range(3):
        x=.20+i*.20;y=.08+(i%2)*.18;rod((x,y,.09),(x-.15,y,1.46+i*.11),.027,'Trim',8);cone((x-.15,y,1.42+i*.11),(x-.15,y,1.68+i*.11),.079,'Iron',4)
    rock((.43,-.23,.27),(.33,.33,.28),19,'Accent');box((.22,.15,.13),(.69,.55,.25),'Trim',.2)
    for x in (.04,.37,.63):ring((x,-.19,.20),.055,.016,'Iron',normal=(0,1,0))

def warrens_fungus():
    for i in range(8):
        a=i*2.399;x=.52*math.sin(a);y=.38*math.cos(a);h=.28+(i%3)*.24;r=.20+(i%3)*.08
        tube([(x,y,.02),(x+.06,y,h*.6),(x-.025,y,h)],[.07,.053,.055],'Bone',9)
        ellipsoid((x-.025,y,h),(r,r*.87,r*.38),'Accent',18,8,cut=lambda p:p.z<h-.03)
        lathe((x-.025,y,h-.025),[(0,r*.8),(.03,.05)],'Bone',18)
    for i in range(4):tube([(-.7+i*.35,-.48,.02),(-.48+i*.28,.10,.07),(-.61+i*.36,.52,.02)],[.035,.05,.028],'Trim',7)

def crypt_sarcophagus():
    # Tapered coffin silhouette and raised ceremonial lid.
    outline=[(-.48,-1.0),(.48,-1.0),(.67,-.58),(.54,.91),(-.54,.91),(-.67,-.58)]
    for i in range(6):
        a=outline[i];b=outline[(i+1)%6];face([(a[0],a[1],.13),(b[0],b[1],.13),(b[0],b[1],.76),(a[0],a[1],.76)],'Wall')
    face([(x,y,.80) for x,y in outline],'Trim');face([(x*.88,y*.92,.88) for x,y in outline],'Wall')
    for x in (-.47,.47):box((x,0,.095),(.24,1.66,.19),'Trim')
    box((0,-.12,.92),(.12,1.12,.08),'Iron');box((0,-.30,.927),(.56,.13,.09),'Iron')
    for x,y in ((-.55,-.65),(.55,-.65),(-.50,.55),(.50,.55)):ring((x,y,.53),.105,.026,'Iron',normal=(1,0,0))
    skull((0,.59,1.02),.63)

def crypt_vigil():
    lathe((0,0,0),[(0,.34),(.12,.37),(.22,.19),(.30,.10),(1.22,.08)],'Iron',12)
    for x,y in ((-.22,-.22),(.22,-.22),(.22,.22),(-.22,.22)):rod((x,y,1.14),(x,y,1.76),.023,'Iron',6)
    box((0,0,1.11),(.60,.60,.09),'Trim');box((0,0,1.76),(.55,.55,.07),'Iron')
    for x,y in ((-.27,-.27),(.27,-.27),(.27,.27),(-.27,.27)):rod((x,y,1.78),(0,0,2.18),.04,'Iron',6)
    candle(0,0,1.17,.44,.10);ring((0,0,2.30),.11,.022,'Iron',normal=(0,1,0))
    for side in (-1,1):tube([(side*.11,0,1.56),(side*.35,.04,1.39),(side*.39,.0,.92)],.022,'Iron',8)

def crypt_standard():
    lathe((0,0,0),[(0,.31),(.12,.33),(.21,.21),(.26,.12)],'Trim',12);rod((0,0,.20),(0,0,2.54),.037,'Iron',12)
    cone((0,0,2.48),(0,0,2.91),.075,'Trim',6);rod((-.67,0,2.48),(.67,0,2.48),.031,'Iron')
    for i in range(10):
        for j in range(8):
            def point(a,b):return (-.54+a*.108,-.025+.055*math.sin(a*.8+b*.35),2.43-b*.165-(.2 if b==8 and a%4 in (0,1) else 0))
            face([point(i,j),point(i+1,j),point(i+1,j+1),point(i,j+1)],'Accent')
            face([point(i,j+1),point(i+1,j+1),point(i+1,j),point(i,j)],'Accent')
    rod((0,-.09,1.60),(0,-.09,2.18),.02,'Iron',6);rod((-.20,-.09,2.00),(.20,-.09,2.00),.02,'Iron',6)

def fortress_armory():
    for x in (-.75,.75):box((x,.30,.72),(.14,.18,1.44),'Trim')
    for z in (.20,1.13):box((0,.30,z),(1.75,.18,.12),'Trim')
    for x in (-.52,.47):
        pts=[(x-.28,-.035,1.14),(x+.28,-.035,1.14),(x+.29,-.045,.76),(x,-.07,.36),(x-.29,-.045,.76)]
        face(list(reversed(pts)),'Iron');face([(a,b+.065,c) for a,b,c in pts],'Iron')
        for i in range(5):rod(pts[i],pts[(i+1)%5],.024,'Trim',6)
        box((x,-.08,.89),(.085,.04,.51),'Accent');box((x,-.084,.94),(.36,.043,.085),'Accent')
    for x in (-.29,.08,.83):
        rod((x,.24,.04),(x,.24,1.93),.028,'Trim',8);cone((x,.24,1.86),(x,.24,2.30),.087,'Iron',4)

def fortress_shackles():
    box((0,.09,.38),(1.13,.30,.76),'Wall');box((0,-.10,.40),(.96,.08,.56),'Iron')
    for x in (-.36,.36):
        ring((x,-.16,.53),.091,.028,'Iron',normal=(0,1,0));chain((x,-.22,.53),(x,-.42,.10),5,.066)
        ring((x,-.49,.10),.12,.032,'Iron',normal=(0,0,1))
    for x in (-.4,.4):
        for z in (.20,.63):rod((x,-.13,z),(x,-.19,z),.025,'Iron',6)
    ellipsoid((.05,-.39,.13),(.21,.20,.23),'Iron',18,10,cut=lambda p:p.z<.12)

def deep_skull():
    drake((0,0,0),1)
    for p,s in (((-.68,.10,.08),(.28,.28,.10)),((.58,.22,.07),(.21,.23,.09))):rock(p,s,9,'Wall')

def deep_ribcage():
    for i in range(7):
        y=-.95+i*.31;h=1.36-.27*abs(i-3)/3
        ellipsoid((0,y,h),(.12,.12,.14),'Bone',12,7)
        for side in (-1,1):tube([(side*.05,y,h),(side*.52,y-.03,h-.07),(side*.88,y-.06,h-.45),(side*.90,y-.07,.26)],[.085,.080,.065,.035],'Bone',9)
        cone((0,y,h+.09),(0,y+.045,h+.37),.074,'Bone',8)
    tube([(0,-1.1,1.08),(0,0,1.4),(0,1.1,1.08)],.079,'Bone',10)
    for side in (-1,1):bone((side*.45,-.97,.06),(side*.91,-.45,.13),.047)

def deep_crystals():
    for i,(x,y,h,r) in enumerate([(-.39,.1,.78,.20),(.12,.08,1.52,.24),(.49,-.12,.85,.18),(-.14,-.40,.52,.16),(.39,.42,.65,.17)]):
        rock((x,y,.10),(r*1.5,r*1.3,.16),i+2,'Wall')
        tube([(x,y,.07),(x-.08,y+.08,h*.77),(x-.13,y+.12,h)],[r,r*.86,.001],'Accent',6)

def infernal_seal():
    box((0,0,.10),(1.11,.64,.20),'Wall')
    tube([(0,0,.17),(0,.0,1.22),(.04,.0,1.55)],[.39,.34,.24],'Wall',5)
    for z,r in ((.53,.21),(.99,.19)):
        ring((0,-.335,z),r,.026,'Accent',normal=(0,1,0),steps=24)
        for i in range(5):
            a=i*2*math.pi/5;b=a+4*math.pi/5;rod((r*math.cos(a),-.367,z+r*math.sin(a)),(r*math.cos(b),-.367,z+r*math.sin(b)),.010,'Accent',5)
    rock((.57,.19,.19),(.29,.24,.22),21,'Wall');chain((-.38,0,1.13),(.45,-.14,.25),9,.082)

def infernal_brazier():
    lathe((0,0,0),[(0,.44),(.10,.46),(.19,.30),(.66,.14),(.74,.25),(.79,.44),(.98,.58),(1.06,.61),(1.06,.52),(.85,.30)],'Iron',24)
    for i in range(8):
        a=i*math.pi/4;cone((.56*math.cos(a),.56*math.sin(a),1.0),(.62*math.cos(a),.62*math.sin(a),1.38),.065,'Iron',6)
    lathe((0,0,.87),[(0,.35),(.03,.45)],'Accent',24);flame((0,0,.90),.79,.24)
    for x in (-.28,.28):skull((x,-.28,.47),.48)

def hell_basin():
    # Molten surface sits above its own solid bowl; it never shares the dungeon floor.
    lathe((0,0,.02),[(0,1.0),(.15,1.05),(.31,.95),(.39,.81),(.36,.65),(.10,.57)],'Wall',15)
    patch((0,0,.25),(.79,.74),.065,'Accent',73)
    for i in range(9):
        a=i*2*math.pi/9;rock((.84*math.cos(a),.84*math.sin(a),.30),(.20,.18,.18),31+i,'Wall')
    for x,y in ((.24,.1),(-.32,-.19),(.2,-.34)):ellipsoid((x,y,.32),(.10,.12,.055),'Accent',12,6)
    flame((-.32,.33,.35),.55,.13)

def hell_fissure():
    patch((0,0,.05),(1.17,.48),.045,'Accent',14)
    for side in (-1,1):
        for i in range(5):
            x=-.93+i*.45;rock((x,side*(.29+.035*(i%2)),.13),(.33,.23,.20),80+i+int(side),'Wall')
    for x in (-.74,.34,.86):flame((x,.02,.10),.40+.10*abs(x),.075)

def hell_trophy():
    box((0,.13,.09),(.84,.86,.18),'Wall')
    tube([(0,.20,.16),(0,.20,1.36)],[.17,.12],'Iron',8)
    drake((0,-.08,.52),.71,True)
    for side in (-1,1):
        cone((side*.27,.20,.53),(side*.62,.26,1.41),.083,'Iron',6);chain((side*.23,.14,.99),(side*.38,-.33,.28),7,.065)
    patch((0,0,.03),(.69,.68),.035,'Accent',21)

PROPS=[
 ('cathedral_votive','Sixfold Votive Candelabrum','Cathedral',cathedral_votive,'wall',130),
 ('cathedral_bell','The Unanswered Bell','Cathedral',cathedral_bell,'corner',170),
 ('cathedral_lectern','Stone Scripture Lectern','Cathedral',cathedral_lectern,'wall',105),
 ('sewer_ooze_pipe','Leaking Sump Elbow','Sewer',sewer_pipe,'wall',200),
 ('sewer_sump_grate','Slime-Crusted Sump','Sewer',sewer_drain,'floor_patch',200),
 ('sewer_sludge_bank','Refuse and Sludge Bank','Sewer',sewer_sludge,'corner',260),
 ('catacombs_bone_pile','The Unburied Remains','Catacombs',catacombs_pile,'corner',160),
 ('catacombs_ossuary','Crowned Bone Shelves','Catacombs',catacombs_ossuary,'wall',180),
 ('warrens_barricade','Scavenger Palisade','Warrens',warrens_barricade,'wall',215),
 ('warrens_cache','Goblin Spear Cache','Warrens',warrens_cache,'corner',170),
 ('warrens_fungus','Pale Root Fungi','Warrens',warrens_fungus,'corner',170),
 ('crypts_sarcophagus','Crimson Vigil Sarcophagus','Crypts',crypt_sarcophagus,'wall',235),
 ('crypts_vigil','The Watchers Lantern','Crypts',crypt_vigil,'wall',90),
 ('crypts_standard','Sepulchral Procession Banner','Crypts',crypt_standard,'wall',145),
 ('fortress_armory','Warden Shield and Spear Rack','Fortress',fortress_armory,'wall',200),
 ('fortress_shackles','Forgotten Restraints','Fortress',fortress_shackles,'wall',130),
 ('deep_drake_skull','Horned Drake Skull','Deep',deep_skull,'corner',270),
 ('deep_ribcage','Drake Ribcage Remains','Deep',deep_ribcage,'corner',250),
 ('deep_crystals','Cold Mineral Outcrop','Deep',deep_crystals,'corner',165),
 ('infernal_seal','The Broken Binding Stone','Infernal',infernal_seal,'wall',160),
 ('infernal_brazier','Eight-Spined Offering Brazier','Infernal',infernal_brazier,'corner',160),
 ('hell_lava_basin','Basalt Lava Sump','Hell',hell_basin,'corner',240),
 ('hell_fissure','Ember Fissure','Hell',hell_fissure,'floor_patch',285),
 ('hell_drake_trophy','Charred Drake Trophy','Hell',hell_trophy,'wall',200),
]

def role_material(role,region):
    mat=bpy.data.materials.new(role);mat.use_nodes=True;nodes=mat.node_tree.nodes;links=mat.node_tree.links;bs=nodes.get('Principled BSDF')
    defaults={'Wall':(.13,.14,.12,1),'Trim':(.25,.19,.10,1),'Iron':(.08,.085,.085,1),'Bone':(.62,.54,.35,1),'Accent':(.08,.21,.04,1)}
    bs.inputs['Base Color'].default_value=defaults[role];bs.inputs['Roughness'].default_value=.78 if role!='Iron' else .42;bs.inputs['Metallic'].default_value=.82 if role=='Iron' else 0
    external=Path('J:/Lonemoore_Regional_Art');identity=Path('J:/Lonemoore_Regional_Identity')
    source=None
    if role=='Bone':source=(external,'Catacombs_ChalkBone')
    elif role=='Iron':source=(external,region+'_'+{'Cathedral':'OxidizedIron','Catacombs':'AgedBronze','Warrens':'BatteredIron','Crypts':'TarnishedSilver','Fortress':'WardenIron','Deep':'AncientIron','Infernal':'HeatIron','Hell':'InfernalMetal'}.get(region,'OxidizedIron'))
    elif role in ('Wall','Trim'):
        table={'Cathedral':('LimeWall','CarvedLimestone'),'Catacombs':('BurialRubble','ChalkLimestone'),'Warrens':('EarthShale','ScavengedOak'),'Crypts':('VigilMarble','SepulchralBorder'),'Fortress':('DefenseBlocks','ForgedStraps'),'Deep':('ChasmRock','MineralEdges'),'Infernal':('SealMonolith','RitualEdges'),'Hell':('ColumnBasalt','BrokenHaloStone')}
        if region in table:source=(identity,region+'_'+table[region][role=='Trim'])
    elif role=='Accent':
        accents={'Catacombs':'Catacombs_ChalkBone','Warrens':'Warrens_RootMoss','Crypts':'Crypts_CrimsonCloth','Fortress':'Fortress_BarredPlate','Deep':'Deep_MineralCrust','Infernal':'Infernal_HeatFissures','Hell':'Hell_EmberSeams'}
        if region in accents:source=(external,accents[region])
    if source and (source[0]/'manifest.json').exists():
        manifest=json.loads((source[0]/'manifest.json').read_text());entry=next((a for a in manifest['materials'] if a['id']==source[1]),None)
        if entry:
            for key,socket in [('base_color','Base Color'),('roughness','Roughness'),('metallic','Metallic')]:
                if key not in entry['maps']:continue
                t=nodes.new('ShaderNodeTexImage');t.image=bpy.data.images.load(str(source[0]/entry['maps'][key]),check_existing=True);t.image.colorspace_settings.name='sRGB' if key=='base_color' else 'Non-Color';links.new(t.outputs['Color'],bs.inputs[socket])
            if 'normal' in entry['maps']:
                t=nodes.new('ShaderNodeTexImage');t.image=bpy.data.images.load(str(source[0]/entry['maps']['normal']),check_existing=True);t.image.colorspace_settings.name='Non-Color'
                flip=nodes.new('ShaderNodeVectorMath');flip.operation='MULTIPLY';flip.inputs[1].default_value=(1,-1,1);links.new(t.outputs['Color'],flip.inputs[0])
                offset=nodes.new('ShaderNodeVectorMath');offset.operation='ADD';offset.inputs[1].default_value=(0,1,0);links.new(flip.outputs[0],offset.inputs[0])
                normal=nodes.new('ShaderNodeNormalMap');normal.inputs['Strength'].default_value=.65;links.new(offset.outputs[0],normal.inputs['Color']);links.new(normal.outputs['Normal'],bs.inputs['Normal'])
    if region=='Sewer':
        if role=='Wall':bs.inputs['Base Color'].default_value=(.19,.22,.17,1)
        if role=='Iron':bs.inputs['Base Color'].default_value=(.12,.13,.07,1)
        if role=='Accent':bs.inputs['Base Color'].default_value=(.065,.15,.014,1);bs.inputs['Roughness'].default_value=.16
    if role=='Bone' and region=='Hell':
        for link in list(bs.inputs['Base Color'].links):links.remove(link)
        bs.inputs['Base Color'].default_value=(.21,.13,.075,1)
    if role=='Accent' and region in ('Cathedral','Deep','Infernal','Hell'):
        colors={'Cathedral':(1,.43,.065,1),'Deep':(.06,.29,.36,1),'Infernal':(1,.10,.015,1),'Hell':(1,.19,.008,1)}
        bs.inputs['Emission Color'].default_value=colors[region];bs.inputs['Emission Strength'].default_value=.35 if region=='Deep' else 3.5
    return mat

def render_preview(obj,path):
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
    try:
        if '--cpu-preview' in sys.argv:raise RuntimeError('CPU preview requested')
        pref=bpy.context.preferences.addons['cycles'].preferences;pref.compute_device_type='OPTIX';pref.get_devices()
        for device in pref.devices:device.use=device.type=='OPTIX'
        scene.cycles.device='GPU'
    except Exception:scene.cycles.device='CPU'
    scene.render.resolution_x=900;scene.render.resolution_y=760;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.render.film_transparent=False
    world=bpy.data.worlds.new('Dressing preview atmosphere');world.use_nodes=True;world.node_tree.nodes['Background'].inputs[0].default_value=(.055,.067,.085,1);world.node_tree.nodes['Background'].inputs[1].default_value=.45;scene.world=world
    try:scene.view_settings.view_transform='AgX'
    except Exception:pass
    bounds=[obj.matrix_world@Vector(v) for v in obj.bound_box];low=Vector(tuple(min(v[i] for v in bounds) for i in range(3)));high=Vector(tuple(max(v[i] for v in bounds) for i in range(3)));center=(low+high)/2;extent=max(high-low)
    bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,min(0,low.z)-.008));ground=bpy.context.object;ground.name='Preview stage - never exported'
    material=bpy.data.materials.new('Preview stage');material.diffuse_color=(.075,.089,.095,1);material.use_nodes=True;material.node_tree.nodes['Principled BSDF'].inputs['Base Color'].default_value=(.075,.089,.095,1);material.node_tree.nodes['Principled BSDF'].inputs['Roughness'].default_value=.87;ground.data.materials.append(material)
    for name,location,power,size in [('Key',(-3,-4,6),650,4),('Fill',(4,-1,3),380,3),('Rim',(1,4,5),850,3)]:
        data=bpy.data.lights.new(name,'AREA');data.energy=power;data.shape='DISK';data.size=size;o=bpy.data.objects.new(name,data);scene.collection.objects.link(o);o.location=center+Vector(location)*(extent/2);o.rotation_euler=(center-o.location).to_track_quat('-Z','Y').to_euler()
    camera_data=bpy.data.cameras.new('Asset preview camera');camera=bpy.data.objects.new('Asset preview camera',camera_data);scene.collection.objects.link(camera)
    camera.location=center+Vector((3.4,-5.4,3.0))*max(.6,extent)/3;camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler();camera_data.type='ORTHO';camera_data.ortho_scale=max(1.05,extent*1.36);scene.camera=camera
    view=camera.rotation_euler.to_matrix().transposed();projected=[view@(v-center) for v in bounds]
    span_x=max(v.x for v in projected)-min(v.x for v in projected);span_y=max(v.y for v in projected)-min(v.y for v in projected)
    camera_data.ortho_scale=max(span_x,span_y*scene.render.resolution_x/scene.render.resolution_y)*1.13
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Scenes'/(obj.name+'.blend')))
    scene.render.filepath=str(path);bpy.ops.render.render(write_still=True)

lectern_only='--lectern-only' in sys.argv
reports=json.loads((OUT/'manifest.json').read_text())['props'] if lectern_only else []
prior_validation=json.loads((OUT/'validation.json').read_text()) if lectern_only else None
if lectern_only:assert len(reports)==24 and len({p['id'] for p in reports})==24
for ident,name,region,builder,placement,clearance in PROPS:
    if lectern_only and ident!='cathedral_lectern':continue
    bpy.ops.wm.read_factory_settings(use_empty=True);scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1
    V=[];F=[];M=[];UV=[];REGION=region;builder()
    mesh=bpy.data.meshes.new('SM_RD_'+ident);mesh.from_pydata(V,[],F);mesh.update();obj=bpy.data.objects.new(mesh.name,mesh);scene.collection.objects.link(obj)
    for role in ROLES:mesh.materials.append(role_material(role,region))
    uv=mesh.uv_layers.new(name='PropUV_1p2m')
    for polygon,role,coords in zip(mesh.polygons,M,UV):
        polygon.material_index=role
        for loop,coord in zip(polygon.loop_indices,coords):uv.data[loop].uv=coord
    bpy.context.view_layer.objects.active=obj;obj.select_set(True);mesh.calc_loop_triangles()
    # Only prop geometry is selected; preview studio is added after FBX export.
    fbx=OUT/'Meshes'/(obj.name+'.fbx');bpy.ops.export_scene.fbx(filepath=str(fbx),use_selection=True,object_types={'MESH'},apply_unit_scale=True,global_scale=1,axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,bake_anim=False,add_leaf_bones=False)
    low=[min(v[i] for v in V) for i in range(3)];high=[max(v[i] for v in V) for i in range(3)]
    report=dict(id=ident,name=name,region=region,regionIndex=REGIONS.index(region),mesh=f'/Game/RegionalDressing/Meshes/{obj.name}.{obj.name}',fbx=str(fbx.relative_to(ROOT)),blend=str((OUT/'Scenes'/(obj.name+'.blend')).relative_to(ROOT)),triangles=len(mesh.loop_triangles),vertices=len(mesh.vertices),boundsCm=[round((high[i]-low[i])*100,2) for i in range(3)],minCm=[round(v*100,2) for v in low],maxCm=[round(v*100,2) for v in high],placement=placement,clearanceDiameterCm=clearance,materialSlots=ROLES,materials=[f'/Game/RegionalDressing/Materials/MI_RD_{region}_{role}.MI_RD_{region}_{role}' for role in ROLES],nanite=True,collision=False,animatedActor=False,addsPointLights=False)
    if lectern_only:reports[next(i for i,p in enumerate(reports) if p['id']==ident)]=report
    else:reports.append(report)
    print('REGIONAL_DRESSING_MESH',ident,report['triangles'],report['boundsCm'],flush=True)
    if '--export-only' not in sys.argv:render_preview(obj,OUT/'Renders'/(ident+'.png'))
    else:bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Scenes'/(obj.name+'.blend')))
    (OUT/'manifest.json').write_text(json.dumps(dict(version=1,roomKitCompatible=True,blenderFront=[0,-1,0],nativeFront=[0,1,0],materialRoles=ROLES,assetCount=len(reports),props=reports,notes=['Opaque surfaces only; Nanite and HISM compatible.','No per-prop point lights, particles, translucency, WPO, actor tick or collision.','UVs are authored in 1.2 metre units. Sludge/lava tops are raised from the base floor.','Floor-centered origin; use unit scale and inspect authored bounds before placing.','Large props belong at wall/corner furnishing markers, outside portal cells and movement lanes.','Preview lighting is Blender studio lighting, not Unreal game lighting.','Existing regional texture files are read-only references.']),indent=2))
assert len(reports)==24 and max(p['triangles'] for p in reports)<18000
runtime=json.loads((ROOT/'Content/Game/Data/regional_dressing.json').read_text()) if lectern_only else dict(version=1,blenderFront=[0,-1,0],nativeFront=[0,1,0],materialRoles=ROLES,assets=[])
for p in reports:
    if lectern_only and p['id']!='cathedral_lectern':continue
    # Measured native prop imports reflect Y. Importer replaces these predicted
    # bounds with measured imported bounds after checking unit scale.
    lo=p['minCm'];hi=p['maxCm'];bounds=p['boundsCm']
    item=dict(id=p['id'],name=p['name'],regionIndex=p['regionIndex'],mesh=p['mesh'],placement='patch' if p['placement']=='floor_patch' else 'wall' if p['placement']=='wall' else 'floor',boundsCm=bounds,minCm=[lo[0],-hi[1],lo[2]],maxCm=[hi[0],-lo[1],hi[2]],maxScale=1.,cornerScale=round(min(1.,100/max(bounds[:2])),4),landmark=max(bounds[:2])>170,triangles=p['triangles'],materialSlots=ROLES,collision=False,addsPointLights=False)
    if lectern_only:runtime['assets'][next(i for i,a in enumerate(runtime['assets']) if a['id']==p['id'])]=item
    else:runtime['assets'].append(item)
assert len(runtime['assets'])==24
(ROOT/'Content/Game/Data/regional_dressing.json').write_text(json.dumps(runtime,indent=2))
records=[]
for p in reports:
    files=[ROOT/p['fbx'],ROOT/p['blend'],OUT/'Renders'/(p['id']+'.png')]
    assert all(f.exists() and f.stat().st_size>1000 for f in files)
    records.append(dict(id=p['id'],triangles=p['triangles'],boundsCm=p['boundsCm'],files=[dict(path=str(f.relative_to(ROOT)),sha256=hashlib.sha256(f.read_bytes()).hexdigest(),bytes=f.stat().st_size) for f in files]))
if lectern_only:
    previous={p['id']:p for p in prior_validation['records']}
    assert all(p==previous[p['id']] for p in records if p['id']!='cathedral_lectern'), 'Non-lectern source files changed'
validation=dict(status='PASS',scope='Source assets and previews only; Unreal import and native placement validation are separate',props=24,regions=9,triangles=sum(p['triangles'] for p in reports),maxTriangles=max(p['triangles'] for p in reports),failures=0,records=records)
if lectern_only:validation.update(selectiveUpdate='cathedral_lectern',unchangedOtherProps=23)
(OUT/'validation.json').write_text(json.dumps(validation,indent=2))
print('REGIONAL_DRESSING_BLENDER_COMPLETE props=24 triangles='+str(sum(p['triangles'] for p in reports)),flush=True)
