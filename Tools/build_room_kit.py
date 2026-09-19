"""Build the editable Gothic room kit in Blender. All dimensions are metres.

Run with Blender --background --python Tools/build_room_kit.py.
Run with ordinary Python --manifest-only to publish the navigation contract first.
No existing game artwork is modified. Wall/floor/vault surfaces share 2m UV units.
"""
import json, math, sys, hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'ArtSource/RoomKit'
DATA = ROOT / 'Content/Game/Data/room_kit.json'
ROLES = ['Wall', 'Floor', 'Vault', 'Trim', 'Iron']
BASE = ['###.###', '#.....#', '#.....#', '.......', '#.....#', '#.....#', '###.###']
OCT = ['###.###', '##...##', '#.....#', '.......', '#.....#', '##...##', '###.###']
CROSS = ['###.###', '##...##', '#.....#', '.......', '#.....#', '##...##', '###.###']
HALL = ['###.###'] * 7
GALLERY = ['###.###'] + ['##...##'] * 5 + ['###.###']
TURN = ['###.###'] * 3 + ['###....'] + ['#######'] * 3
TEE = ['###.###'] * 3 + ['###....'] + ['###.###'] * 3
BRIDGE = ['###.###'] + ['#~~.~~#'] * 5 + ['###.###']
CISTERN = ['###.###', '#~~.~~#', '#~~.~~#', '.......', '#~~.~~#', '#~~.~~#', '###.###']

