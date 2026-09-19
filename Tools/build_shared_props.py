"""Repeatable Blender 5.2 shared prop production; run blender -b -t 4 -P this_file."""
import bpy, math, json
import numpy as np
from pathlib import Path
from mathutils import Vector
R=Path(__file__).resolve().parents[1]; D=R/'ArtReview/SharedProps';D.mkdir(parents=True,exist_ok=True)
for sub in ['textures','exports','previews']: (D/sub).mkdir(exist_ok=True)
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
scene=bpy.context.scene;scene.unit_settings.system='METRIC';scene.unit_settings.scale_length=1
N=2048;rng=np.random.default_rng(91626)
y,x=np.mgrid[0:N,0:N].astype(np.float32)/N
def noise(size):
 a=rng.random((size,size)).astype(np.float32);p=x*size;q=y*size;i=p.astype(int)%size;j=q.astype(int)%size;u=p%1;v=q%1;u=u*u*(3-2*u);v=v*v*(3-2*v)
 return a[j,i]*(1-u)*(1-v)+a[j,(i+1)%size]*u*(1-v)+a[(j+1)%size,i]*(1-u)*v+a[(j+1)%size,(i+1)%size]*u*v
coarse=noise(8);fine=noise(256);mid=noise(48)
def image(name,a):
 if a.ndim==2:a=np.repeat(a[:,:,None],3,2)
 if name.endswith('_BC'):a=np.where(a<=.0031308,a*12.92,1.055*np.power(a,1/2.4)-.055)
 im=bpy.data.images.new(name,width=N,height=N,alpha=False);rgba=np.concatenate([np.clip(a,0,1),np.ones((N,N,1),np.float32)],2).astype(np.float32)
 im.pixels.foreach_set(rgba.ravel());im.filepath_raw=str(D/'textures'/f'{name}.png');im.file_format='PNG';im.save();path=im.filepath_raw;bpy.data.images.remove(im);return bpy.data.images.load(path)
maps={}
for kind in ['Oak','Iron']:
 if kind=='Oak':
  warp=.018*np.sin(2*math.pi*y)+.008*np.sin(6*math.pi*y)+.024*(coarse-.5)
  grain=(.5+.5*np.sin((x+warp)*math.pi*2*95))**16
  knots=np.zeros_like(x)
  for cx,cy in [(.27,.35),(.73,.78)]:
   dx=np.minimum(abs(x-cx),1-abs(x-cx));dy=np.minimum(abs(y-cy),1-abs(y-cy));rr=np.sqrt((dx*7)**2+(dy*2.3)**2);knots+=np.exp(-rr*rr*24)*(.5+.5*np.cos(rr*180))
  h=.5+.12*(coarse-.5)+.035*(fine-.5)-.055*grain-.05*knots
  tone=.82+.25*coarse+.10*mid-.19*grain-.23*knots
  bc=tone[:,:,None]*np.array([.30,.18,.085],np.float32);rough=np.clip(.77+.1*mid+.05*grain,0,1);metal=np.zeros_like(x)
 else:
  rust=np.clip((coarse*.65+mid*.35-.49)*5,0,1);h=.5+.04*(mid-.5)+.02*(fine-.5)
  bc=(1-rust[:,:,None])*np.array([.12,.135,.14])+rust[:,:,None]*np.array([.20,.085,.027]);bc*=.8+.3*fine[:,:,None]
  rough=.5+.3*rust+.07*mid;metal=.85*(1-rust)
 dx=(np.roll(h,-1,1)-np.roll(h,1,1))*13;dy=(np.roll(h,-1,0)-np.roll(h,1,0))*13
 normal=np.stack([-dx,dy,np.ones_like(x)],2);normal/=np.linalg.norm(normal,axis=2)[:,:,None]
 maps[kind]={k:image(kind+'_'+k,a) for k,a in [('BC',bc),('N',normal*.5+.5),('R',rough),('M',metal)]}
