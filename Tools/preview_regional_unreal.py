"""Isolated UE regional lighting/scale review; 16 captures, saved showcase maps."""
import unreal as u,json,time,traceback
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Art');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve();M=json.loads((R/'manifest.json').read_text());ROOT='/Game/RegionalArt';A=u.EditorAssetLibrary;actors=u.get_editor_subsystem(u.EditorActorSubsystem);level=u.get_editor_subsystem(u.LevelEditorSubsystem)
index=0;stage=0;started=time.time();cam=None;lights=[];handle=None;report={'status':'RUNNING','regions':[],'errors':[]}
def build(reg):
 global cam,lights
 path=ROOT+'/Review_'+reg['code']
 if A.does_asset_exist(path):level.load_level(path)
 else:u.EditorLoadingAndSavingUtils.new_blank_map(False)
 for old in actors.get_all_level_actors():
  if isinstance(old,(u.StaticMeshActor,u.CameraActor,u.PointLight,u.PostProcessVolume,u.DecalActor)):actors.destroy_actor(old)
 mesh=u.load_asset(ROOT+'/Meshes/SM_'+reg['code']+'_Showcase');assert mesh
 b=mesh.get_bounding_box();sign=1 if b.max.y>abs(b.min.y) else -1
 def pos(x,y,z):return u.Vector(x*100,y*100*sign,z*100)
 obj=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector());obj.static_mesh_component.set_static_mesh(mesh)
 for i,s in enumerate(mesh.get_editor_property('static_materials')):
  n=str(s.get_editor_property('imported_material_slot_name'));obj.static_mesh_component.set_material(i,u.load_asset(ROOT+'/Materials/MI_'+n))
 obj.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
 for i,name in enumerate(reg['props']):
  for j in range(2):
   o=actors.spawn_actor_from_class(u.StaticMeshActor,pos((-1 if (i+j)%2 else 1)*1.6,3+i*2+j*3,0));o.static_mesh_component.set_static_mesh(u.load_asset(ROOT+'/Meshes/SM_'+name));o.static_mesh_component.set_collision_enabled(u.CollisionEnabled.NO_COLLISION)
 for i,name in enumerate(reg['decals']):
  d=actors.spawn_actor_from_class(u.DecalActor,pos(-1.965,2+i*2,.95+i*.2),u.Rotator(pitch=0,yaw=0,roll=0));dc=d.get_component_by_class(u.DecalComponent);dc.set_decal_material(u.load_asset(ROOT+'/Materials/MI_'+name));dc.set_editor_property('decal_size',u.Vector(12,65,80))
 cam=actors.spawn_actor_from_class(u.CameraActor,pos(.2,.4,1.55),u.Rotator(pitch=0,yaw=90*sign,roll=0));cam.camera_component.set_field_of_view(76);cam.camera_component.set_constraint_aspect_ratio(False);u.EditorLevelLibrary.set_level_viewport_camera_info(cam.get_actor_location(),cam.get_actor_rotation());lights=[]
 for x,y,z,power,col in [(-1.6,2,2,5000,(1,.46,.16)),(1.6,5,2,5000,(1,.46,.16)),(-1.6,9,2,5000,(1,.46,.16)),(0,-.3,2,3000,(.65,.76,1)),(0,11,4,7000,(.42,.56,.72) if reg['index']<7 else (.95,.20,.05))]:
  l=actors.spawn_actor_from_class(u.PointLight,pos(x,y,z));c=l.point_light_component;c.set_mobility(u.ComponentMobility.MOVABLE);c.set_intensity(power);c.set_attenuation_radius(1000);c.set_light_color(u.LinearColor(*col));c.set_source_radius(20);lights.append(l)
 pp=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector());pp.set_editor_property('unbound',True);settings=pp.get_editor_property('settings');settings.set_editor_property('override_auto_exposure_method',True);settings.set_editor_property('auto_exposure_method',u.AutoExposureMethod.AEM_MANUAL);settings.set_editor_property('override_auto_exposure_bias',True);settings.set_editor_property('auto_exposure_bias',2);pp.set_editor_property('settings',settings);assert u.EditorLoadingAndSavingUtils.save_map(u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world(),path)
 report['regions'].append({'name':reg['name'],'level':path,'camera_height_cm':155,'horizontal_fov':76,'decals':4,'props':6});print('REGIONAL_ISOLATED_REVIEW',reg['code'],flush=True)
def tick(dt):
 global index,stage,started,handle
 try:
  elapsed=time.time()-started
  if stage==0 and elapsed>3:
   stage=-1;build(M['regions'][index]);stage=1;started=time.time()
  elif stage==1 and elapsed>18:
   u.AutomationLibrary.take_high_res_screenshot(1600,900,str(R/'previews'/(M['regions'][index]['code']+'_Corridor_Unreal.png')),camera=cam);stage=2;started=time.time()
  elif stage==2 and elapsed>5:
   for l in lights:l.point_light_component.set_light_color(u.LinearColor(1,1,1));l.point_light_component.set_intensity(5500)
   stage=3;started=time.time()
  elif stage==3 and elapsed>8:
   u.AutomationLibrary.take_high_res_screenshot(1600,900,str(R/'previews'/(M['regions'][index]['code']+'_Neutral_Unreal.png')),camera=cam);stage=4;started=time.time()
  elif stage==4 and elapsed>5:
   index+=1
   if index<len(M['regions']):stage=0
   else:report['status']='CAPTURES_COMPLETE';(R/'reports/unreal_lighting.json').write_text(json.dumps(report,indent=2));u.unregister_slate_post_tick_callback(handle);u.SystemLibrary.quit_editor()
 except Exception:
  report['errors'].append(traceback.format_exc());(R/'reports/unreal_lighting.json').write_text(json.dumps(report,indent=2));u.unregister_slate_post_tick_callback(handle);u.SystemLibrary.quit_editor()
handle=u.register_slate_post_tick_callback(tick)
