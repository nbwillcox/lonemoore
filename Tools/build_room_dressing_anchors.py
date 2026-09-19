"""Read authored wall surfaces and emit bounded, ray-verified dressing anchors.

This does not save Blender scenes or change the room/navigation manifest.
"""
import bpy, json, math, hashlib
from pathlib import Path
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT=Path(__file__).resolve().parents[1]
MANIFEST=ROOT/'Content/Game/Data/room_kit.json'
DEST=ROOT/'Content/Game/Data/room_dressing_anchors.json'
REPORT=ROOT/'Saved/RoomKitPass/dressing_anchor_audit.json'
manifest=json.loads(MANIFEST.read_text());manifest_hash=hashlib.sha256(MANIFEST.read_bytes()).hexdigest()

def segment_distance(point,a,b):
    d=b-a;along=max(0,min(1,(point-a).dot(d)/d.length_squared));return (point-(a+d*along)).length

records=[];audits=[]
for template in manifest['templates']:
    path=ROOT/'ArtSource/RoomKit/Scenes'/('SM_RK_'+template['id']+'.blend')
    before=hashlib.sha256(path.read_bytes()).hexdigest()
    bpy.ops.wm.open_mainfile(filepath=str(path),load_ui=False)
    vertices=[];triangles=[];identities=[];roof_triangles=[];candidates=[]
    for obj in bpy.context.scene.objects:
        if obj.type!='MESH':continue
        mesh=obj.data;mesh.calc_loop_triangles();base=len(vertices);transform=obj.matrix_world
        vertices += [transform@v.co for v in mesh.vertices]
        for triangle in mesh.loop_triangles:
            polygon=mesh.polygons[triangle.polygon_index];role=mesh.materials[polygon.material_index].name.split('.')[0]
            indices=tuple(base+i for i in triangle.vertices);triangles.append(indices)
            identities.append(dict(role=role,polygon=polygon.index))
            if role=='Vault':roof_triangles.append(indices)
        for polygon in mesh.polygons:
            role=mesh.materials[polygon.material_index].name.split('.')[0]
            normal=(transform.to_3x3()@polygon.normal).normalized()
            if role!='Wall' or abs(normal.z)>.025:continue
            center=transform@polygon.center;center.z=1.4
            candidates.append(dict(point=center,normal=normal,polygon=polygon.index))
    tree=BVHTree.FromPolygons(vertices,triangles,all_triangles=True)
    roof=BVHTree.FromPolygons(vertices,roof_triangles,all_triangles=True)
    def covered(point):return roof.ray_cast(point,Vector((0,0,1)),30)[0] is not None
    def wall_hit(point,normal):
        hit,hit_normal,index,distance=tree.ray_cast(point+normal*.35,-normal,.8)
        if hit is None or (hit-point).length>.015 or identities[index]['role']!='Wall':return None
        if hit_normal.dot(normal)<.98:return None
        return hit
    portals=[]
    for bit,a,b in [(1,(-2,-14),(2,-14)),(2,(14,-2),(14,2)),(4,(-2,14),(2,14)),(8,(-14,-2),(-14,2))]:
        if template['socketMask']&bit:portals.append((Vector(a),Vector(b)))
    valid=[]
    for candidate in candidates:
        p=candidate['point'];n=candidate['normal']
        # True interior walls have roof on the room side and no roof outside.
        # This rejects outer backing surfaces and free-standing furnishings.
        if not covered(p+n*.35) or covered(p-n*.4):continue
        clearance=min(segment_distance(Vector((p.x,p.y)),a,b) for a,b in portals)
        if clearance<2.0:continue
        if wall_hit(p,n) is None:continue
        candidate['portalClearance']=clearance;valid.append(candidate)
    selected=[]
    while valid and len(selected)<8:
        if not selected:best=max(valid,key=lambda c:c['point'].x*c['point'].x+c['point'].y*c['point'].y)
        else:best=max(valid,key=lambda c:min((c['point']-s['point']).length for s in selected))
        if selected and min((best['point']-s['point']).length for s in selected)<3.0:break
        selected.append(best);valid.remove(best)
    selected.sort(key=lambda c:math.atan2(c['point'].y,c['point'].x))
    anchors=[]
    for index,c in enumerate(selected):
        p=c['point'].copy();p.z=(.90,1.25,1.65,1.90)[index%4];n=c['normal']
        assert wall_hit(p,n) is not None,(template['id'],'height-specific wall ray failed',index)
        anchors.append(dict(position=[round(v*100,3) for v in p],inwardNormal=[round(v,6) for v in n],
            clearanceToPortalCm=round(c['portalClearance']*100,3),sourcePolygon=c['polygon']))
    assert 4<=len(anchors)<=8,(template['id'],'insufficient safe anchors',len(anchors))
    unchanged=before==hashlib.sha256(path.read_bytes()).hexdigest();assert unchanged
    records.append(dict(id=template['id'],anchors=anchors))
    audits.append(dict(id=template['id'],count=len(anchors),sourceSha256=before,sourceUnchanged=unchanged,
        minimumPortalClearanceCm=min(a['clearanceToPortalCm'] for a in anchors),
        minimumSpacingCm=round(min((a['point']-b['point']).length for i,a in enumerate(selected) for b in selected[i+1:])*100,3),
        wallRayChecks=len(anchors),insideOutsideRoofChecks=len(anchors)))
    print('ROOM_DRESSING_ANCHORS',template['id'],len(anchors),flush=True)
assert hashlib.sha256(MANIFEST.read_bytes()).hexdigest()==manifest_hash
DEST.write_text(json.dumps(dict(version=1,units='cm',coordinateSystem='Native room coordinates; N=-Y, E=+X, floorZ=0',templates=records),indent=2)+'\n')
REPORT.write_text(json.dumps(dict(rooms=audits,roomCount=25,anchorCount=sum(r['count'] for r in audits),failures=0,
    roomManifestUnchanged=True,method='Actual inward wall faces; inside/outside authored vault coverage; inward-to-wall ray at final height; >=200cm from doorway segments and >=300cm spacing.'),indent=2)+'\n')
print('ROOM_DRESSING_ANCHORS_COMPLETE rooms=25 anchors='+str(sum(r['count'] for r in audits))+' failures=0')