def spec(id, name, rows=BASE, sockets=15, ceiling=740, style='chamber', **kw):
    rows=list(rows)
    # Unsupported ports are structurally sealed in the authored shell.
    for bit,x,y in [(1,3,0),(2,6,3),(4,3,6),(8,0,3)]:
        if not sockets & bit: rows[y]=rows[y][:x]+'#'+rows[y][x+1:]
    lights=[]
    for y in (-800, 800):
        for x in (-1,1):
            tx=165 if style in ('hall','approach','turn') else 520 if style in ('gallery','burial') else 710 if style in ('round','octagon','reliquary','ruin','cavern','chapel','cross') else 920
            xx=tx*x;yy=y*.72 if tx==710 else y
            gx,gy=round(xx/400)+3,round(yy/400)+3
            if 0<=gx<7 and 0<=gy<7 and rows[gy][gx]!='#':
                lights.append({'position':[xx,yy,310], 'yaw':0 if x<0 else 180, 'kind':'torch','variant':0})
    props=[]
    if style not in ('hall','turn','bridge','approach'):
        for x,y,var in [(-800,-800,0),(800,800,1),(-800,800,2)]:
            if rows[y//400+3][x//400+3]=='.': props.append({'position':[x+(-160 if x<0 else 160),y+(-160 if y<0 else 160),0],'yaw':0 if x<0 else 180,'kind':'furnishing','variant':var})
    return dict(id=id,name=name,mesh=f'/Game/RoomKit/Meshes/SM_RK_{id}.SM_RK_{id}',rows=rows,socketMask=sockets,ceilingCm=ceiling,style=style,lights=lights,props=props,**kw)

TEMPLATES = [
    spec('arrival_chamber','The Pilgrim Vestibule',style='arrival'),
    spec('vaulted_hall','Ribbed Passage',HALL,5,740,'hall'),
    spec('long_gallery','Gallery of the Vigil',GALLERY,5,740,'gallery'),
    spec('short_turn','The Stone Elbow',TURN,3,640,'turn'),
    spec('rounded_turn','The Curved Ambulatory',TURN,3,680,'turn',roundCorners=True),
    spec('t_junction','Threefold Passage',TEE,7,740,'hall'),
    spec('cross_hall','Crossing of Lanterns',CROSS,15,780,'cross'),
    spec('circular_ossuary','The Round Ossuary',OCT,15,820,'round',roundCorners=True),
    spec('octagonal_chamber','Eightfold Chapterhouse',OCT,15,780,'octagon'),
    spec('cruciform_chapel','Chapel of Lost Names',CROSS,15,840,'chapel'),
    spec('burial_gallery','The Sepulchral Walk',GALLERY,5,740,'burial'),
    spec('twin_crypt','The Twin Tombs',BASE,15,720,'twin'),
    spec('reliquary','The Iron Reliquary',OCT,15,760,'reliquary'),
    spec('sunken_shrine','The Drowned Shrine',BASE,15,760,'shrine'),
    spec('guard_barracks','Wardens Rest',BASE,15,640,'barracks'),
    spec('ruined_library','The Broken Archive',BASE,15,700,'library'),
    spec('pillar_court','Court of Black Pillars',BASE,15,780,'pillars'),
    spec('broken_sanctum','Sanctum of the Fallen Crown',OCT,15,800,'ruin'),
    spec('cavern_hall','The Hollow Beneath',OCT,15,920,'cavern',roundCorners=True),
    spec('void_bridge','Bridge of the Unseen Deep',BRIDGE,5,860,'bridge',pit='void'),
    spec('fire_bridge','The Ember Crossing',BRIDGE,5,860,'bridge',pit='fire'),
    spec('cistern_crossing','Four Ways over the Cistern',CISTERN,15,780,'cistern',pit='water'),
    spec('boss_approach','The Procession of Judgment',HALL,5,780,'approach'),
    spec('boss_arena','The Throne of the Guardian',BASE,5,900,'arena'),
    spec('sealed_descent','The Last Seal',BASE,1,860,'descent'),
]
MANIFEST = dict(version=1,cellCm=400,spanCells=7,centerCell=[3,3],materialSlots=ROLES,
    capMesh='/Game/RoomKit/Meshes/SM_RK_portal_cap.SM_RK_portal_cap',
    capSizeCm=[400,36,600], socketOrder=['N','E','S','W'],socketClearWidthCm=400,
    textureRepeatMetres=2, templates=TEMPLATES)
DATA.parent.mkdir(parents=True,exist_ok=True)
DATA.write_text(json.dumps(MANIFEST,indent=2)+'\n')
if '--manifest-only' in sys.argv:
    print('ROOM_KIT_MANIFEST_READY templates=25');sys.exit(0)

import bpy
from mathutils import Vector, Matrix
from mathutils.geometry import tessellate_polygon

OUT.mkdir(parents=True,exist_ok=True)
(OUT/'Meshes').mkdir(exist_ok=True)
(OUT/'Scenes').mkdir(exist_ok=True)
(OUT/'Renders').mkdir(exist_ok=True)
VERTS=[];FACES=[];MATS=[];UVS=[];PARTS=[]
ROOM_CONTEXT=None

def face(points,role,uv=None):
    start=len(VERTS);VERTS.extend(points);FACES.append(tuple(range(start,start+len(points))));MATS.append(ROLES.index(role))
    if uv is None:
        n=(Vector(points[1])-Vector(points[0])).cross(Vector(points[2])-Vector(points[0]))
        axis=max(range(3),key=lambda i:abs(n[i]));axes=[i for i in range(3) if i!=axis]
        uv=[(p[axes[0]]/2,p[axes[1]]/2) for p in points]
    UVS.append(uv)

def box(c,s,role='Trim'):
    x,y,z=c;hx,hy,hz=[a/2 for a in s]
    # Floor-contact solids penetrate 4cm; their underside never shares the deck plane.
    if abs(z-hz)<.00001:z-=.02;hz+=.02
    vs=[(x+sx*hx,y+sy*hy,z+sz*hz) for sx,sy,sz in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
    for inds in [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:face([vs[i] for i in inds],role)

def beam(a,b,width,depth,role='Trim'):
    v=Vector(b)-Vector(a);w=v.normalized().cross(Vector((0,1,0)))
    if w.length<.1:w=v.normalized().cross(Vector((1,0,0)))
    w.normalize();d=v.normalized().cross(w).normalized();c=(Vector(a)+Vector(b))/2
    verts=[tuple(c+w*sx*width/2+d*sy*depth/2+v*sz/2) for sx,sy,sz in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]]
    for inds in [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:face([verts[i] for i in inds],role)

def column(x,y,height,r=.30,role='Trim',sides=8):
    # Structural monoliths, not individual masonry blocks.
    for i in range(sides):
        a=i*2*math.pi/sides;b=(i+1)*2*math.pi/sides
        face([(x+math.cos(a)*r,y+math.sin(a)*r,.12),(x+math.cos(b)*r,y+math.sin(b)*r,.12),(x+math.cos(b)*r,y+math.sin(b)*r,height),(x+math.cos(a)*r,y+math.sin(a)*r,height)],role)
    box((x,y,.13),(r*2.7,r*2.7,.26),role)
    box((x,y,height-.12),(r*2.6,r*2.6,.24),role)

def arch(y,half=1.83,spring=4.25,peak=6.8,role='Trim',thick=.18):
    for side in (-1,1):
        support=ceiling(ROOM_CONTEXT,side*half,y)-.12 if ROOM_CONTEXT else spring
        column(side*half,y,support,.20,role)
        prev=(side*half,y,support)
        for i in range(1,13):
            t=i/12;x=side*half*(1-t);z=spring+(peak-spring)*(math.sin(t*math.pi/2)**.72)
            if ROOM_CONTEXT:z=ceiling(ROOM_CONTEXT,x,y)-.12
            nxt=(x,y,z);beam(prev,nxt,thick,.26,role);prev=nxt

def rows_edges(rows):
    # Directed perimeter edges keep occupied area on their left, including pits.
    edges=[]
    for y in range(7):
        for x in range(7):
            if rows[y][x]=='#':continue
            for dx,dy,a,b in [(0,-1,(x,y),(x+1,y)),(1,0,(x+1,y),(x+1,y+1)),(0,1,(x+1,y+1),(x,y+1)),(-1,0,(x,y+1),(x,y))]:
                nx,ny=x+dx,y+dy
                if not (0<=nx<7 and 0<=ny<7) or rows[ny][nx]=='#':edges.append((a,b))
    return edges

def interior_ceiling(t,x,y):
    peak=t['ceilingCm']/100
    style=t['style'];span=2 if style in ('hall','approach','turn') else 6 if style in ('gallery','burial') else 10
    if style=='cavern':return peak-.8+.36*math.sin(x*.49)*math.cos(y*.32)+.28*math.cos(y*.57+x*.13)
    if style in ('round','octagon','shrine','reliquary','arena'):
        return peak-1.7+1.7*math.sqrt(max(0,1-(x*x+y*y)/210))
    if style in ('barracks','library'):return peak-.18*abs(math.sin(y*.27))
    # Narrow entry stubs lower gracefully to a consistent Gothic tunnel vault.
    if abs(y)>10:span=2
    if abs(x)>10:return peak-1.5+1.5*max(0,1-abs(y)/2)**.7
    return peak-1.5+1.5*max(0,1-abs(x)/span)**.7

def ceiling(t,x,y):
    h=interior_ceiling(t,x,y)
    # Every socket ends in exactly the same pointed throat. The last four metres
    # blend to it, so different-height neighbours have no vertical roof crack.
    if abs(y)>10 and abs(x)<=2.001:
        blend=min(1,(abs(y)-10)/4);throat=5.6+.8*max(0,1-abs(x)/2)**.7
        return h*(1-blend)+throat*blend
    if abs(x)>10 and abs(y)<=2.001:
        blend=min(1,(abs(x)-10)/4);throat=5.6+.8*max(0,1-abs(y)/2)**.7
        return h*(1-blend)+throat*blend
    return h

def smooth_outline(t):
    if t['style']=='round':
        # True circular masonry shell with four straight, grid-matched doorway stubs.
        r=10.75;a=math.asin(2/r);s=math.sqrt(r*r-4);poly=[]
        for quadrant in range(4):
            rot=quadrant*math.pi/2
            local=[(14,-2),(14,2),(s,2)]
            local += [(r*math.cos(a+(math.pi/2-2*a)*i/24),r*math.sin(a+(math.pi/2-2*a)*i/24)) for i in range(1,25)]
            local += [(2,14)]
            for x,y in local:poly.append((x*math.cos(rot)-y*math.sin(rot),x*math.sin(rot)+y*math.cos(rot)))
        clean=[]
        for p in poly:
            if not clean or (Vector(p)-Vector(clean[-1])).length>.0001:clean.append(p)
        if (Vector(clean[0])-Vector(clean[-1])).length<.0001:clean.pop()
        return clean
    edges=rows_edges(t['rows']);by_start={a:b for a,b in edges};start=edges[0][0];p=start;pts=[]
    while True:
        pts.append(((p[0]-3.5)*4,(p[1]-3.5)*4));p=by_start[p]
        if p==start:break
    clean=[]
    for i,p in enumerate(pts):
        before=Vector(pts[i-1]);at=Vector(p);after=Vector(pts[(i+1)%len(pts)])
        if abs((at-before).cross(after-at))>.001:clean.append(p)
    out=[];radius=1.65 if t['style']=='turn' else .75
    for i,p in enumerate(clean):
        at=Vector(p);before=Vector(clean[i-1]);after=Vector(clean[(i+1)%len(clean)])
        if abs(at.x)>13.9 or abs(at.y)>13.9:out.append(tuple(at));continue
        d=min(radius,(at-before).length*.4,(after-at).length*.4)
        a=at+(before-at).normalized()*d;b=at+(after-at).normalized()*d
        for j in range(9):
            f=j/8;q=(1-f)**2*a+2*(1-f)*f*at+f*f*b;out.append(tuple(q))
    return out

def bounded_floor_grid(outline, spacing=1.25):
    """Clip a regular grid to the exact shell; avoid room-sized Nanite fan triangles.

    The three curved footprints have no feature narrower than a grid cell. Each
    clipped cell is therefore one simple polygon, including the concave elbow.
    Metric UVs continue to come directly from XY, independent of tessellation.
    """
    def clip(points, axis, boundary, greater):
        result=[]
        for i,p in enumerate(points):
            q=points[(i+1)%len(points)]
            p_inside=p[axis]>=boundary if greater else p[axis]<=boundary
            q_inside=q[axis]>=boundary if greater else q[axis]<=boundary
            if p_inside:result.append(p)
            if p_inside!=q_inside:
                alpha=(boundary-p[axis])/(q[axis]-p[axis])
                point=[p[j]+(q[j]-p[j])*alpha for j in range(2)]
                point[axis]=boundary
                result.append(tuple(point))
        return result

    def area(points):
        return .5*sum(p[0]*points[(i+1)%len(points)][1]-p[1]*points[(i+1)%len(points)][0] for i,p in enumerate(points))

    expected=area(outline);actual=0;triangle_count=0
    assert expected>0,'Floor outline must be counterclockwise'
    min_x=math.floor(min(p[0] for p in outline)/spacing)
    max_x=math.ceil(max(p[0] for p in outline)/spacing)
    min_y=math.floor(min(p[1] for p in outline)/spacing)
    max_y=math.ceil(max(p[1] for p in outline)/spacing)
    for iy in range(min_y,max_y):
        for ix in range(min_x,max_x):
            points=list(outline)
            for axis,boundary,greater in [(0,ix*spacing,True),(0,(ix+1)*spacing,False),(1,iy*spacing,True),(1,(iy+1)*spacing,False)]:
                points=clip(points,axis,boundary,greater)
                if len(points)<3:break
            clean=[]
            for p in points:
                if not clean or math.dist(p,clean[-1])>1e-8:clean.append(p)
            if len(clean)>1 and math.dist(clean[0],clean[-1])<1e-8:clean.pop()
            # Clipping can retain collinear original wall samples. Remove them
            # before tessellation so corner-touching cells have no zero-area faces.
            changed=True
            while changed and len(clean)>=3:
                changed=False
                for i,p in enumerate(clean):
                    a=clean[i-1];b=clean[(i+1)%len(clean)]
                    cross=(p[0]-a[0])*(b[1]-p[1])-(p[1]-a[1])*(b[0]-p[0])
                    if abs(cross)<1e-10:
                        clean.pop(i);changed=True;break
            if len(clean)<3 or abs(area(clean))<1e-9:continue
            assert area(clean)>0,'Clipped floor cell inverted'
            vertices=[Vector((x,y,0)) for x,y in clean]
            for indices in tessellate_polygon([vertices]):
                # Blender 5.2 returns indices; older releases return Vectors.
                triangle=[vertices[v] if isinstance(v,int) else v for v in indices]
                cross=(triangle[1]-triangle[0]).cross(triangle[2]-triangle[0]).z
                if abs(cross)<1e-8:continue
                if cross<0:triangle=(triangle[0],triangle[2],triangle[1]);cross=-cross
                assert max((triangle[(j+1)%3]-triangle[j]).length for j in range(3))<=spacing*math.sqrt(2)+1e-5
                face([tuple(v) for v in triangle],'Floor');actual+=cross*.5;triangle_count+=1
    assert abs(actual-expected)<1e-4,('Curved floor coverage changed',expected,actual)
    print('ROOM_KIT_BOUNDED_FLOOR',triangle_count,'triangles','area',round(actual,6),'maxEdge',round(spacing*math.sqrt(2),6),flush=True)


def curved_shell(t):
    outline=smooth_outline(t);poly=[]
    # In particular, all four doorway profiles have vertices every metre, just
    # like rectangular rooms. Matching endpoints alone would leave a roof slit.
    for i,a in enumerate(outline):
        b=outline[(i+1)%len(outline)];count=max(1,math.ceil((Vector(b)-Vector(a)).length-1e-6))
        for j in range(count):poly.append(tuple(Vector(a).lerp(Vector(b),j/count)))
    n=len(poly)
    # Keep near-camera floor triangles bounded for Nanite. The previous radial
    # fan disappeared at the central camera position in the native renderer.
    bounded_floor_grid(poly)
    # These footprints are star-shaped around the central cell. Ring triangulation
    # supplies a genuinely curved vault instead of a single broad ceiling polygon.
    center=(0,-.02) if t['style']=='turn' else (0,0)
    for i in range(n):
        a=poly[i];b=poly[(i+1)%n]
        for r in range(1,13):
            inner=(r-1)/12;outer=r/12
            coords=[(center[0]+(a[0]-center[0])*inner,center[1]+(a[1]-center[1])*inner),(center[0]+(b[0]-center[0])*inner,center[1]+(b[1]-center[1])*inner),(center[0]+(b[0]-center[0])*outer,center[1]+(b[1]-center[1])*outer),(center[0]+(a[0]-center[0])*outer,center[1]+(a[1]-center[1])*outer)]
            if r==1:coords=coords[1:]
            # Inner A -> inner B -> outer B -> outer A is already clockwise.
            # Keep it downward-facing: Unreal culls an upward-facing roof skin.
            face([(x,y,ceiling(t,x,y)) for x,y in coords],'Vault')
    u=0
    for i in range(n):
        a=Vector(poly[i]);b=Vector(poly[(i+1)%n]);length=(b-a).length
        if length<.0001:continue
        if (abs(abs(a.x)-14)<.001 and abs(abs(b.x)-14)<.001) or (abs(abs(a.y)-14)<.001 and abs(abs(b.y)-14)<.001):u+=length;continue
        ha=ceiling(t,a.x,a.y);hb=ceiling(t,b.x,b.y)
        wall_points=[(a.x,a.y,-.16),(a.x,a.y,ha),(b.x,b.y,hb),(b.x,b.y,-.16)]
        # Straight socket cheeks use the exact same metric phase as every other
        # room. Curved masonry uses unbroken arc-length UVs around its curved wall.
        is_stub=max(abs(a.x),abs(a.y),abs(b.x),abs(b.y))>10 and (abs(a.x-b.x)<.001 or abs(a.y-b.y)<.001)
        face(wall_points,'Wall',None if is_stub else [(u/2,-.08),(u/2,ha/2),((u+length)/2,hb/2),((u+length)/2,-.08)])
        norm=Vector((b.y-a.y,a.x-b.x)).normalized()*.25;aa=a+norm;bb=b+norm
        face([(aa.x,aa.y,-.16),(bb.x,bb.y,-.16),(bb.x,bb.y,hb+.15),(aa.x,aa.y,ha+.15)],'Wall')
        u+=length

def floors_and_vault(t):
    if t.get('roundCorners'):
        curved_shell(t);return
    rows=t['rows']
    for y in range(7):
        for x in range(7):
            if rows[y][x]=='#':continue
            x0=(x-3)*4-2;y0=(y-3)*4-2
            if rows[y][x]=='.':
                face([(x0,y0,0),(x0+4,y0,0),(x0+4,y0+4,0),(x0,y0+4,0)],'Floor')
            # Runtime supplies water/lava/void at -850cm; no coincident basin here.
            # Smooth continuous ceiling skin: adjoining cells use identical coordinates.
            for a in range(4):
                for b in range(4):
                    coords=[(x0+a,y0+b),(x0+a,y0+b+1),(x0+a+1,y0+b+1),(x0+a+1,y0+b)]
                    face([(xx,yy,ceiling(t,xx,yy)) for xx,yy in coords],'Vault')
    # Floors have no overlapping trims. Each boundary strip belongs to this module.
    for a,b in rows_edges(rows):
        pa=Vector(((a[0]-3.5)*4,(a[1]-3.5)*4,0));pb=Vector(((b[0]-3.5)*4,(b[1]-3.5)*4,0))
        # Native socket edges are deliberately open; neighbors/caps own their closures.
        if (abs(pa.x)==14 and abs(pb.x)==14) or (abs(pa.y)==14 and abs(pb.y)==14):continue
        tangent=(pb-pa).normalized();inside=Vector((-tangent.y,tangent.x,0))
        # Wall inward face is exact tile perimeter. Thick outer shell prevents daylight.
        for j in range(4):
            p=pa.lerp(pb,j/4);q=pa.lerp(pb,(j+1)/4);hp=ceiling(t,p.x,p.y);hq=ceiling(t,q.x,q.y)
            low=-9 if t.get('pit') else -.16
            face([tuple(p+Vector((0,0,low))),tuple(p+Vector((0,0,hp))),tuple(q+Vector((0,0,hq))),tuple(q+Vector((0,0,low)))],'Wall')
            p2=p-inside*.25;q2=q-inside*.25
            face([tuple(p2+Vector((0,0,low))),tuple(q2+Vector((0,0,low))),tuple(q2+Vector((0,0,hq+.15))),tuple(p2+Vector((0,0,hp+.15)))],'Wall')
        mid=(pa+pb)/2+inside*.07
        # Skirting projects 14cm inward, top/face stand proud of wall plane.
        size=(4,.14,.24) if abs(tangent.x)>.5 else (.14,4,.24)
        box((mid.x,mid.y,.12),size,'Trim')
    if t.get('pit'):
        for y in range(7):
            for x in range(7):
                if rows[y][x]!='.':continue
                for dx,dy in [(1,0),(-1,0),(0,1),(0,-1)]:
                    nx,ny=x+dx,y+dy
                    if 0<=nx<7 and 0<=ny<7 and rows[ny][nx]=='~':
                        xx=(x-3)*4+dx*2;yy=(y-3)*4+dy*2
                        box((xx,yy,-.21),(.30 if dx else 4,4 if dx else .30,.52),'Trim')
                        # Two rails and two posts keep bridges visually readable.
                        box((xx,yy,.95),(.14 if dx else 4,4 if dx else .14,.16),'Iron')
                        for off in (-1.5,1.5):box((xx+(off if dy else 0),yy+(off if dx else 0),.49),(.16,.16,.98),'Iron')

def ornament(t):
    global ROOM_CONTEXT
    ROOM_CONTEXT=t
    style=t['style'];peak=t['ceilingCm']/100
    # Each connector has an intentional stone reveal. It frames the join and
    # makes a change from straight UV masonry to curved masonry architectural.
    for bit,rotation in [(1,math.pi),(2,-math.pi/2),(4,0),(8,math.pi/2)]:
        if not t['socketMask']&bit:continue
        first=len(VERTS);arch(13.55,1.84,5.30,6.18,thick=.17)
        for index in range(first,len(VERTS)):
            x,y,z=VERTS[index];VERTS[index]=(x*math.cos(rotation)-y*math.sin(rotation),x*math.sin(rotation)+y*math.cos(rotation),z)
    if style in ('hall','approach','gallery','burial'):
        half=1.83 if style in ('hall','approach') else 5.75
        for y in (-10,-6,-2,2,6,10):
            arch(y,half,peak-2.9,peak-.10,thick=.23 if style=='approach' else .16)
            if style in ('approach','burial','gallery'):
                for side in (-1,1):
                    # Coffin ledges sit clear of the central route. Dark recess is wall shader.
                    x=side*(half-.25)
                    box((x,y+1.25,.65),(.52,1.45,.30),'Trim')
                    box((x,y+1.25,.91),(.44,1.20,.18),'Wall')
                    if style=='burial':
                        box((x,y+1.25,2.10),(.57,1.55,.20),'Trim')
                        box((x,y+1.25,2.34),(.43,1.20,.27),'Wall')
                    if style=='approach':box((side*(half-.15),y+1.25,3.25),(.11,.45,2.3),'Iron')
    elif style=='turn':
        arch(-9,1.82,peak-2.25,peak-.10)
        arch(-5,1.82,peak-2.25,peak-.10)
        # A full-height corner pilaster touches the actual inner wall. The former
        # short suspended corbel ended in open air in the native first-person view.
        corner=2.38 if t.get('roundCorners') else 1.94
        beam((corner,-corner,-.04),(corner,-corner,ceiling(t,corner,-corner)+.06),.24,.24)
    else:
        count={'round':12,'octagon':8,'chapel':8,'cross':8,'arena':12,'reliquary':8}.get(style,4)
        if style in ('round','octagon','reliquary'):
            for i in range(count):
                a=(i+.5)*2*math.pi/count;x=8.1*math.cos(a);y=8.1*math.sin(a)
                # Pillars occupy intersections BETWEEN centre-to-centre lanes.
                x=math.floor(x/4)*4+2;y=math.floor(y/4)*4+2
                # Only authored peripheral positions outside centre-to-centre routes.
                gx,gy=round(x/4)+3,round(y/4)+3
                if t['rows'][gy][gx]=='#':continue
                column(x,y,ceiling(t,x,y)-.20,.24)
                beam((x,y,ceiling(t,x,y)-.27),(x*.2,y*.2,peak-.12),.18,.18)
        elif style not in ('arrival','chapel','cross','arena','descent'):
            for x in (-6,6):
                for y in (-6,6):
                    h=ceiling(t,x,y)-.12
                    if style=='ruin':h=2.5 if x*y>0 else h
                    if style=='cavern':h=2.2+(x+y+12)/12
                    column(x,y,h,.42 if style in ('pillars','arena') else .28, 'Wall' if style=='cavern' else 'Trim',6 if style=='cavern' else 8)
        if style in ('arrival','chapel','cross','arena','descent'):
            for y in (-6,-2,2,6):arch(y,5.7,peak-3.0,peak-.18,thick=.23)
        if style in ('twin','shrine','descent','arena'):
            # Side tombs/altar platforms never cover the central route or stairs anchor.
            for x,y in [(-6,-2),(6,2)]:
                box((x,y,.34),(1.5,2.65,.68),'Trim')
                box((x,y,.76),(1.65,2.8,.18),'Wall')
                if style=='arena':column(x,y,4.8,.26,'Iron')
        if style=='reliquary':
            for x,y in [(-6,-6),(6,-6),(-6,6),(6,6)]:
                box((x,y,.7),(1.5,1.5,1.4),'Trim')
                for a in (-.62,.62):
                    for b in (-.62,.62):column(x+a,y+b,3,.055,'Iron',6)
                box((x,y,3),(1.4,1.4,.13),'Iron')
        if style=='shrine':
            for x,y in [(-6,-6),(6,-6),(-6,6),(6,6)]:
                box((x,y,.16),(2.0,2.0,.32),'Trim');column(x,y,1.4,.48,'Iron',12)
        if style in ('library','barracks'):
            for x in (-9,9):
                for y in (-6,-2,2,6):
                    box((x,y,1.7),(.70,2.8,3.4),'Trim')
                    for z in ((.35,1.3,2.3,3.35) if style=='library' else (.35,2.65)):
                        box((x-.40*(1 if x>0 else -1),y,z),(.17,2.85,.10),'Iron')
                    if style=='barracks':
                        for yy in (-.7,0,.7):beam((x-.5*(1 if x>0 else -1),y+yy,1.3),(x-.5*(1 if x>0 else -1),y+yy,2.7),.08,.08,'Iron')
        if style in ('ruin','cavern'):
            for i in range(7):
                x=(-1 if i%2 else 1)*9.4;y=(-6,-2,2,6)[i%4]
                box((x,y,.22+(i%2)*.2),(.7+(i%3)*.13,.8,.44+(i%2)*.4),'Wall')
        if style=='bridge':
            for y in (-9,9):arch(y,1.79,4.4,peak-.15,thick=.24)
            if t.get('pit')=='fire':
                for side in (-1,1):
                    for y in (-6,0,6):column(side*6,y,3.2,.5,'Iron',6)
        if style=='cistern':
            for x in (-6,6):
                for y in (-6,6):column(x,y,peak-1.2,.7,'Wall')

def preview_material(role):
    m=bpy.data.materials.new(role);m.use_nodes=True;bs=m.node_tree.nodes.get('Principled BSDF')
    colors={'Wall':(.22,.20,.16,1),'Floor':(.15,.14,.12,1),'Vault':(.17,.16,.14,1),'Trim':(.31,.28,.21,1),'Iron':(.075,.063,.048,1)}
    bs.inputs['Base Color'].default_value=colors[role];bs.inputs['Roughness'].default_value=.78
    bs.inputs['Metallic'].default_value=.72 if role=='Iron' else 0
    # Use original 2K/4K identity maps directly as references, without editing them.
    manifest=Path(r'J:\Lonemoore_Regional_Identity\manifest.json')
    if manifest.exists() and role!='Iron':
        source=json.loads(manifest.read_text());r={'Wall':'wall','Floor':'floor','Vault':'ceiling','Trim':'trim'}[role]
        entry=next((a for a in source['materials'] if a.get('region')=='Crypts' and a['role']==r),None)
        if entry is None:entry=next((a for a in source['materials'] if 'Crypts' in a['id'] and a['role']==r),None)
        if entry:
            for key,input_name in [('base_color','Base Color'),('roughness','Roughness')]:
                if key in entry['maps']:
                    node=m.node_tree.nodes.new('ShaderNodeTexImage');node.image=bpy.data.images.load(str(manifest.parent/entry['maps'][key]),check_existing=True)
                    node.image.colorspace_settings.name='sRGB' if key=='base_color' else 'Non-Color';m.node_tree.links.new(node.outputs[0],bs.inputs[input_name])
    return m

def export_fbx(o,name):
    # Unreal's FBX coordinate conversion reflects Blender Y. Reflect only an
    # export copy (and reverse its winding) so the imported mesh exactly matches
    # the native N=-Y contract. Editable scenes and runtime transforms stay normal.
    bpy.ops.object.select_all(action='DESELECT')
    export_mesh=o.data.copy();export_mesh.name=name+'_ExportBasis'
    export_mesh.transform(Matrix.Diagonal((1,-1,1,1)));export_mesh.flip_normals();export_mesh.update()
    export_object=bpy.data.objects.new(name+'_Export',export_mesh);bpy.context.collection.objects.link(export_object)
    export_object.select_set(True);bpy.context.view_layer.objects.active=export_object
    bpy.ops.export_scene.fbx(filepath=str(OUT/'Meshes'/(name+'.fbx')),use_selection=True,object_types={'MESH'},apply_unit_scale=True,global_scale=1,axis_forward='-Y',axis_up='Z',use_mesh_modifiers=True,mesh_smooth_type='FACE',use_triangles=True,bake_anim=False,add_leaf_bones=False)
    bpy.data.objects.remove(export_object,do_unlink=True);bpy.data.meshes.remove(export_mesh)
    o.select_set(True);bpy.context.view_layer.objects.active=o

def save_mesh(name,t=None):
    global FACES,MATS,UVS
    # Remove both internal faces where adjacent structural trims meet exactly.
    # This is geometry cleanup, not a depth bias hiding overlapping geometry.
    signatures={}
    for index,f in enumerate(FACES):
        key=tuple(sorted(tuple(round(c,6) for c in VERTS[i]) for i in f));signatures.setdefault(key,[]).append(index)
    discard={i for indices in signatures.values() if len(indices)>1 for i in indices}
    FACES=[f for i,f in enumerate(FACES) if i not in discard]
    MATS=[m for i,m in enumerate(MATS) if i not in discard]
    UVS=[uv for i,uv in enumerate(UVS) if i not in discard]
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(VERTS,[],FACES);mesh.update()
    o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o)
    for role in ROLES:mesh.materials.append(preview_material(role))
    uv=mesh.uv_layers.new(name='MetricUV_2m')
    for polygon,mi,coords in zip(mesh.polygons,MATS,UVS):
        polygon.material_index=mi
        for li,co in zip(polygon.loop_indices,coords):uv.data[li].uv=co
        if mi==ROLES.index('Vault'):assert polygon.normal.z<-.01,(name,'upward-facing ceiling',polygon.index)
        if mi==ROLES.index('Floor'):assert polygon.normal.z>.01,(name,'downward-facing floor',polygon.index)
    # Keep editable architectural surfaces; triangulate only on FBX export.
    bpy.context.view_layer.objects.active=o;o.select_set(True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Scenes'/(name+'.blend')))
    export_fbx(o,name)
    signatures=set();duplicates=0;degenerate=0
    for f in FACES:
        sig=tuple(sorted(tuple(round(c,6) for c in VERTS[i]) for i in f))
        if sig in signatures:duplicates+=1
        signatures.add(sig)
        pts=[Vector(VERTS[i]) for i in f]
        if sum(((pts[i]-pts[0]).cross(pts[i+1]-pts[0])).length for i in range(1,len(pts)-1))<1e-9:degenerate+=1
    report=dict(id=t['id'] if t else 'portal_cap',vertices=len(VERTS),polygons=len(FACES),triangles=sum(len(f)-2 for f in FACES),internalFacesRemoved=len(discard),duplicateFaces=duplicates,degenerateFaces=degenerate,ceilingFacesDown=True,floorFacesUp=True,uvLayer='MetricUV_2m',materialSlots=ROLES,unitScale=1,fbx=str(OUT/'Meshes'/(name+'.fbx')))
    if t:
        ports=[]
        for bit,axis,sign in [(1,1,-1),(2,0,1),(4,1,1),(8,0,-1)]:
            if not t['socketMask']&bit:continue
            for along in (-2,-1,0,1,2):
                xy=[along,along];xy[axis]=sign*14;z=ceiling(t,*xy)
                assert any(abs(v[0]-xy[0])<1e-4 and abs(v[1]-xy[1])<1e-4 and abs(v[2]-z)<1e-4 for v in VERTS),(name,bit,along,'missing matching vault boundary vertex')
                ports.append(dict(socket=bit,lateralCm=along*100,heightCm=round(z*100,4)))
        report.update(walkableCells=sum(r.count('.') for r in t['rows']),sockets=t['socketMask'],style=t['style'],socketRoofVertices=ports)
    return o,report

def reset():
    global VERTS,FACES,MATS,UVS
    VERTS=[];FACES=[];MATS=[];UVS=[]
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    # Stable material role names survive full and partial Blender rebuilds.
    for mesh in list(bpy.data.meshes):
        if mesh.users==0:bpy.data.meshes.remove(mesh)
    for material in list(bpy.data.materials):
        if material.users==0:bpy.data.materials.remove(material)
    scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1

def render_preview(t,obj):
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24
    scene.cycles.use_denoising=True
    try:
        settings=bpy.context.preferences.addons['cycles'].preferences;settings.compute_device_type='OPTIX';settings.get_devices()
        for device in settings.devices:device.use=device.type!='CPU'
        scene.cycles.device='GPU'
    except Exception:pass
    scene.render.resolution_x=960;scene.render.resolution_y=640;scene.render.resolution_percentage=100
    scene.world.color=(.09,.10,.13);scene.view_settings.view_transform='AgX';scene.view_settings.look='AgX - Medium High Contrast';scene.view_settings.exposure=1.1
    bpy.ops.object.camera_add(location=(0,-11.6,2.2));cam=bpy.context.object
    target=Vector((2.4,0,2.5)) if t['style']=='turn' else Vector((0,2,2.7))
    cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.lens=21;scene.camera=cam
    lights=[((0,-10,3.3),1800,(1,.67,.32),1.3),((0,4,4.2),2000,(.39,.55,.75),2.0)]
    lights += [(tuple(v/100 for v in m['position']),600,(1,.48,.12),.24) for m in t['lights']]
    for position,power,color,radius in lights:
        bpy.ops.object.light_add(type='POINT',location=position);light=bpy.context.object;light.data.energy=power;light.data.color=color;light.data.shadow_soft_size=radius
    scene.render.filepath=str(OUT/'Renders'/(t['id']+'.png'))
    if '--render-only' not in sys.argv:
        bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Scenes'/('SM_RK_'+t['id']+'.blend')))
    bpy.ops.render.render(write_still=True)

reports=[]
selected=set(next((a.split('=',1)[1] for a in sys.argv if a.startswith('--only=')), '').split(','))-{''}
if '--audit-only' in sys.argv:
    audit=[]
    for t in TEMPLATES:
        name='SM_RK_'+t['id'];bpy.ops.wm.open_mainfile(filepath=str(OUT/'Scenes'/(name+'.blend')))
        mesh=bpy.data.objects[name].data;roof=[p for p in mesh.polygons if p.material_index==2];floor=[p for p in mesh.polygons if p.material_index==1]
        bad_roof=sum(p.normal.z>=-.01 for p in roof);bad_floor=sum(p.normal.z<=.01 for p in floor)
        audit.append(dict(id=t['id'],ceilingPolygons=len(roof),floorPolygons=len(floor),incorrectCeilingNormals=bad_roof,incorrectFloorNormals=bad_floor))
    (OUT/'surface_orientation_audit.json').write_text(json.dumps(dict(rooms=audit,failures=sum(a['incorrectCeilingNormals']+a['incorrectFloorNormals'] for a in audit)),indent=2))
    assert all(a['incorrectCeilingNormals']==0 and a['incorrectFloorNormals']==0 for a in audit)
    print('ROOM_KIT_ORIENTATION_AUDIT_COMPLETE rooms=25 failures=0');sys.exit(0)
for t in TEMPLATES:
    if selected and t['id'] not in selected:continue
    if '--export-only' in sys.argv:
        name='SM_RK_'+t['id'];bpy.ops.wm.open_mainfile(filepath=str(OUT/'Scenes'/(name+'.blend')))
        export_fbx(bpy.data.objects[name],name);continue
    if '--render-only' in sys.argv:
        bpy.ops.wm.open_mainfile(filepath=str(OUT/'Scenes'/('SM_RK_'+t['id']+'.blend')))
        render_preview(t,bpy.data.objects['SM_RK_'+t['id']]);continue
    reset();floors_and_vault(t);ornament(t)
    obj,report=save_mesh('SM_RK_'+t['id'],t);reports.append(report)
    print('ROOM_KIT_MESH',t['id'],report['triangles'],flush=True)
    if '--render' in sys.argv:render_preview(t,obj)
if '--render-only' in sys.argv:
    print('ROOM_KIT_RENDERS_COMPLETE rooms=25');sys.exit(0)
if '--export-only' in sys.argv:
    if not selected:
        name='SM_RK_portal_cap';bpy.ops.wm.open_mainfile(filepath=str(OUT/'Scenes'/(name+'.blend')));export_fbx(bpy.data.objects[name],name)
    print('ROOM_KIT_EXPORT_COMPLETE meshes=26 correctedUEHandedness=True');sys.exit(0)
if not selected:
    reset();box((0,0,3),(4,.36,6),'Wall');box((0,-.205,.17),(4,.06,.34),'Trim');box((0,.205,.17),(4,.06,.34),'Trim')
    _,report=save_mesh('SM_RK_portal_cap');reports.append(report)
else:
    previous=json.loads((OUT/'validation.json').read_text())['meshes']
    updated={r['id']:r for r in reports};reports=[updated.get(r['id'],r) for r in previous]
(OUT/'validation.json').write_text(json.dumps(dict(meshes=reports,roomCount=25,notes=['Authored source dimensions in metres, FBX automatic centimetre conversion.','UV0 is metric 2m tiling on every structural surface.','Floors own disjoint cell surfaces, portal caps are runtime edge-owned.','Navigation masks are conservative tile contracts; exact physical boundaries are authored.','No original artwork or player save is modified.']),indent=2))
assert all(r['duplicateFaces']==0 and r['degenerateFaces']==0 for r in reports)
print('ROOM_KIT_BLENDER_COMPLETE rooms=25 meshes=26')
