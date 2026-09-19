"""Blender-authored modular development geometry. Original art is never opened for writing."""
import bpy, math, pathlib
root=pathlib.Path(__file__).resolve().parents[1]
dest=root/'ArtReview/ModularKit'; dest.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
def box(name,loc,size,bevel=.03):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object;o.name=name;o.dimensions=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        m=o.modifiers.new('Worn edges','BEVEL');m.width=bevel;m.segments=2
        bpy.context.view_layer.objects.active=o;bpy.ops.object.modifier_apply(modifier=m.name)
    return o
def export(name,parts):
    bpy.ops.object.select_all(action='DESELECT')
    for o in parts:o.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join()
    o=bpy.context.object;o.name=name;bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.export_scene.fbx(filepath=str(dest/(name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
    o.hide_set(True)
export('FloorTile',[box('slab',(0,0,-.08),(3.98,3.98,.16),.015)])
parts=[]
for row in range(7):
    for col in range(5):
        parts.append(box('masonry',(-1.6+col*.8,0,.245+row*.49),(.785,.28,.475),.035))
parts += [box('base',(0,0,.12),(4,.38,.24)),box('cornice',(0,0,3.43),(4,.4,.18))]
export('StoneWall',parts)
parts=[box('pierL',(-1.72,0,1.4),(.56,.65,2.8)),box('pierR',(1.72,0,1.4),(.56,.65,2.8))]
for i in range(11):
    angle=math.pi*(i+.5)/11
    o=box('archstone',(1.42*math.cos(angle),0,2.1+1.42*math.sin(angle)),(.46,.65,.46),.035);o.rotation_euler[1]=angle;parts.append(o)
export('Arch',parts)
export('Pillar',[box('base',(0,0,.12),(.65,.65,.24)),box('shaft',(0,0,1.65),(.4,.4,3.05)),box('cap',(0,0,3.25),(.7,.7,.3))])
export('Door',[box('oak',(0,0,1.25),(2.55,.16,2.5),.03),box('brace',(0,-.11,.6),(2.55,.12,.1)),box('brace',(0,-.11,1.85),(2.55,.12,.1))])
export('Chest',[box('wood',(0,0,.32),(1.1,.68,.64),.07),box('lid',(0,0,.69),(1.15,.73,.16),.06),box('lock',(0,-.37,.53),(.15,.1,.22),.015)])
export('Stairs',[box('step',(0,-1.5+i*.5,-.2+i*.18),(3.1,.5,.4+i*.36),.015) for i in range(7)])
export('PressurePlate',[box('plate',(0,0,.015),(1.9,1.9,.06),.015)])
export('Lever',[box('plinth',(0,0,.25),(.55,.55,.5)),box('handle',(0,0,.78),(.09,.09,.7)),box('grip',(0,0,1.13),(.35,.15,.12))])
for o in bpy.data.objects:o.hide_set(False)
bpy.ops.wm.save_as_mainfile(filepath=str(dest/'Dungeon_Modular_Kit.blend'))
print('DUNGEON_KIT_COMPLETE',dest)
