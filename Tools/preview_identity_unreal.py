"""UE 5.8 isolated architectural identity gallery using the exported runtime modules."""
import unreal as u,json,time,traceback
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Identity');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve()
M=json.loads((R/'manifest.json').read_text());ROOT='/Game/RegionalIdentity';A=u.EditorAssetLibrary;actors=u.get_editor_subsystem(u.EditorActorSubsystem);level=u.get_editor_subsystem(u.LevelEditorSubsystem)
index=0;stage=0;started=time.time();cam=None;lights=[];handle=None;report={'status':'RUNNING','regions':[],'errors':[]}
choices=[dict(index=1,code='Sewers',name='Old City Sewers - approved benchmark')]+list(M['regions'])+[dict(index=8,code='Hell',name='Hell - The Hellforge',variant='ForgeFloor'),dict(index=8,code='Hell',name='Hell - The Broken Halo',variant='HaloFloor')]
hell_only='-IdentityHellOnly' in u.SystemLibrary.get_command_line()
if hell_only:choices=[r for r in choices if r['index']==8]
def tag(reg):return reg['code']+('_'+reg['variant'] if 'variant' in reg else '')
def build(reg):
 global cam,lights
 path=ROOT+'/Review_'+tag(reg)
 u.EditorLoadingAndSavingUtils.new_blank_map(False)
 def place(name,p,yaw=0):
  original=name
  if reg['index']==1:
   name={'Cap':'Floor','Niche':'Wall'}.get(name,name);mesh=u.load_asset('/Game/RegionalIdentityBenchmark/SM_Bench_'+name)
  else:mesh=u.load_asset(ROOT+'/Meshes/SM_ID_'+reg['code']+'_'+name)
  assert mesh,name
  o=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*p),u.Rotator(pitch=0,yaw=yaw,roll=0));o.static_mesh_component.set_static_mesh(mesh);o.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION);return o
 for j in range(4):
  yy=j*400+200;place(reg.get('variant','Floor'),(0,yy,0));place('Vault',(0,yy,0));place('Cap',(0,yy,590))
  for side in [-1,1]:place('Niche' if j%2==0 else 'Wall',(side*200,yy,0),90)
  if j>0:place('Arch',(0,j*400,0))
 place('Wall',(0,1600,0))
 cam=actors.spawn_actor_from_class(u.CameraActor,u.Vector(18,20,155),u.Rotator(pitch=0,yaw=90,roll=0));cam.camera_component.set_field_of_view(76);cam.camera_component.set_constraint_aspect_ratio(False);u.EditorLevelLibrary.set_level_viewport_camera_info(cam.get_actor_location(),cam.get_actor_rotation())
 lights=[]
 for p,power in [((0,-50,260),5000),((-145,220,250),5000),((145,600,250),5000),((-145,1000,250),5000),((145,1400,250),5000)]:
  l=actors.spawn_actor_from_class(u.PointLight,u.Vector(*p));c=l.point_light_component;c.set_mobility(u.ComponentMobility.MOVABLE);c.set_intensity(power);c.set_attenuation_radius(900);c.set_light_color(u.LinearColor(1,1,1));c.set_source_radius(30);lights.append(l)
 pp=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector());pp.set_editor_property('unbound',True);s=pp.get_editor_property('settings');s.set_editor_property('override_auto_exposure_method',True);s.set_editor_property('auto_exposure_method',u.AutoExposureMethod.AEM_MANUAL);s.set_editor_property('override_auto_exposure_bias',True);s.set_editor_property('auto_exposure_bias',2);pp.set_editor_property('settings',s)
 assert u.EditorLoadingAndSavingUtils.save_map(u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world(),path)
 report['regions'].append(dict(name=reg['name'],level=path,camera_height_cm=155,horizontal_fov=76,lighting='Identical white lights and exposure across all regions'))
 print('IDENTITY_UNREAL_REVIEW',tag(reg),flush=True)
def tick(dt):
 global index,stage,started,handle
 try:
  elapsed=time.time()-started
  if stage==0 and elapsed>3:
   stage=-1;build(choices[index]);stage=1;started=time.time()
  elif stage==1 and elapsed>16:
   u.AutomationLibrary.take_high_res_screenshot(1600,900,str(R/'previews'/(tag(choices[index])+'_Neutral_Unreal.png')),camera=cam);stage=2;started=time.time()
  elif stage==2 and elapsed>4:
   for i,l in enumerate(lights):l.point_light_component.set_light_color(u.LinearColor(1,.45,.16) if i else u.LinearColor(.60,.70,.85));l.point_light_component.set_intensity(4200 if i else 2500)
   stage=3;started=time.time()
  elif stage==3 and elapsed>7:
   u.AutomationLibrary.take_high_res_screenshot(1600,900,str(R/'previews'/(tag(choices[index])+'_Torch_Unreal.png')),camera=cam);stage=4;started=time.time()
  elif stage==4 and elapsed>4:
   index+=1
   if index<len(choices):stage=0
   else:report['status']='CAPTURES_COMPLETE';(R/('reports/unreal_hell_lighting.json' if hell_only else 'reports/unreal_lighting.json')).write_text(json.dumps(report,indent=2));u.unregister_slate_post_tick_callback(handle);u.SystemLibrary.quit_editor()
 except Exception:
  report['errors'].append(traceback.format_exc());(R/'reports/unreal_lighting.json').write_text(json.dumps(report,indent=2));u.unregister_slate_post_tick_callback(handle);u.SystemLibrary.quit_editor()
handle=u.register_slate_post_tick_callback(tick)
