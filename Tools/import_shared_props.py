"""Import only new shared interactable assets. Run with UE 5.8 Python commandlet."""
import unreal as u,json
from pathlib import Path
R=Path(u.Paths.project_dir()).resolve();D=R/'ArtReview/SharedProps';A=u.EditorAssetLibrary;E=u.MaterialEditingLibrary;T=u.AssetToolsHelpers.get_asset_tools();ROOT='/Game/SharedInteractables'
def imported(file,path,name,options=None):
 task=u.AssetImportTask();task.filename=str(file);task.destination_path=path;task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
 if options:task.options=options
 T.import_asset_tasks([task]);asset=u.load_asset(path+'/'+name);assert asset,name;return asset
materials={}
for kind in ['Oak','Iron']:
 m=u.load_asset(ROOT+'/Materials/M_'+kind) or T.create_asset('M_'+kind,ROOT+'/Materials',u.Material,u.MaterialFactoryNew());E.delete_all_material_expressions(m)
 for i,role in enumerate(['BC','N','R','M']):
  tex=imported(D/'textures'/f'{kind}_{role}.png',ROOT+'/Textures','T_'+kind+'_'+role);tex.set_editor_property('srgb',role=='BC')
  tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_NORMALMAP if role=='N' else u.TextureCompressionSettings.TC_GRAYSCALE if role in ['R','M'] else u.TextureCompressionSettings.TC_DEFAULT)
  if role=='N':tex.set_editor_property('flip_green_channel',False)
  A.save_loaded_asset(tex);n=E.create_material_expression(m,u.MaterialExpressionTextureSampleParameter2D,-400,i*220);n.set_editor_property('parameter_name',role);n.set_editor_property('texture',tex)
  n.set_editor_property('sampler_type',u.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='N' else u.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE if role in ['R','M'] else u.MaterialSamplerType.SAMPLERTYPE_COLOR)
  prop={'BC':u.MaterialProperty.MP_BASE_COLOR,'N':u.MaterialProperty.MP_NORMAL,'R':u.MaterialProperty.MP_ROUGHNESS,'M':u.MaterialProperty.MP_METALLIC}[role];assert E.connect_material_property(n,'RGB' if role in ['BC','N'] else 'R',prop)
 E.set_material_usage(m,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);assert not E.recompile_material(m);A.save_loaded_asset(m);materials[kind]=m
report=[]
for p in sorted((D/'exports').glob('*.fbx')):
 opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;opt.static_mesh_import_data.generate_lightmap_u_vs=False
 mesh=imported(p,ROOT+'/Meshes',p.stem,opt)
 slots=mesh.get_editor_property('static_materials');names=[]
 for i,s in enumerate(slots):
  name=str(s.get_editor_property('imported_material_slot_name'));key=next((k for k in materials if k in name),None);assert key,name;mesh.set_material(i,materials[key]);names.append(key)
 b=mesh.get_bounding_box();size=b.max-b.min;assert 40<size.x<130 and 60<size.z<130
 A.save_loaded_asset(mesh);report.append({'asset':mesh.get_path_name(),'size_cm':[size.x,size.y,size.z],'materials':names})
(R/'Saved/SharedPropUpdate/import_report.json').write_text(json.dumps(report,indent=2));print('SHARED_PROPS_IMPORT_PASS',report)
