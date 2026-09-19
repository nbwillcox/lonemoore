"""Read-only floor coverage and FBX round-trip audit. Never saves source assets."""
import bpy, hashlib, json, math, sys
from pathlib import Path
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT=Path(__file__).resolve().parents[1]
MANIFEST=json.loads((ROOT/'Content/Game/Data/room_kit.json').read_text())
OUT=ROOT/'Saved/RoomKitPass/floor_coverage.json'
ROUNDTRIP='--roundtrip' in sys.argv
OFFSETS=(-1.8,-1.35,-.9,-.45,0,.45,.9,1.35,1.8)

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()

def audit_scene(template,reflected=False):
    verts=[];tris=[];upward=0;downward=0;degenerate=0;areas=[];max_edge=0
    surface_z=[];material_names=[];corner_normal_z=[];corner_normal_lengths=[];tangent_signs=[];invalid_uvs=0;custom_normal_meshes=0;metric_uv_error=0
    for obj in bpy.context.scene.objects:
        if obj.type!='MESH':continue
        mesh=obj.data;mesh.calc_loop_triangles();base=len(verts)
        custom_normal_meshes+=bool(mesh.has_custom_normals)
        if mesh.uv_layers.active:mesh.calc_tangents(uvmap=mesh.uv_layers.active.name)
        verts += [obj.matrix_world@v.co for v in mesh.vertices]
        material_names += [m.name for m in mesh.materials if m]
        for tri in mesh.loop_triangles:
            polygon=mesh.polygons[tri.polygon_index]
            role=mesh.materials[polygon.material_index].name.split('.')[0]
            if role!='Floor':continue
            for loop_index in tri.loops:
                normal=(obj.matrix_world.to_3x3()@mesh.corner_normals[loop_index].vector)
                corner_normal_z.append(normal.z);corner_normal_lengths.append(normal.length)
                tangent_signs.append(mesh.loops[loop_index].bitangent_sign)
                if mesh.uv_layers.active:
                    uv=mesh.uv_layers.active.data[loop_index].uv
                    invalid_uvs+=not all(math.isfinite(c) for c in uv)
                    position=obj.matrix_world@mesh.vertices[mesh.loops[loop_index].vertex_index].co
                    expected=(position.x*.5,position.y*(-.5 if reflected else .5))
                    metric_uv_error=max(metric_uv_error,abs(uv.x-expected[0]),abs(uv.y-expected[1]))
            indices=tuple(base+i for i in tri.vertices);p=[verts[i] for i in indices]
            cross=(p[1]-p[0]).cross(p[2]-p[0]);area=cross.length*.5;areas.append(area)
            upward+=cross.z>1e-8;downward+=cross.z<-1e-8;degenerate+=area<=1e-8
            surface_z += [v.z for v in p]
            max_edge=max(max_edge,*[(p[(i+1)%3]-p[i]).length for i in range(3)])
            tris.append(indices)
    assert tris,(template['id'],'missing floor material triangles')
    tree=BVHTree.FromPolygons(verts,tris,all_triangles=True)
    center_missing=[];interior_missing=[];lane_missing=[];samples=0;wrong_z=[]
    def sample(x,y,group,cell):
        nonlocal samples
        if reflected:y=-y
        hit,normal,index,distance=tree.ray_cast(Vector((x,y,1)),Vector((0,0,-1)),2)
        samples+=1
        if hit is None:group.append(dict(cell=cell,xy=[x,y]))
        elif abs(hit.z)>.002:wrong_z.append(dict(cell=cell,z=hit.z))
    rows=template['rows']
    for y,row in enumerate(rows):
        for x,ch in enumerate(row):
            if ch!='.':continue
            cx=(x-3)*4;cy=(y-3)*4
            sample(cx,cy,center_missing,[x,y])
            for dx in OFFSETS:
                for dy in OFFSETS:sample(cx+dx,cy+dy,interior_missing,[x,y])
            for dx,dy in [(1,0),(0,1)]:
                nx,ny=x+dx,y+dy
                if nx>=7 or ny>=7 or rows[ny][nx]!='.':continue
                for step in range(1,16):
                    for lateral in (-.30,0,.30):sample(cx+dx*step/4-dy*lateral,cy+dy*step/4+dx*lateral,lane_missing,[x,y])
    return dict(floorTriangles=len(tris),upwardTriangles=upward,downwardTriangles=downward,degenerateTriangles=degenerate,
        floorAreaM2=sum(areas),largestTriangleM2=max(areas),longestTriangleEdgeM=max_edge,
        floorZMin=min(surface_z),floorZMax=max(surface_z),materialNames=sorted(set(material_names)),samples=samples,
        cornerNormalZMin=min(corner_normal_z),cornerNormalZMax=max(corner_normal_z),cornerNormalLengthMin=min(corner_normal_lengths),
        downwardCornerNormals=sum(z<0 for z in corner_normal_z),horizontalCornerNormals=sum(abs(z)<.8 for z in corner_normal_z),
        customNormalMeshes=custom_normal_meshes,bitangentSigns=sorted(set(tangent_signs)),invalidUVs=invalid_uvs,metricUvMaxError=metric_uv_error,
        centerMissing=center_missing,footprintSampleMissing=interior_missing,laneMissing=lane_missing,wrongZ=wrong_z)

