"""Blender 5.2: metres, 4m grid, boundary pivot, editable sources and FBX exports."""
import bpy, math, pathlib, json
from mathutils import Vector
root=pathlib.Path(__file__).resolve().parents[1]
dest=root/'ArtReview/IntegrityKit';dest.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC'
bpy.context.scene.unit_settings.scale_length=1
def box(loc,size,bevel=.025):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object;o.dimensions=size;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        mod=o.modifiers.new('Soft dressed edges','BEVEL');mod.width=bevel;mod.segments=2
        bpy.ops.object.modifier_apply(modifier=mod.name)
    return o
def cylinder(loc,r,depth,vertices=12):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=depth,location=loc)
    return bpy.context.object
def export(name,parts):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.hide_set(False);o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name='I_'+name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    # UV projection uses actual metre positions: consistent density on every module.
    uv=o.data.uv_layers.new(name='MetricUV') if not o.data.uv_layers else o.data.uv_layers[0]
    for poly in o.data.polygons:
        axes=(0,1) if abs(poly.normal.z)>.7 else (0,2) if abs(poly.normal.y)>.7 else (1,2)
        for li in poly.loop_indices:
            co=o.data.vertices[o.data.loops[li].vertex_index].co;uv.data[li].uv=(co[axes[0]],co[axes[1]])
    bpy.ops.export_scene.fbx(filepath=str(dest/(o.name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
    o.hide_set(True)
def arch(radius=1.50,spring=2.8,depth=.65):
    parts=[box((-1.78,0,spring/2),(.44,depth,spring)),box((1.78,0,spring/2),(.44,depth,spring))]
    for i in range(15):
        a=math.pi*(i+.5)/15;o=box(((radius+.18)*math.cos(a),0,spring+(radius+.18)*math.sin(a)),(.39,depth,.40));o.rotation_euler[1]=-a+math.pi/2;parts.append(o)
    # Side masonry and sealed upper lintel leave a 3m doorway clear.
    parts+=[box((0,0,4.96),(4,depth,.50))]
    return parts
export('Floor',[box((-1.5+i, -1.5+j,-.1),(1.002,1.002,.20),.012) for i in range(4) for j in range(4)])
wall=[]
for z in range(10):
    for x in range(5):wall.append(box((-1.6+x*.8,0,.26+z*.52),(.80,.46,.52),.018))
wall += [box((0,-.04,.18),(4,.60,.36)),box((0,0,5.17),(4,.62,.16))]
export('Wall',wall)
niche=[box((0,.20,2.6),(4,.15,5.2)),box((-1.7,-.15,2.6),(.6,.65,5.2)),box((1.7,-.15,2.6),(.6,.65,5.2))]
for z in (.24,1.55,2.85,4.2,5.05):niche.append(box((0,-.18,z),(3,.70,.18)))
for z in (.58,1.90,3.20):
    niche += [box((0,-.10,z),(2.5,.4,.40),.08),box((0,-.23,z+.25),(2.65,.5,.12))]
export('Niche',niche)
export('Arch',arch())
vault=[]
for i in range(18):
    a=math.pi*(i+.5)/18;o=box((2.0*math.cos(a),0,4.65+1.2*math.sin(a)),(.4,4.04,.22),.015);o.rotation_euler[1]=-a+math.pi/2;vault.append(o)
export('Vault',vault)
export('Pillar',[box((0,0,.15),(.7,.7,.30)),cylinder((0,0,2.5),.23,4.6),box((0,0,4.85),(.8,.8,.4))])
export('Door',[box((0,0,2.25),(3.02,.30,4.5),.03)]+[box((0,-.19,z),(3.02,.1,.13)) for z in (.5,2.2,3.9)])
export('Grate',[box((x*.3,0,2.25),(.075,.22,4.5),.006) for x in range(-5,6)]+[box((0,0,z),(3.1,.25,.12)) for z in (.12,2.25,4.45)])
export('Tomb',[box((0,0,.30),(1.35,2.5,.60),.09),box((0,0,.66),(1.5,2.65,.18),.06),box((0,0,.80),(.3,1.7,.10))])
export('Statue',[box((0,0,.3),(1.4,1.4,.6)),cylinder((0,0,1.35),.44,1.5,8),box((0,0,2.25),(.85,.50,.55),.10),cylinder((0,0,2.90),.26,.48,10),box((-.43,0,1.8),(.23,.32,1.0)),box((.44,.1,2.0),(.22,.35,.6))])
export('Stairs',[box((0,-1.8+i*.5,.1+i*.13),(3.0,.52,.2+i*.26),.015) for i in range(8)])
export('Rubble',[box((math.sin(i*5)*.55,math.cos(i*3)*.45,.08+(i%3)*.1),(.35,.3,.2),.04) for i in range(12)])
export('Brazier',[cylinder((0,0,.65),.12,1.3),cylinder((0,0,1.30),.33,.22),box((0,0,.09),(.55,.55,.18))])
export('Pipe',[cylinder((0,0,1.5),.26,3.0)]+[cylinder((0,0,z),.33,.14) for z in (.15,1.5,2.85)])
export('Drain',[box((-1.84,0,-.14),(.28,4,.28)),box((1.84,0,-.14),(.28,4,.28))]+[box((x*.22,0,-.015),(.08,4,.07),.004) for x in range(-7,8)])
export('Mushrooms',[cylinder((math.sin(i*2)*.65,math.cos(i*3)*.6,.3+i*.11),.055,.6+i*.22,8) for i in range(5)]+[cylinder((math.sin(i*2)*.65,math.cos(i*3)*.6,.62+i*.22),.25+i*.02,.12,12) for i in range(5)])
export('Camp',[box((0,0,.05),(.85,1.8,.1)),box((.7,.6,.2),(.4,.55,.4)),box((-.6,-.5,.15),(.25,.35,.3))])
export('Altar',[box((0,0,.15),(1.6,1.0,.3)),box((0,0,.75),(1.25,.75,1.2)),box((0,0,1.40),(1.8,1.15,.18))])
export('Trim',[box((0,0,.09),(4,.35,.18)),box((0,0,4.8),(4,.35,.16))])
for o in bpy.data.objects:o.hide_set(False)
bpy.ops.wm.save_as_mainfile(filepath=str(dest/'Lonemoore_Integrity_Kit.blend'))
(dest/'kit_spec.json').write_text(json.dumps({'blender':bpy.app.version_string,'grid_metres':4,'door_clearance_metres':3,'wall_thickness_metres':.46,'boundary_pivot':[0,0,0],'modules':[o.name for o in bpy.data.objects]},indent=2))
print('INTEGRITY_KIT_EXPORTED',len(bpy.data.objects))
