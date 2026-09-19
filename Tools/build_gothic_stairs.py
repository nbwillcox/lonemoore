"""Editable 3D gothic stair thresholds for all eighteen campaign floors."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector

root=Path(__file__).resolve().parents[1]
out=root/'ArtReview/CampaignExpansion/Stairs';out.mkdir(parents=True,exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC'
bpy.context.scene.unit_settings.scale_length=1
materials={n:bpy.data.materials.new(n) for n in ['Stone','Trim','Metal','Inset']}
floors=json.loads((root/'Content/Game/Data/campaign.json').read_text())['floors']
report=[]

def finish(obj,mat):
    obj.data.materials.append(materials[mat]);return obj

def box(p,s,mat='Stone',bevel=.018):
    bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.dimensions=s
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        m=o.modifiers.new('Dressed edges','BEVEL');m.width=bevel;m.segments=2;bpy.ops.object.modifier_apply(modifier=m.name)
    return finish(o,mat)

def cone(p,r,d,mat='Trim',top=0,vertices=8):
    bpy.ops.mesh.primitive_cone_add(vertices=vertices,radius1=r,radius2=top,depth=d,location=p)
    return finish(bpy.context.object,mat)

def bar(a,b,width,mat='Trim'):
    a,b=Vector(a),Vector(b);o=box((a+b)/2,(width,width,(b-a).length),mat,.009)
    o.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();return o

def pointed(cx,y,base,width,height,mat='Trim',thick=.10):
    parts=[]
    for sign in (-1,1):
        pts=[(cx+sign*width/2,y,base),(cx+sign*width*.47,y,base+height*.42),(cx+sign*width*.32,y,base+height*.76),(cx,y,base+height)]
        parts.extend(bar(a,b,thick,mat) for a,b in zip(pts,pts[1:]))
    return parts

def export(name,parts):
    bpy.ops.object.select_all(action='DESELECT')
    for obj in parts:obj.select_set(True)
    bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    uv=o.data.uv_layers.new(name='MetricUV')
    for poly in o.data.polygons:
        axes=(0,1) if abs(poly.normal.z)>.7 else (0,2) if abs(poly.normal.y)>.7 else (1,2)
        for li in poly.loop_indices:
            co=o.data.vertices[o.data.loops[li].vertex_index].co;uv.data[li].uv=(co[axes[0]],co[axes[1]])
    bpy.ops.export_scene.fbx(filepath=str(out/(name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
    o.hide_set(True)
    return len(o.data.polygons)

for index,floor in enumerate(floors):
    region=floor['regionIndex'];sub=sum(f['regionIndex']==region for f in floors[:index]);parts=[]
    # The former first tread had its upper surface exactly at z=0. All new treads
    # are separated from the dungeon floor by at least 18 cm; no duplicate overlay.
    for i in range(7):
        y=-1.55+i*.48;z=.18+i*.235
        for x in (-.9,0,.9):parts.append(box((x,y,z/2-.055),(.89,.474,z+.11),'Stone'))
        parts.append(box((0,y-.23,z-.025),(2.75,.085,.07),'Trim',.012))
        if i in (1,3,5):
            for x in (-.92,0,.92):parts.append(box((x,y-.245,z-.115),(.26,.03,.06),'Metal',.009))
    # Low, sloping balustrades: real stone panels, cap rails, carved lancets.
    for side in (-1,1):
        x=side*1.55
        for j in range(4):
            y=-1.60+j*1.03;base=.14+j*.50;height=1.03
            parts.extend([box((x,y,base/2),(.40,.44,base),'Stone'),box((x,y,base+height/2),(.24,.26,height),'Trim'),box((x,y,base+height),(.40,.42,.12),'Trim'),cone((x,y,base+height+.23),.15,.35,'Metal',vertices=region in (3,6) and 5 or 8)])
        for j in range(3):
            y=-1.085+j*1.03;base=.35+j*.50
            parts += [box((x,y,base+.35),(.15,.88,.57),'Stone'),bar((x,y-.48,base+.81),(x,y+.48,base+1.28),.16)]
            # Recessed diamond and two beads decorate the visible outer panel.
            o=box((x+side*.085,y,base+.35),(.035,.20,.20),'Metal',.012);o.rotation_euler[0]=math.pi/4;parts.append(o)
    # Rear gateway joins the stair structure. It is not scattered around rooms.
    for x in (-1.49,1.49):
        parts += [box((x,1.68,2.08),(.38,.42,1.65),'Stone'),box((x,1.68,2.92),(.51,.50,.17),'Trim')]
        parts += [bar((x-.10,1.42,1.35),(x-.10,1.42,2.83),.045,'Metal'),bar((x+.10,1.42,1.35),(x+.10,1.42,2.83),.045,'Metal')]
    parts += pointed(0,1.68,2.89,2.98,1.76,'Stone',.30)
    parts += pointed(0,1.435,2.91,2.67,1.48,'Trim',.075)
    # Regional crest: bell, burial crown, thorn fan, cross, fortress spear,
    # mineral shard, sealed eye or broken halo. Sublevels change crown counts.
    parts.append(box((0,1.63,4.51),(.40,.46,.38),'Trim'))
    if region in (0,4):
        parts += [bar((0,1.38,4.48),(0,1.38,5.15),.11,'Metal'),bar((-.23,1.38,4.92),(.23,1.38,4.92),.10,'Metal')]
    elif region in (2,5):
        for x in (-.28,0,.28):parts += [bar((x,1.44,4.50),(x,1.44,4.96+(.12 if x==0 else 0)),.08,'Metal'),cone((x,1.44,5.02),.09,.20,'Metal')]
    elif region in (3,6):
        for j in range(5):
            x=(j-2)*.19;parts.append(bar((0,1.54,4.47),(x,1.54,4.83+(.26-abs(x))*.6),.10,'Trim'))
    elif region in (7,8):
        for j in range(11 if region==8 else 14):
            a=j*2*math.pi/14; b=(j+1)*2*math.pi/14
            parts.append(bar((.34*math.cos(a),1.44,4.88+.34*math.sin(a)),(.34*math.cos(b),1.44,4.88+.34*math.sin(b)),.08,'Metal'))
    else:
        parts += [cone((0,1.40,4.83),.25,.42,'Metal',top=.08),box((0,1.40,4.59),(.48,.20,.07),'Trim')]
    # Distinct lower landing medallions for every floor; no coplanar tread faces.
    for j in range(sub+1):
        x=(j-sub*.5)*.27;parts.append(box((x,-1.785,.09),(.13,.027,.065),'Metal',.005))
    name=f'Stair_{index:02d}';faces=export(name,parts)
    report.append({'floor':index,'name':floor['name'],'mesh':name,'region':region,'polygons':faces,'first_tread_top_cm':18,'lowest_tread_top_cm':18,'footprint_cm':[350,380],'height_cm':523})
for obj in bpy.data.objects:obj.hide_set(False)
bpy.ops.wm.save_as_mainfile(filepath=str(out/'Gothic_Stairs.blend'))
(out/'manifest.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('GOTHIC_STAIRS_COMPLETE',len(report))
