"""Blender 5.2: regional visual modules, authored metre UVs, editable scenes and renders.
Collision is deliberately not exported. Runtime retains the original collision kit.
"""
import bpy, math, random, json, sys
from pathlib import Path
from mathutils import Vector
R=Path(r'J:\Lonemoore_Regional_Identity');M=json.loads((R/'manifest.json').read_text());selection=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
results=[];parts=[]
def reset():
 for o in bpy.data.objects:o.hide_set(False);o.hide_viewport=False
 bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
 S=bpy.context.scene;S.unit_settings.system='METRIC';S.unit_settings.scale_length=1;S.render.engine='CYCLES';S.cycles.samples=32
 try:
  p=bpy.context.preferences.addons['cycles'].preferences;p.compute_device_type='OPTIX';p.get_devices()
  for d in p.devices:d.use=d.type!='CPU'
  S.cycles.device='GPU'
 except Exception:pass
 S.render.resolution_x=1600;S.render.resolution_y=900;S.render.resolution_percentage=100;S.world.color=(.08,.08,.08);S.view_settings.view_transform='AgX';S.view_settings.look='AgX - Medium High Contrast';return S
def mat(a):
 m=bpy.data.materials.new(a['id']);m.use_nodes=True;nd=m.node_tree.nodes;l=m.node_tree.links;bs=nd.get('Principled BSDF');bs.inputs['Roughness'].default_value=.8
 for role,path in a['maps'].items():
  t=nd.new('ShaderNodeTexImage');t.image=bpy.data.images.load(str(R/path),check_existing=True);t.image.colorspace_settings.name='sRGB' if role=='base_color' else 'Non-Color';t.extension='REPEAT'
  if role=='normal':
   sep=nd.new('ShaderNodeSeparateColor');com=nd.new('ShaderNodeCombineColor');inv=nd.new('ShaderNodeMath');inv.operation='SUBTRACT';inv.inputs[0].default_value=1;l.new(t.outputs['Color'],sep.inputs[0]);l.new(sep.outputs[0],com.inputs[0]);l.new(sep.outputs[1],inv.inputs[1]);l.new(inv.outputs[0],com.inputs[1]);l.new(sep.outputs[2],com.inputs[2]);n=nd.new('ShaderNodeNormalMap');n.inputs['Strength'].default_value=.8;l.new(com.outputs[0],n.inputs['Color']);l.new(n.outputs[0],bs.inputs['Normal'])
  elif role=='emission':
   mix=nd.new('ShaderNodeMixRGB');mix.blend_type='MULTIPLY';mix.inputs[0].default_value=1;mix.inputs[2].default_value=(1,.11,.008,1);l.new(t.outputs[0],mix.inputs[1]);l.new(mix.outputs[0],bs.inputs['Emission Color']);bs.inputs['Emission Strength'].default_value=2
  else:l.new(t.outputs[0],bs.inputs[{'base_color':'Base Color','roughness':'Roughness','metallic':'Metallic'}[role]])
 return m
def finish(o,ma,bevel=0):
 bpy.context.view_layer.objects.active=o;bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
 o.data.materials.append(ma)
 if bevel:
  mod=o.modifiers.new('Small worn edges','BEVEL');mod.width=bevel;mod.segments=2;bpy.ops.object.modifier_apply(modifier=mod.name)
 if not o.data.uv_layers:o.data.uv_layers.new(name='MetricUV_2m')
 o.data.uv_layers.active.name='MetricUV_2m'
 for p in o.data.polygons:
  # Fresh from_pydata meshes may not have cached polygon normals yet.
  # Use the actual geometric normal, then project along its dominant axis.
  cs=[o.data.vertices[vi].co for vi in p.vertices]
  normal=Vector((0,0,0))
  for i in range(1,len(cs)-1):normal+=(cs[i]-cs[0]).cross(cs[i+1]-cs[0])
  axis=max(range(3),key=lambda i:abs(normal[i]));axes=[i for i in range(3) if i!=axis]
  for li in p.loop_indices:
   co=o.data.vertices[o.data.loops[li].vertex_index].co+o.location;o.data.uv_layers.active.data[li].uv=(co[axes[0]]/2,co[axes[1]]/2)
 parts.append(o);return o
def box(p,sz,ma,b=.015,name='Block'):
 bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.name=name;o.dimensions=sz;return finish(o,ma,b)
def mesh(name,vs,fs,ma):
 me=bpy.data.meshes.new(name);me.from_pydata(vs,[],fs);me.update();o=bpy.data.objects.new(name,me);bpy.context.collection.objects.link(o);return finish(o,ma)
