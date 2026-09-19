"""Wall-mounted sewer furnishings; metres, authored material slots, original sources."""
import bpy, math
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'ArtReview/ExpansionPrototype/AtmosphereRevision';out.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC'
mats={n:bpy.data.materials.new(n) for n in ('Oak','Iron','Stone')}
def finish(o,mat):
    o.data.materials.append(mats[mat]);return o
def box(p,s,mat='Oak'):
    bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.dimensions=s;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    m=o.modifiers.new('Worn edges','BEVEL');m.width=.018;m.segments=2;bpy.ops.object.modifier_apply(modifier=m.name);return finish(o,mat)
def cyl(p,r,d,mat='Iron',rotation=None):
    bpy.ops.mesh.primitive_cylinder_add(vertices=16,radius=r,depth=d,location=p);o=bpy.context.object
    if rotation:o.rotation_euler=rotation
    return finish(o,mat)
def crate(x,y,z,scale=.55):
    return [box((x+(i-2)*scale/5,y,z+scale/2),(scale/5-.012,scale,scale)) for i in range(5)]+[box((x,y+scale/2+.01,z+h),(scale,.045,.065),'Iron') for h in (.12,scale-.12)]
def barrel(x,y,z):
    return [cyl((x,y,z+.36),.29,.72,'Oak')]+[cyl((x,y,z+h),.308,.07) for h in (.10,.61)]
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
    bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
parts=[box((x,0,1.1),(.16,.75,2.2)) for x in (-1.25,1.25)]+[box((0,0,z),(2.65,.8,.12)) for z in (.10,1.08,2.14)]
parts+=crate(-.83,.03,.16)+crate(-.13,.03,.16)+barrel(.78,0,.16)+crate(.7,0,1.14,.62)+barrel(-.73,0,1.14)
export('WallStores',parts)
parts=[box((0,-.06,.15),(2.6,.85,.3),'Stone'),box((0,-.12,1.5),(2.45,.2,2.7),'Iron')]
for x in (-.78,.78):
    parts += [cyl((x,.10,1.35),.22,2.4)]+[cyl((x,.10,z),.3,.12) for z in (.38,1.9,2.5)]
    parts += [cyl((x,.34,1.25),.42,.1,rotation=(math.pi/2,0,0)),cyl((x,.40,1.25),.10,.15,rotation=(math.pi/2,0,0))]
    for i in range(8):
        a=i*math.pi/4;o=box((x+.4*math.cos(a),.42,1.25+.4*math.sin(a)),(.2,.12,.08),'Iron');o.rotation_euler[1]=-a;parts.append(o)
parts += [box((0,.16,.52),(2.35,.24,.16),'Iron'),box((0,.15,2.15),(2.35,.24,.16),'Iron')]
export('PumpStation',parts)
parts=[]
for i,x in enumerate((-1.1,-.35,.4,1.15)):
    z=1.4+(i%2)*.25
    parts += [cyl((x,.02,z),.16,2*z)]+[cyl((x,.02,h),.215,.10) for h in (.28,z,2*z-.2)]
parts += [box((0,-.17,z),(2.9,.16,.18),'Iron') for z in (.40,2.25)]
export('PipeBank',parts)
bpy.ops.wm.save_as_mainfile(filepath=str(out/'Sewer_Furnishings.blend'))
print('ATMOSPHERE_KIT_COMPLETE 3 assets')
