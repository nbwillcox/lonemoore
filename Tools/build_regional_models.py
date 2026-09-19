"""Blender regional prop, interactable and showcase production. Uses only isolated outputs."""
import bpy,math,json,random,sys
from pathlib import Path
from mathutils import Vector
R=Path(r'J:\Lonemoore_Regional_Art');M=json.loads((R/'manifest.json').read_text());results=[]
selection=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
def reset():
 bpy.ops.wm.read_factory_settings(use_empty=True);s=bpy.context.scene;s.unit_settings.system='METRIC';s.render.engine='CYCLES';s.cycles.samples=24;s.cycles.use_denoising=True
 try:
  p=bpy.context.preferences.addons['cycles'].preferences;p.compute_device_type='OPTIX';p.get_devices()
  for d in p.devices:d.use=d.type=='OPTIX'
  s.cycles.device='GPU'
 except:pass
 s.render.resolution_x=1600;s.render.resolution_y=900;s.render.resolution_percentage=100;s.render.image_settings.file_format='PNG';s.world=bpy.data.worlds.new('Neutral atmosphere');s.world.use_nodes=True;s.world.node_tree.nodes['Background'].inputs[0].default_value=(.12,.15,.18,1);s.world.node_tree.nodes['Background'].inputs[1].default_value=.3;return s
def material(a):
 m=bpy.data.materials.new(a['id']);m.use_nodes=True;n=m.node_tree.nodes;l=m.node_tree.links;bs=n.get('Principled BSDF')
 for role,path in a['maps'].items():
  t=n.new('ShaderNodeTexImage');t.image=bpy.data.images.load(str(R/path),check_existing=True);t.image.colorspace_settings.name='sRGB' if role=='base_color' else 'Non-Color'
  if role=='normal':
   sep=n.new('ShaderNodeSeparateXYZ');l.new(t.outputs['Color'],sep.inputs[0]);inv=n.new('ShaderNodeMath');inv.operation='SUBTRACT';inv.inputs[0].default_value=1;l.new(sep.outputs[1],inv.inputs[1]);comb=n.new('ShaderNodeCombineXYZ');l.new(sep.outputs[0],comb.inputs[0]);l.new(inv.outputs[0],comb.inputs[1]);l.new(sep.outputs[2],comb.inputs[2]);nm=n.new('ShaderNodeNormalMap');nm.inputs['Strength'].default_value=.65;l.new(comb.outputs[0],nm.inputs['Color']);l.new(nm.outputs[0],bs.inputs['Normal'])
  elif role=='emission':
   tint=n.new('ShaderNodeMixRGB');tint.blend_type='MULTIPLY';tint.inputs[0].default_value=1;tint.inputs[2].default_value=(1,.20,.025,1);l.new(t.outputs['Color'],tint.inputs[1]);l.new(tint.outputs[0],bs.inputs['Emission Color']);bs.inputs['Emission Strength'].default_value=1.5
  else:l.new(t.outputs['Color'],bs.inputs[{'base_color':'Base Color','roughness':'Roughness','metallic':'Metallic'}[role]])
 return m
parts=[]
def finish(o,name,mat,bevel=0):
 o.name=name;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(mat)
 if bevel:
  q=o.modifiers.new('Worn edge bevel','BEVEL');q.width=bevel;q.segments=2;bpy.context.view_layer.objects.active=o;bpy.ops.object.modifier_apply(modifier=q.name)
 if not o.data.uv_layers:o.data.uv_layers.new()
 for p in o.data.polygons:
  axis=max(range(3),key=lambda a:abs(p.normal[a]));axes=[a for a in range(3) if a!=axis]
  for li in p.loop_indices:
   v=o.data.vertices[o.data.loops[li].vertex_index].co+o.location;o.data.uv_layers.active.data[li].uv=(v[axes[0]]/2,v[axes[1]]/2)
 parts.append(o);return o
def box(p,sz,mat,name='Dressed piece',b=.012):
 bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.dimensions=sz;return finish(o,name,mat,b)
def rod(a,b,r,mat,name='Rod',vertices=16):
 v=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=v.length,location=(Vector(a)+Vector(b))/2);o=bpy.context.object;o.rotation_euler=v.to_track_quat('Z','Y').to_euler();o=finish(o,name,mat,.004)
 for poly in o.data.polygons:
  if len(poly.vertices)==4:poly.use_smooth=True
 return o
