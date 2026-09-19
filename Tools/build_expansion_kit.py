"""Original modular sewer additions. Metres, editable Blender source and FBX."""
import bpy, math
from pathlib import Path
root=Path(__file__).resolve().parents[1]
out=root/'ArtReview/ExpansionPrototype';out.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC'
def box(p,s):
    bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.dimensions=s
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    m=o.modifiers.new('Worn edges','BEVEL');m.width=.025;m.segments=2
    bpy.ops.object.modifier_apply(modifier=m.name);return o
def cyl(p,r,d):
    bpy.ops.mesh.primitive_cylinder_add(vertices=16,radius=r,depth=d,location=p);return bpy.context.object
def export(name,parts):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    uv=o.data.uv_layers.new(name='MetricUV')
    for poly in o.data.polygons:
        axes=(0,1) if abs(poly.normal.z)>.7 else (0,2) if abs(poly.normal.y)>.7 else (1,2)
        for li in poly.loop_indices:
            co=o.data.vertices[o.data.loops[li].vertex_index].co;uv.data[li].uv=(co[axes[0]]/2,co[axes[1]]/2)
    bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'})
export('BridgeRail',[box((x,0,.55),(.16,.22,1.1)) for x in (-1.9,0,1.9)]+[box((0,0,z),(4,.18,.14)) for z in (.35,1.05)]+[box((0,0,.06),(4,.4,.12))])
export('ReservoirPier',[box((0,0,-.5),(1.8,1.8,1)),box((0,0,4.7),(.95,.95,9.4)),box((0,0,9.5),(1.8,1.8,.35))]+[box((0,0,z),(1.2,1.2,.2)) for z in (1.8,3.6,5.4,7.2)])
parts=[]
for i in range(11):
    a=math.pi*(i+.5)/14;o=box((3*math.cos(a),0,2.5+3*math.sin(a)),(.7,.9,.65));o.rotation_euler[1]=-a+math.pi/2;parts.append(o)
parts.extend([box((2.85,0,1.2),(.9,1.1,2.4)),box((-2.3,-.6,.23),(1.2,.8,.45)),box((-1.2,.4,.3),(.8,.9,.6))])
export('BrokenAqueduct',parts)
parts=[box((0,0,.15),(2.1,1.4,.3)),box((-.8,0,1.15),(.3,.6,2)),box((.8,0,1.15),(.3,.6,2))]
wheel=cyl((0,0,1.7),.85,.15);wheel.rotation_euler[0]=math.pi/2;parts.append(wheel)
for a in range(8):
    t=a*math.pi/4;o=box((.78*math.cos(t),-.15,1.7+.78*math.sin(t)),(.34,.22,.15));o.rotation_euler[1]=-t;parts.append(o)
export('PumpWheel',parts)
export('BridgeDeck',[box((0,0,-.22),(4,4,.44))]+[box((x,0,-.7),(.35,4,.5)) for x in (-1.6,1.6)])
bpy.ops.wm.save_as_mainfile(filepath=str(out/'Expansion_Sewer_Kit.blend'))
print('EXPANSION_KIT_COMPLETE 5 assets')