def beam(a,b,width,depth,ma):
 v=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cube_add(size=1,location=(Vector(a)+Vector(b))/2);o=bpy.context.object;o.dimensions=(width,depth,v.length);o.rotation_euler=v.to_track_quat('Z','Y').to_euler();return finish(o,ma,.014)
def prism_xz(poly,depth,ma,name='Profile',cy=0):
 vs=[(xx,cy+yy,zz) for yy in [-depth/2,depth/2] for xx,zz in poly];n=len(poly)
 fs=[tuple(reversed(range(n))),tuple(range(n,n*2))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)];return mesh(name,vs,fs,ma)
def faceted_wall(ma,seed,columns=False):
 rng=random.Random(seed);nx=10 if columns else 9;nz=8 if columns else 13
 box((0,0,2.60),(4,.34,5.2),ma,.0)
 for sign in [-1,1]:
  vs=[]
  for j in range(nz+1):
   for i in range(nx+1):
    xx=-2+i*4/nx;zz=j*5.2/nz
    if i not in [0,nx]:xx+=rng.uniform(-.12,.12)
    if j not in [0,nz]:zz+=rng.uniform(-.10,.10)
    relief=(.06+.10*(.5+.5*math.cos(i*2.2))) if columns else rng.uniform(.025,.19)
    if i in [0,nx]:relief=.045
    vs.append((xx,sign*(.19+relief),zz))
  fs=[]
  for j in range(nz):
   for i in range(nx):
    a=j*(nx+1)+i;b=a+1;c=b+nx+1;d=a+nx+1
    faces=[(a,b,c),(a,c,d)] if (i+j)%2 else [(a,b,d),(b,c,d)]
    fs += [tuple(reversed(f)) if sign>0 else f for f in faces]
  mesh('Fractured volcanic faces' if columns else 'Excavated rock facets',vs,fs,ma)
def roots(ma):
 for side in [-1,1]:
  for i in range(5):
   cu=bpy.data.curves.new('Exposed root','CURVE');cu.dimensions='3D';cu.bevel_depth=.024+(i%3)*.011;cu.bevel_resolution=2;s=cu.splines.new('BEZIER');s.bezier_points.add(4)
   for j,p in enumerate(s.bezier_points):p.co=(-1.8+i*.83+math.sin(j*1.1+i)*.09,side*(.35+math.cos(j)*.035),5.15-j*(.32+(i%2)*.13));p.handle_left_type='AUTO';p.handle_right_type='AUTO';p.radius=1-j*.15
   o=bpy.data.objects.new('Hanging root',cu);bpy.context.collection.objects.link(o);bpy.context.view_layer.objects.active=o;o.select_set(True);bpy.ops.object.convert(target='MESH');o=bpy.context.object;finish(o,ma);o.select_set(False)
def arch_frame(code,wall,trim):
 if code=='Cathedral':
  for side in [-1,1]:
   box((side*1.81,0,2.35),(.38,.65,4.7),trim,.025)
   for y in [-.36,.36]:
    beam((side*1.72,y,3.7),(side*.84,y,4.65),.16,.13,trim);beam((side*.84,y,4.65),(0,y,5.05),.16,.13,trim)
  prism_xz([(-2,5.2),(2,5.2),(1.54,4.43),(.80,4.83),(0,5.12),(-.80,4.83),(-1.54,4.43)],.56,wall)
 elif code=='Catacombs':
  for side in [-1,1]:box((side*1.81,0,2.28),(.38,.66,4.56),trim,.04)
  box((0,0,4.93),(4,.68,.55),wall,.04);box((0,-.02,4.61),(3.96,.74,.16),trim,.035)
 elif code in ['Warrens','Fortress']:
  for side in [-1,1]:
   box((side*1.82,0,2.40),(.36,.57,4.8),trim,.035)
   for z in [.3,2.2,4.2]:box((side*1.82,-.03,z),(.39,.62,.095),wall if code=='Warrens' else trim,.004)
   beam((side*1.80,0,3.55),(side*.98,0,4.75),.22,.35,trim)
  box((0,0,4.95),(4,.59,.40),trim,.04)
 elif code=='Crypts':
  for side in [-1,1]:
   box((side*1.82,0,2.25),(.36,.60,4.5),trim,.012);box((side*1.81,0,.16),(.42,.66,.32),wall)
  prism_xz([(-2,5.2),(2,5.2),(1.61,4.15),(1.16,4.66),(-1.16,4.66),(-1.61,4.15)],.57,wall)
  for side in [-1,1]:beam((side*1.63,-.34,4.13),(side*1.12,-.34,4.72),.12,.10,trim)
  box((0,-.34,4.74),(2.3,.1,.12),trim)
 else:
  # Angular portals, no reused round masonry arch.
  for side in [-1,1]:
   prism_xz([(side*1.61,0),(side*2,0),(side*2,5.2),(side*1.74,4.55)],.67,wall)
   beam((side*1.78,-.37,3.75),(side*.98,-.37,4.80),.18,.17,trim)
  prism_xz([(-2,5.2),(2,5.2),(1.60,4.24),(.90,4.78),(0,5.04),(-.90,4.78),(-1.60,4.24)],.64,wall)
  if code=='Hell':
   for side in [-1,1]:prism_xz([(side*1.82,3.2),(side*1.55,4.42),(side*1.64,4.30),(side*1.98,3.48)],.84,trim,name='Infernal angular brace')