def mat(kind):
 m=bpy.data.materials.new(kind);m.use_nodes=True;n=m.node_tree.nodes;l=m.node_tree.links;bs=n.get('Principled BSDF')
 for role,socket in [('BC','Base Color'),('R','Roughness'),('M','Metallic')]:
  t=n.new('ShaderNodeTexImage');t.image=maps[kind][role];t.image.colorspace_settings.name='sRGB' if role=='BC' else 'Non-Color';l.new(t.outputs['Color'],bs.inputs[socket])
 # Export normals are DirectX. Invert green only for the Blender preview.
 t=n.new('ShaderNodeTexImage');t.image=maps[kind]['N'];t.image.colorspace_settings.name='Non-Color';sep=n.new('ShaderNodeSeparateXYZ');l.new(t.outputs['Color'],sep.inputs[0]);inv=n.new('ShaderNodeMath');inv.operation='SUBTRACT';inv.inputs[0].default_value=1;l.new(sep.outputs['Y'],inv.inputs[1]);comb=n.new('ShaderNodeCombineXYZ');l.new(sep.outputs['X'],comb.inputs['X']);l.new(inv.outputs[0],comb.inputs['Y']);l.new(sep.outputs['Z'],comb.inputs['Z']);nm=n.new('ShaderNodeNormalMap');l.new(comb.outputs[0],nm.inputs['Color']);l.new(nm.outputs[0],bs.inputs['Normal']);return m
oak=mat('Oak');iron=mat('Iron');parts=[]
def finish(o,name,material,bev=0):
 o.name=name;bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(material)
 if bev:
  m=o.modifiers.new('Soft worn edges','BEVEL');m.width=bev;m.segments=3;bpy.context.view_layer.objects.active=o;bpy.ops.object.modifier_apply(modifier=m.name)
 # Individual part UVs preserve grain direction on long board faces.
 if not o.data.uv_layers:o.data.uv_layers.new()
 for p in o.data.polygons:
  axis=max(range(3),key=lambda a:abs(p.normal[a]));axes=[a for a in range(3) if a!=axis]
  for li in p.loop_indices:
   v=o.data.vertices[o.data.loops[li].vertex_index].co;o.data.uv_layers.active.data[li].uv=(v[axes[0]]*1.3+.371,v[axes[1]]*.55+.213)
 parts.append(o);return o
def box(name,p,s,material=oak,b=.008):
 bpy.ops.mesh.primitive_cube_add(size=1,location=p);o=bpy.context.object;o.dimensions=s;return finish(o,name,material,b)
def cyl(name,a,b,r,material=iron,verts=24):
 v=Vector(b)-Vector(a);bpy.ops.mesh.primitive_cylinder_add(vertices=verts,radius=r,depth=v.length,location=(Vector(a)+Vector(b))/2);o=bpy.context.object;o.rotation_euler=v.to_track_quat('Z','Y').to_euler();return finish(o,name,material,.004)
def bolt(p,axis='Y'):
 a=Vector(p);v=Vector((0,-.014,0) if axis=='Y' else (0,0,.014));return cyl('Forged bolt',a-v,a+v,.022,iron,6)
assets=[]
def export(name):
 bpy.ops.object.select_all(action='DESELECT')
 for o in parts:o.select_set(True)
 bpy.context.view_layer.objects.active=parts[0];bpy.ops.object.join();o=bpy.context.object;o.name=name;scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
 bpy.ops.export_scene.fbx(filepath=str(D/'exports'/f'{name}.fbx'),use_selection=True,axis_forward='-Y',axis_up='Z',apply_unit_scale=True,add_leaf_bones=False,object_types={'MESH'},mesh_smooth_type='FACE')
 o.data.calc_loop_triangles();assets.append({'id':name,'size_m':list(o.dimensions),'triangles':len(o.data.loop_triangles),'materials':[m.name for m in o.data.materials]});parts.clear();return o
# Footprint matches the old 1.15 x .73 m loot box. Real separate boards and metal straps.
for i in range(7):
 for yy in [-.325,.325]:box('Vertical oak plank',(-.48+i*.16,yy,.35),(.154,.055,.60))
for i in range(4):
 for xx in [-.535,.535]:box('End plank',(xx,-.24+i*.16,.35),(.055,.154,.60))