results=[]
for template in MANIFEST['templates']:
    name='SM_RK_'+template['id'];source=ROOT/'ArtSource/RoomKit/Scenes'/(name+'.blend');fbx=ROOT/'ArtSource/RoomKit/Meshes'/(name+'.fbx')
    before=sha(source);fbx_before=sha(fbx)
    bpy.ops.wm.open_mainfile(filepath=str(source),load_ui=False)
    report=dict(id=template['id'],source=audit_scene(template),sourceSha256=before,fbxSha256=fbx_before)
    if ROUNDTRIP:
        bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
        bpy.ops.import_scene.fbx(filepath=str(fbx),use_custom_normals=True)
        report['fbx']=audit_scene(template,reflected=True)
    report['unchanged']=before==sha(source) and fbx_before==sha(fbx)
    results.append(report)
    print('FLOOR_COVERAGE',template['id'],'centerMissing',len(report['source']['centerMissing']),'denseMissing',len(report['source']['footprintSampleMissing']),'laneMissing',len(report['source']['laneMissing']),flush=True)
report=dict(rooms=results,roundTrip=ROUNDTRIP,roomCount=len(results),notes=[
 'Dense samples cover a 3.6m square within each nominal 4m navigation cell, so curved physical wall cut-offs can produce corner misses outside their true room shell.',
 'Centre and 60cm-wide route-strip coverage are reported independently from full cell footprint samples.',
 'Export copies intentionally reflect Y for Unreal. FBX coverage queries therefore reflect Y without changing the imported geometry.',
 'This audit checks source and round-trip geometry; it does not test Unreal Nanite cluster culling or material rendering.' ])
failures=[]
for room in results:
    if not room['unchanged']:failures.append([room['id'],'audit modified an asset'])
    for kind in ('source','fbx') if ROUNDTRIP else ('source',):
        data=room[kind]
        for field in ('centerMissing','laneMissing','wrongZ','downwardTriangles','degenerateTriangles','downwardCornerNormals','horizontalCornerNormals','invalidUVs'):
            if data[field]:failures.append([room['id'],kind,field])
        if data['metricUvMaxError']>1e-5:failures.append([room['id'],kind,'metric UV phase'])
        if room['id'] in ('circular_ossuary','rounded_turn','cavern_hall') and data['longestTriangleEdgeM']>1.8:
            failures.append([room['id'],kind,'room-sized curved floor triangle'])
    if ROUNDTRIP and abs(room['source']['floorAreaM2']-room['fbx']['floorAreaM2'])>.001:
        failures.append([room['id'],'FBX changed floor area'])
report.update(failures=failures,passed=not failures)
OUT.write_text(json.dumps(report,indent=2)+'\n')
assert not failures,failures
print('ROOM_KIT_FLOOR_COVERAGE_COMPLETE rooms=25 roundTrip='+str(ROUNDTRIP)+' passed=True')