def wall_module(code,ma,niche=False):
 w,f,c,t=[ma[k] for k in ['wall','floor','ceiling','trim']]
 if code in ['Warrens','Deep','Hell']:
  faceted_wall(w,81+len(code),code=='Hell')
  if code=='Warrens':
   for xx in [-1.82,.0,1.82]:box((xx,0,2.6),(.19,.79,5.2),t,.027)
   for z in [.45,3.8,5.0]:box((0,0,z),(4,.74,.16),t,.02)
   roots(t)
   # Keep timber bevels clear of the rock relief (up to 38cm deep).
   # Apply after UV authoring, matching fix_warrens_surfaces.py for existing assets.
   for part in parts:
    if t in list(part.data.materials):
     part.location.y*=1.4
     for vertex in part.data.vertices:vertex.co.y*=1.4
  if code=='Hell':
   for xx in [-1.8,1.8]:
    for sign in [-1,1]:prism_xz([(xx-.14,0),(xx+.14,0),(xx+.10,4.9),(xx-.03,5.2)],.24,t,cy=sign*.43)
 elif code=='Catacombs':
  box((0,0,2.6),(4,.28,5.2),w)
  for z in [.20,1.50,2.80,4.10,5.10]:box((0,0,z),(4,.94,.19),t,.035)
  for xx in [-1.88,-.64,.64,1.88]:box((xx,0,2.6),(.17,.86,5.2),t,.026)
  # Real recessed burial compartments with lids on shelves, in both wall faces.
  for sign in [-1,1]:
   for xx in [-1.26,0,1.26]:
    for z in [.36,1.66,2.96,4.26]:
     box((xx,sign*.27,z),(1.02,.39,.14),w,.048)
     if niche:
      box((xx,sign*.29,z+.15),(.78,.30,.16),t,.04)
 elif code=='Crypts':
  box((0,0,2.6),(4,.36,5.2),t)
  for sign in [-1,1]:
   for xx in [-1.34,0,1.34]:
    poly=[(xx-.52,.30),(xx+.52,.30),(xx+.52,3.74),(xx+.33,4.63),(xx-.33,4.63),(xx-.52,3.74)]
    prism_xz(poly,.14,w,cy=sign*.24,name='Tall funerary facing')
    for zz in [1.50,1.66,1.82]:box((xx,sign*.32,zz),(.45,.018,.025),t,.003)
  for zz in [.16,4.85,5.15]:box((0,0,zz),(4,.67,.15),w,.022)
 elif code=='Cathedral':
  box((0,0,2.6),(4,.46,5.2),w,.005)
  for zz in [.16,1.1,4.55,5.12]:box((0,0,zz),(4,.66,.17),t,.025)
  for sign in [-1,1]:
   for xx in [-1.83,1.83]:box((xx,sign*.29,2.65),(.22,.18,4.6),t)
   if niche:
    for xx in [-1,1]:
     for dx in [-.58,.58]:box((xx+dx,sign*.33,2.44),(.10,.12,2.65),t)
     beam((xx-.58,sign*.33,3.75),(xx,sign*.33,4.28),.10,.12,t);beam((xx,sign*.33,4.28),(xx+.58,sign*.33,3.75),.10,.12,t)
 elif code=='Fortress':
  box((0,0,2.6),(4,.46,5.2),w,.014)
  for zz in [.23,3.65,5.13]:box((0,0,zz),(4,.65,.22),w,.027)
  for xx in [-1.84,1.84]:box((xx,0,2.6),(.18,.72,5.2),t,.009)
 else:
  box((0,0,2.6),(4,.46,5.2),w,.01)
  for side in [-1,1]:
   for xx in [-1.8,0,1.8]:prism_xz([(xx-.12,.0),(xx+.12,0),(xx+.08,5.2),(xx-.08,5.2)],.20,t,cy=side*.29)
   for zz in [.32,4.62]:box((0,side*.30,zz),(4,.12,.12),t)