def ring(p,r,t,mat,rot=(0,0,0),arc=2*math.pi,name='Ring'):
 verts=[];faces=[];steps=max(12,int(40*arc/(2*math.pi)))
 for i in range(steps+1):
  a=i/steps*arc
  for j in range(8):
   b=j/8*2*math.pi;verts.append(((r+t*math.cos(b))*math.cos(a),t*math.sin(b),(r+t*math.cos(b))*math.sin(a)))
 for i in range(steps):
  for j in range(8):a=i*8+j;b=i*8+(j+1)%8;faces.append((a,b,b+8,a+8))
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);o.location=p;o.rotation_euler=rot;o=finish(o,name,mat)
 for poly in o.data.polygons:poly.use_smooth=True
 return o
def rock(p,sz,mat,seed=1):
 bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=1,location=p);o=bpy.context.object;r=random.Random(seed)
 for v in o.data.vertices:v.co*=r.uniform(.80,1.18)
 o.scale=sz;return finish(o,'Fractured rock',mat)
def horn(a,b,r,mat):
 v=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cone_add(vertices=10,radius1=r,radius2=.005,depth=v.length,location=(Vector(a)+Vector(b))/2);o=bpy.context.object;o.rotation_euler=v.to_track_quat('Z','Y').to_euler();return finish(o,'Tapered spur',mat,.003)
def urn(mat):
 profile=[(0,.17),(.05,.22),(.13,.15),(.25,.27),(.45,.32),(.62,.24),(.69,.14),(.79,.15),(.82,.18),(.82,.12),(.76,.10),(.69,.10),(.61,.19)]
 verts=[(r*math.cos(i*2*math.pi/32),r*math.sin(i*2*math.pi/32),z) for z,r in profile for i in range(32)];faces=[]
 for j in range(len(profile)-1):
  for i in range(32):a=j*32+i;b=j*32+(i+1)%32;faces.append((a,b,b+32,a+32))
 mesh=bpy.data.meshes.new('Vessel lathe');mesh.from_pydata(verts,[],faces);o=bpy.data.objects.new('Hollow vessel',mesh);bpy.context.collection.objects.link(o);return finish(o,o.name,mat,.007)