for i in range(5):box('Lid board',(0,-.28+i*.14,.695),(1.13,.134,.075))
box('Bottom',(0,0,.055),(1.10,.69,.10))
for xx in [-.43,.43]:
 for yy in [-.36,.36]:
  box('Iron corner band',(xx,yy,.35),(.067,.025,.67),iron,.004)
  for z in [.10,.34,.61]:bolt((xx,yy-.017 if yy<0 else yy+.017,z))
 box('Lid iron strap',(xx,0,.744),(.067,.735,.025),iron,.004)
for yy in [-.36,.36]:
 box('Lower frame rail',(0,yy,.10),(1.14,.045,.085));box('Upper frame rail',(0,yy,.615),(1.14,.045,.08))
box('Lid latch',(0,-.396,.575),(.105,.04,.21),iron,.008);bolt((0,-.42,.64));box('Latch recess',(0,-.421,.545),(.038,.008,.049),iron,.003)
crate=export('SM_SharedCrate')
# Lever base and open bearing supports with an actual axle and tilted handle.
box('Iron foot',(0,0,.055),(.55,.55,.11),iron,.025);box('Raised base',(0,0,.15),(.43,.42,.10),iron,.02)
for xx in [-.21,.21]:
 for yy in [-.21,.21]:bolt((xx,yy,.12),'Z')
for xx in [-.14,.14]:
 box('Bearing upright',(xx,0,.32),(.075,.23,.30),iron,.028)
 cyl('Bearing collar',(xx-.055,0,.45),(xx+.055,0,.45),.09)
cyl('Pivot axle',(-.245,0,.45),(.245,0,.45),.045)
cyl('Lever arm',(0,0,.45),(0,.20,1.02),.029)
cyl('Grip metal core',(-.18,.20,1.02),(.18,.20,1.02),.034)
cyl('Turned oak grip',(-.16,.20,1.02),(.16,.20,1.02),.052,oak,32)
for xx in [-.17,.17]:cyl('Grip ferrule',(xx-.014,.20,1.02),(xx+.014,.20,1.02),.055)
lever=export('SM_SharedLever')
# Separate view positions affect source preview only; FBX origins stay grounded.
crate.location=(-.80,0,0);lever.location=(.65,0,0)
floor=bpy.data.materials.new('Neutral studio');floor.diffuse_color=(.105,.11,.12,1);floor.use_nodes=True;floor.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=.9
box('Studio floor',(0,0,-.07),(200,200,.1),floor,0);parts.clear()
scene.render.engine='CYCLES';scene.cycles.samples=40;scene.cycles.use_denoising=True
scene.world.color=(.18,.18,.18)
def area(p,power,color,size):
 bpy.ops.object.light_add(type='AREA',location=p);o=bpy.context.object;o.data.energy=power;o.data.color=color;o.data.shape='DISK';o.data.size=size;o.rotation_euler=(Vector((0,0,.5))-o.location).to_track_quat('-Z','Y').to_euler()
area((1,-3,4),700,(1,.88,.73),4);area((-3,-1,2),500,(.72,.83,1),3);area((1,3,3),800,(1,1,1),3)
bpy.ops.object.camera_add(location=(2.3,-3.7,2.3));cam=bpy.context.object;cam.rotation_euler=(Vector((-.15,0,.48))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=3.1;scene.camera=cam
scene.render.resolution_x=1500;scene.render.resolution_y=1000;scene.render.resolution_percentage=100;scene.render.image_settings.file_format='PNG';scene.render.filepath=str(D/'previews/shared_props_blender.png')
bpy.ops.wm.save_as_mainfile(filepath=str(D/'Shared_Interactables.blend'));bpy.ops.render.render(write_still=True)
(D/'manifest.json').write_text(json.dumps({'assets':assets,'textures':'2K procedural source maps, DirectX normals, sRGB BC, linear N/R/M','units':'metres; FBX import to Unreal centimetres','preview':'Blender Cycles neutral studio','seed':91626},indent=2));print('SHARED_PROPS_CREATED',assets)