def ceiling_module(code,ma):
 w,f,c,t=[ma[k] for k in ['wall','floor','ceiling','trim']]
 if code=='Cathedral':
  vs=[];fs=[];n=24
  for j in range(n+1):
   for i in range(n+1):
    xx=-2+4*i/n;yy=-2+4*j/n;z=4.88+.90*(1-max(abs(xx),abs(yy))/2)**.65;vs.append((xx,yy,z))
  for j in range(n):
   for i in range(n):a=j*(n+1)+i;fs.append((a,a+n+1,a+n+2,a+1))
  mesh('Pointed plaster vault',vs,fs,c)
  for sign in [-1,1]:
   for i in range(16):
    xx=-2+i*.25;nx=xx+.25
    beam((xx,sign*xx,4.85+.9*(1-abs(xx)/2)**.65),(nx,sign*nx,4.85+.9*(1-abs(nx)/2)**.65),.11,.11,t)
 elif code in ['Warrens','Fortress','Crypts','Catacombs','Infernal']:
  box((0,0,5.55),(4.02,4.02,.23),c,.01)
  if code in ['Crypts','Catacombs']:
   for p in [-1.98,0,1.98]:box((p,0,5.28),(.18,4,.35),t);box((0,p,5.28),(4,.18,.35),t)
   if code=='Crypts':
    for xx in [-1,1]:
     for yy in [-1,1]:box((xx,yy,5.39),(1.48,1.48,.12),w)
  elif code=='Infernal':
   for side in [-1,1]:
    beam((-2,side*1.8,4.95),(0,side*1.8,5.40),.22,.27,t);beam((0,side*1.8,5.40),(2,side*1.8,4.95),.22,.27,t)
   box((0,0,5.3),(.28,4,.32),t)
  else:
   for yy in [-1.8,0,1.8]:box((0,yy,5.16),(4,.24,.46),t,.025)
   for xx in [-1.8,1.8]:box((xx,0,5.18),(.19,4,.36),t)
   if code=='Warrens':
    for i in range(6):
     beam((-1.8,-1.6+i*.6,5.14),(-1.2,-1.45+i*.6,4.55+(i%3)*.12),.04,.05,t)
 else:
  rng=random.Random(166);n=10;vs=[]
  for j in range(n+1):
   for i in range(n+1):
    xx=-2+i*.4;yy=-2+j*.4;edge=i in [0,n] or j in [0,n];z=5.58 if edge else rng.uniform(5.12,5.76);vs.append((xx,yy,z))
  fs=[]
  for j in range(n):
   for i in range(n):a=j*(n+1)+i;fs.extend([(a,a+n+1,a+1),(a+1,a+n+1,a+n+2)])
  mesh('Angular overburden',vs,fs,c)
  for i in range(8 if code=='Hell' else 5):
   xx=rng.uniform(-1.8,1.8);yy=rng.uniform(-1.8,1.8);bpy.ops.mesh.primitive_cone_add(vertices=5,radius1=.018,radius2=.16+rng.random()*.15,depth=.45+rng.random()*.25,location=(xx,yy,5.23));finish(bpy.context.object,c)
  if code=='Hell':
   for xx in [-1.8,1.8]:box((xx,0,5.12),(.17,4,.28),t)
   for yy in [-1.8,1.8]:
    beam((-2,yy,4.94),(0,yy,5.45),.20,.25,t);beam((0,yy,5.45),(2,yy,4.94),.20,.25,t)