def export(name,role,reg):
 bpy.ops.object.select_all(action='DESELECT')
 for o in parts:o.select_set(True)
 bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name;bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR');bpy.ops.object.transform_apply(location=False,rotation=True,scale=True);o.data.calc_loop_triangles()
 bpy.ops.export_scene.fbx(filepath=str(R/'props'/(name+'.fbx')),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
 results.append(dict(id=name,role=role,region=reg,fbx='props/'+name+'.fbx',size_m=list(o.dimensions),triangles=len(o.data.loop_triangles),pivot='ground centre; keys visual centre',materials=[m.name for m in o.data.materials]));parts.clear();return o
def aim(o,p):o.rotation_euler=(Vector(p)-o.location).to_track_quat('-Z','Y').to_euler()
def area(p,power,color,size,target=(0,0,.6)):
 bpy.ops.object.light_add(type='AREA',location=p);o=bpy.context.object;o.data.energy=power;o.data.color=color;o.data.size=size;aim(o,target);return o
for reg in M['regions']:
 code=reg['code'];idx=reg['index']
 if selection and code not in selection:continue
 print('REGION_MODELS',code,flush=True);S=reset();mats={a['role']:material(a) for a in M['materials'] if a['region']==idx};stone=mats[{0:'secondary',2:'accent',3:'trim',4:'detail',5:'wall',6:'trim',7:'accent',8:'secondary'}[idx]];metal=mats['metal'];accent=mats['accent'];detail=mats['detail'];main=mats['wall'];props=[]
 for k,id in enumerate(reg['props']):
  if code=='Cathedral':
   if k==0:
    box((0,0,.10),(.85,.48,.20),stone);box((0,.03,.30),(.72,.43,.20),detail);rock((.32,0,.39),(.19,.23,.10),stone,12)
   elif k==1:
    box((0,.10,.42),(.45,.12,.80),metal);ring((0,-.02,.52),.22,.035,metal)
    for i,z in enumerate([.18,.265,.35,.435]):ring((0,-.035,z),.055,.014,metal,rot=(0,0,(i%2)*math.pi/2))
   else:
    for xx in [-.37,.37]:box((xx,0,.22),(.085,.4,.44),accent)
    for yy in [-.14,0,.14]:box((0,yy,.48),(.95,.135,.055),accent)
    box((-.15,.19,.75),(.67,.06,.46),accent)
  elif code=='Catacombs':
   if k==0:
    for z in [.06,.39,.72]:box((0,.1,z),(.9,.42,.09),detail)
    for xx in [-.43,.43]:box((xx,.10,.39),(.08,.40,.75),detail)
    for z in [.18,.52]:
     for i in range(5):
      xx=-.30+i*.15;rod((xx,-.07,z),(xx+.04,.18,z+.03),.035,accent,'Long bone');rock((xx,-.08,z),(.058,.048,.05),accent,i)
   elif k==1:
    box((0,0,.43),(.65,.15,.86),main);ring((0,-.09,.57),.18,.026,stone)
    for xx in [-.11,0,.11]:horn((xx,-.10,.56),(xx,-.10,.78),.035,stone)
   else:urn(stone)
  elif code=='Warrens':
   if k==0:
    for i in range(6):o=box((-.35+i*.12,0,.11+i%2*.05),(.10,.75,.14),accent if False else mats['secondary']);o.rotation_euler[2]=(i-3)*.08
    for yy in [-.22,.22]:box((0,yy,.26),(.86,.055,.035),metal)
   elif k==1:
    box((0,0,.10),(.75,.48,.15),mats['secondary'])
    for i in range(7):rock((-.25+i*.08,math.sin(i)*.13,.22),(.065,.065,.035),stone,i)
   else:
    for i in range(9):
     xx=math.sin(i*4)*.35;yy=math.cos(i*4)*.2;rod((xx,yy,0),(xx*.6,yy*.5,.70+i%3*.12),.026,mats['secondary']);horn((xx*.8,yy*.8,.35),(xx+.18,yy,.55),.035,mats['secondary'])
  elif code=='Crypts':
   if k==0:
    box((0,.06,.4),(.75,.18,.8),detail);box((0,-.045,.46),(.60,.03,.40),metal)
    for i in range(5):o=box((0,-.069,.33+i*.06),(.46,.009,.012),stone,b=.002);o.rotation_euler[1]=.1
   elif k==1:
    box((0,0,.15),(.75,.75,.3),detail);box((0,0,.34),(.82,.80,.12),stone)
    for xx in [-.28,.28]:rod((xx,-.34,.14),(xx,-.34,.36),.028,metal)
   else:
    box((0,0,.06),(.45,.4,.12),metal);rod((0,0,.1),(0,0,.65),.042,metal)
    for xx in [-.22,0,.22]:rod((0,0,.52),(xx,0,.7),.02,metal);rod((xx,0,.70),(xx,0,.87),.042,stone)
  elif code=='Fortress':
   if k==0:
    box((0,.1,.42),(.45,.09,.8),metal)
    for xx in [-.15,.15]:
     for i,z in enumerate([.3,.38,.46,.54]):ring((xx,0,z),.052,.014,metal,rot=(0,0,(i%2)*math.pi/2))
     ring((xx,-.02,.13),.10,.025,metal,arc=5.2)
   elif k==1:
    for xx in [-.36,.36]:box((xx,.1,.46),(.085,.2,.92),accent)
    box((0,.1,.63),(.85,.09,.1),accent);o=box((0,-.04,.47),(.52,.10,.65),metal,b=.04);o.rotation_euler[1]=.14
    box((0,-.11,.47),(.08,.025,.70),stone)
   else:
    box((0,0,.13),(.55,.4,.26),main);ring((0,-.02,.48),.26,.055,metal)
    for i,z in enumerate([.30,.39,.48,.57,.66]):ring((0,-.11,z),.058,.015,metal,rot=(0,0,(i%2)*math.pi/2))
  elif code=='Deep':
   if k==0:
    rock((0,0,.12),(.4,.28,.13),main,2)
    for i in range(9):horn((math.sin(i)*.22,math.cos(i)*.16,.15),(math.sin(i)*.32,math.cos(i)*.25,.45+i%3*.20),.085,stone)
   elif k==1:
    for i in range(7):o=rock((math.sin(i)*.28,math.cos(i)*.22,.08+i%2*.04),(.16,.11,.028),accent,i+4);o.rotation_euler[1]=i*.14
   else:rock((0,0,.6),(.30,.25,.6),mats['secondary'],27);rock((.19,.1,.19),(.19,.24,.21),main,11)
  elif code=='Infernal':
   if k==0:
    rock((0,0,.16),(.44,.30,.18),stone,3);ring((0,-.07,.48),.30,.055,metal,arc=4.6)
   elif k==1:
    box((0,0,.09),(.65,.5,.18),main)
    for xx in [-.22,.22]:rod((xx,0,.16),(xx,0,.60),.038,metal);horn((xx,0,.60),(xx*.2,0,.74),.055,metal)
    ring((0,0,.48),.22,.04,metal,arc=4.9)
   else:urn(metal);rock((0,0,.63),(.16,.16,.05),accent,1)
  elif code=='Hell':
   if k==0:
    for i in range(8):rock((math.sin(i)*.29,math.cos(i)*.22,.12+i%3*.06),(.18,.16,.14),accent,i+35)
   elif k==1:
    box((0,0,.16),(.65,.52,.32),main);ring((0,-.05,.49),.27,.06,metal)
    for i in range(4):ring((0,-.14,.45+i*.12),.082,.019,metal,rot=(0,0,(i%2)*math.pi/2))
   else:
    box((0,0,.10),(.55,.4,.2),main);ring((0,0,.65),.46,.055,stone,arc=4.65)
    for i in range(7):a=i*.7;horn((.46*math.cos(a),0,.65+.46*math.sin(a)),(.60*math.cos(a),0,.65+.60*math.sin(a)),.032,metal)
  props.append(export('SM_'+id,'decoration',idx))
 # Shared mechanism with regional silhouette and material construction.
 with bpy.data.libraries.load(r'J:\First Person Dungeon Crawler Game\ArtReview\SharedProps\Shared_Interactables.blend',link=False) as (src,dst):dst.objects=['SM_SharedLever']
 lever=dst.objects[0];bpy.context.collection.objects.link(lever);lever.location=(0,0,0)
 for i,slot in enumerate(lever.material_slots):slot.material=metal if slot.material.name.startswith('Iron') else (mats['secondary'] if idx==3 else stone)
 parts.append(lever)
 if idx>=7:
  for xx in [-.23,.23]:horn((xx,0,.16),(xx*1.5,0,.58 if idx==7 else .78),.065,metal)
  ring((0,.20,1.05),.18,.025,metal,arc=4.7 if idx==8 else 6.28)
 elif idx in [2,4]:ring((0,-.035,.5),.22,.024,stone)
 elif idx==6:
  for xx in [-.20,.20]:horn((xx,0,.12),(xx,0,.45),.085,stone)
 elif idx==5:box((0,-.24,.23),(.28,.06,.28),stone)
 elif idx==3:
  for z in [.63,.68,.73]:ring((0,.1,z),.043,.012,mats['secondary'])
 themed=export('SM_'+code+'_Lever','lever',idx);props.append(themed)
 # Six world-key silhouettes; original inventory/UI key artwork and names remain authoritative.
 keytype={0:'Rusted',2:'Crypt',5:'Warden',6:'Drake',7:'Infernal',8:'Hell'}.get(idx)
 if keytype:
  rod((0,0,-.28),(0,0,.18),.026,metal,'Key shaft')
  for z,w in [(-.23,.14),(-.12,.10)]:box((w/2,0,z),(w,.055,.048),metal,b=.004)
  if keytype=='Warden':
   for p,sz in [((0,0,.30),(.30,.055,.04)),((0,0,.10),(.30,.055,.04)),((-.13,0,.20),(.04,.055,.24)),((.13,0,.20),(.04,.055,.24))]:box(p,sz,metal,b=.006)
  else:ring((0,0,.23),.15,.026,metal,arc=5.4 if keytype=='Hell' else 2*math.pi)
  if keytype in ['Crypt','Drake','Infernal','Hell']:
   for xx in [-.10,.10]:horn((xx,0,.33),(xx*1.8,0,.48 if keytype in ['Drake','Hell'] else .41),.03,stone if keytype=='Crypt' else metal)
  if keytype in ['Infernal','Hell']:box((0,-.012,.23),(.052,.07,.22),detail,b=.006)
  key=export('SM_Key_'+keytype,'world_key',idx);props.append(key)
 # Neutral prop layout. All exports already retain their own pivots.
 for i,o in enumerate(props):o.location=((i-(len(props)-1)/2)*1.1,0,0 if 'Key_' not in o.name else .75)
 neutral=bpy.data.materials.new('StudioNeutral');neutral.diffuse_color=(.11,.12,.13,1);box((0,0,-.08),(20,20,.10),neutral,b=0);parts.clear()
 lights=[area((1,-4,5),850,(1,.88,.73),5),area((-3,-1,3),650,(.72,.85,1),4),area((2,3,4),900,(1,1,1),3)]
 bpy.ops.object.camera_add(location=(3,-7,3));cam=bpy.context.object;aim(cam,(0,0,.5));cam.data.type='ORTHO';cam.data.ortho_scale=6.0;S.camera=cam;S.render.filepath=str(R/'previews'/(code+'_Props_Blender.png'));bpy.ops.render.render(write_still=True)
 # Neutral 3x3 material sheets are real shader renders, arranged in a reusable source scene.
 for o in list(S.objects):o.hide_render=True
 boards=[]
 for i,(role,mat) in enumerate(mats.items()):
  bpy.ops.mesh.primitive_plane_add(size=2,location=((i%4)*2.1-3.15,(i//4)*2.1-1.05,0));o=bpy.context.object;o.data.materials.append(mat)
  for d in o.data.uv_layers.active.data:d.uv*=3
  boards.append(o)
 cam.hide_render=False;cam.location=(0,0,10);cam.rotation_euler=(0,0,0);cam.data.ortho_scale=8.6;S.render.resolution_x=1600;S.render.resolution_y=850;light=area((0,-2,5),850,(1,1,1),6,target=(0,0,0));S.render.filepath=str(R/'previews'/(code+'_Materials_Blender3x3.png'));bpy.ops.render.render(write_still=True)
 # Compact corridor showcase uses real arched openings and region dressing.
 for o in list(S.objects):o.hide_render=True
 parts.clear();S.render.resolution_x=1600;S.render.resolution_y=900
 box((0,6,-.12),(4,12,.24),mats['floor'],b=.015);box((0,6,5.3),(4.5,12,.20),mats['vault'])
 for xx in [-2.13,2.13]:
  box((xx,6,2.6),(.26,12,5.2),main)
  box((xx*.93,6,.42),(.05,12,.84),mats['secondary'])
  for yy in [2,6,10]:box((xx*.91,yy,2.1),(.32,.35,4.2),mats['trim'])
 # voussoirs around a true aperture, no wall painting.
 for yy in [4,8,12]:
  for i in range(13):
   a=math.pi*(i+.5)/13;o=box((1.68*math.cos(a),yy,3.35+1.68*math.sin(a)),(.40,.38,.40),mats['trim']);o.rotation_euler[1]=a
 box((0,12.12,2.6),(4.2,.24,5.2),main)
 corridor=export('SM_'+code+'_Showcase','showcase',idx)
 for i,p in enumerate(props[:3]):
  for j in range(2):q=p.copy();q.data=p.data;bpy.context.collection.objects.link(q);q.hide_render=False;q.location=((-1 if (i+j)%2 else 1)*1.6,3+i*2+j*3,0);q.rotation_euler[2]=0
 cam.hide_render=False;cam.location=(.2,.4,1.55);cam.data.type='PERSP';cam.data.sensor_width=36;cam.data.lens=36/(2*math.tan(math.radians(76)/2));aim(cam,(.2,8,1.55));S.camera=cam
 for p in [(-1.8,2,2.0),(1.8,5,2),(-1.8,8,2)]:area(p,230,(1,.45,.14),.35,target=(0,p[1]+1,1))
 area((0,11,4),380,(.40,.56,.70) if idx<7 else (.85,.18,.08),2,target=(0,5,1));area((0,-.6,2),160,(.65,.76,1),2,target=(0,5,1.4))
 S.render.filepath=str(R/'previews'/(code+'_Corridor_Blender.png'));bpy.ops.render.render(write_still=True)
 bpy.ops.wm.save_as_mainfile(filepath=str(R/'sources'/(code+'.blend')))
 (R/'reports'/(code+'_models.json')).write_text(json.dumps([a for a in results if a['region']==idx],indent=2));print('REGION_BLENDER_COMPLETE',code,flush=True)
allprops=[]
for p in sorted((R/'reports').glob('*_models.json')):allprops+=json.loads(p.read_text())
M['props']=allprops;(R/'manifest.json').write_text(json.dumps(M,indent=2));print('MODELS_COMPLETE',len(allprops),flush=True)