def export(code,name,role):
 bpy.ops.object.select_all(action='DESELECT')
 for o in parts:o.select_set(True)
 bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name='SM_ID_'+code+'_'+name;bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR');bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
 o.data.calc_loop_triangles();path=R/'meshes'/(o.name+'.fbx')
 for tri in o.data.loop_triangles:
  us=[o.data.uv_layers.active.data[li].uv for li in tri.loops]
  uvarea=abs((us[1].x-us[0].x)*(us[2].y-us[0].y)-(us[1].y-us[0].y)*(us[2].x-us[0].x))*.5
  assert tri.area<1e-7 or uvarea>tri.area*.10,(o.name,tri.index,'stretched or degenerate UV')
 bpy.ops.export_scene.fbx(filepath=str(path),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
 results.append(dict(id=o.name,role=role,region=reg['index'],fbx=str(path.relative_to(R)).replace('\\','/'),size_m=list(o.dimensions),triangles=len(o.data.loop_triangles),materials=[m.name for m in o.data.materials],pivot='original module pivot at ground centre, metres',collision='none; original runtime collision retained'))
 parts.clear();o.hide_render=True;return o
def aim(o,p):o.rotation_euler=(Vector(p)-o.location).to_track_quat('-Z','Y').to_euler()
def area(p,power,col,size,target):
 bpy.ops.object.light_add(type='AREA',location=p);o=bpy.context.object;o.data.energy=power;o.data.color=col;o.data.shape='DISK';o.data.size=size;aim(o,target);return o
for reg in M['regions']:
 code=reg['code']
 if selection and code not in selection:continue
 print('MODELING_IDENTITY',code,flush=True);S=reset();ma={a['role']:mat(a) for a in M['materials'] if a['region']==reg['index']};kit={}
 for name in ['Wall','Niche','Arch','Floor','Vault','Cap']:
  if name in ['Wall','Niche']:
   wall_module(code,ma,name=='Niche')
   if code!='Cathedral':box((0,0,5.44),(4,.48,.55),ma['wall'],.005)
  elif name=='Arch':
   arch_frame(code,ma['wall'],ma['trim'])
   if code!='Cathedral':box((0,0,5.44),(4,.60,.55),ma['ceiling'],.01)
  elif name=='Vault':ceiling_module(code,ma)
  else:box((0,0,-.105),(4.002,4.002,.20),ma['floor' if name=='Floor' else 'ceiling'],.003)
  kit[name]=export(code,name,name.lower())
 if code=='Hell':
  for nm,role in [('ForgeFloor','forge_floor'),('HaloFloor','halo_floor')]:
   box((0,0,-.105),(4.002,4.002,.20),ma[role],.003);kit[nm]=export(code,nm,role)
 # Native-source gallery and corridor are separately editable collections.
 source=bpy.data.collections.new('Export modules - original pivots');S.collection.children.link(source)
 for o in kit.values():
  for col in list(o.users_collection):col.objects.unlink(o)
  source.objects.link(o);o.hide_set(True)
 show=bpy.data.collections.new('Showcase corridor - visual only');S.collection.children.link(show)
 def place(name,p,rz=0):
  o=kit[name].copy();o.data=kit[name].data;show.objects.link(o);o.location=p;o.rotation_euler.z=rz;o.hide_render=False;o.hide_set(False);return o
 for j in range(4):
  yy=j*4+2;place('Floor',(0,yy,0));place('Vault',(0,yy,0));place('Cap',(0,yy,5.9))
  for side in [-1,1]:place('Niche' if j%2==0 else 'Wall',(side*2,yy,0),math.pi/2)
  if j>0:place('Arch',(0,j*4,0))
 place('Wall',(0,16,0))
 bpy.ops.object.camera_add(location=(.18,.2,1.55));cam=bpy.context.object;cam.data.type='PERSP';cam.data.angle=math.radians(76);cam.data.lens=36/(2*math.tan(math.radians(38)));aim(cam,(.18,10,1.85));S.camera=cam
 lights=[]
 for yy in [1,5,9,13]:lights.append(area((0,yy,3.6),130,(1,1,1),3,(0,yy+1,1.5)))
 lights.append(area((0,-.3,2.8),100,(1,1,1),3,(0,6,2)))
 S.render.filepath=str(R/'previews'/(code+'_Neutral_Blender.png'));bpy.ops.render.render(write_still=True)
 for i,o in enumerate(lights):o.data.color=(1,.42,.16) if i%2==0 else (.55,.65,.76);o.data.energy*=.75
 S.render.filepath=str(R/'previews'/(code+'_Torch_Blender.png'));bpy.ops.render.render(write_still=True)
 bpy.ops.wm.save_as_mainfile(filepath=str(R/'sources'/(code+'_Architecture.blend')))
 (R/'reports'/('models_'+code+'.json')).write_text(json.dumps([p for p in results if p['region']==reg['index']],indent=2))
allprops=[]
for p in sorted((R/'reports').glob('models_*.json')):allprops+=json.loads(p.read_text())
M=json.loads((R/'manifest.json').read_text());M['props']=allprops;(R/'manifest.json').write_text(json.dumps(M,indent=2));print('IDENTITY_MODELS_COMPLETE',len(results),flush=True)
