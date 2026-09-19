"""UE 5.8 isolated import. Explicit map reconstruction, world/UV parents, bounded stages."""
import unreal as u,json,re
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Identity');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve()
assert u.SystemLibrary.get_engine_version().startswith('5.8.')
M=json.loads((R/'manifest.json').read_text());ROOT='/Game/RegionalIdentity';T=u.AssetToolsHelpers.get_asset_tools();A=u.EditorAssetLibrary;E=u.MaterialEditingLibrary
stage='Meshes' if '-ArtStage=Meshes' in u.SystemLibrary.get_command_line() else 'Textures';report={'stage':stage,'engine':u.SystemLibrary.get_engine_version(),'textures':[],'meshes':[],'errors':[]}
def make(n,cls,f):return u.load_asset(ROOT+'/Materials/'+n) or T.create_asset(n,ROOT+'/Materials',cls,f)
def node(m,cls,**kw):
 n=E.create_material_expression(m,cls,0,0)
 for k,v in kw.items():n.set_editor_property(k,v)
 return n
def link(a,ao,b,bi):assert E.connect_material_expressions(a,ao,b,bi),(a.get_name(),b.get_name(),bi)
def prop(n,out,p):assert E.connect_material_property(n,out,p)
def scalar(m,n,v):return node(m,u.MaterialExpressionScalarParameter,parameter_name=n,default_value=v)
def tex(a,role):return u.load_asset(ROOT+'/Textures/T_'+Path(a['maps'][role]).stem)
if stage=='Textures':
 for a in M['materials']+M['decals']:
  for role,path in a['maps'].items():
   name='T_'+Path(path).stem;task=u.AssetImportTask();task.filename=str(R/path);task.destination_path=ROOT+'/Textures';task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True;
   if '-SkipTextures' not in u.SystemLibrary.get_command_line() or not u.load_asset(ROOT+'/Textures/'+name):T.import_asset_tasks([task])
   t=u.load_asset(ROOT+'/Textures/'+name);assert t,name
   t.set_editor_property('srgb',role=='base_color');t.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_DEFAULT if role=='base_color' else u.TextureCompressionSettings.TC_NORMALMAP if role=='normal' else u.TextureCompressionSettings.TC_GRAYSCALE)
   t.set_editor_property('address_x',u.TextureAddress.TA_CLAMP if a['role']=='decal' else u.TextureAddress.TA_WRAP);t.set_editor_property('address_y',u.TextureAddress.TA_CLAMP if a['role']=='decal' else u.TextureAddress.TA_WRAP);t.set_editor_property('never_stream',False)
   if role=='normal':t.set_editor_property('flip_green_channel',False)
   A.save_loaded_asset(t);report['textures'].append({'asset':t.get_path_name(),'role':role,'srgb':role=='base_color'})
 default=next(a for a in M['materials'] if a.get('kind')=='iron')
 def parent(name,world=False,decal=False):
  m=make(name,u.Material,u.MaterialFactoryNew());E.delete_all_material_expressions(m)
  if decal:m.set_editor_property('material_domain',u.MaterialDomain.MD_DEFERRED_DECAL);m.set_editor_property('blend_mode',u.BlendMode.BLEND_TRANSLUCENT)
  if world:m.set_editor_property('tangent_space_normal',False)
  size=node(m,u.MaterialExpressionVectorParameter,parameter_name='TextureSizeCm',default_value=u.LinearColor(200,200,200,1));uv=node(m,u.MaterialExpressionTextureCoordinate);tiling=scalar(m,'Tiling',1);uvmul=node(m,u.MaterialExpressionMultiply);link(uv,'',uvmul,'A');link(tiling,'',uvmul,'B');samples={}
  for role in (['base_color','roughness'] if decal else ['base_color','normal','roughness','metallic','emission']):
   texture=tex(M['decals'][0] if decal else default,role if role!='emission' else 'roughness')
   if world:
    obj=node(m,u.MaterialExpressionTextureObjectParameter,parameter_name=role,texture=texture);fn=node(m,u.MaterialExpressionMaterialFunctionCall);assert fn.set_material_function(u.load_asset('/Engine/Functions/Engine_MaterialFunctions01/Texturing/'+('WorldAlignedNormal' if role=='normal' else 'WorldAlignedTexture')))
    inputs=list(E.get_material_expression_input_names(fn));link(obj,'',fn,next(n for n in inputs if n.lower().startswith('textureobject')));link(size,'',fn,next(n for n in inputs if n.lower().startswith('texturesize')))
    if role=='normal':b=node(m,u.MaterialExpressionStaticBool,value=True);link(b,'',fn,'WorldSpace')
    output=next(n for n in E.get_material_expression_output_names(fn) if 'xyz' in n.lower());samples[role]=(fn,output)
   else:
    n=node(m,u.MaterialExpressionTextureSampleParameter2D,parameter_name=role,texture=texture,sampler_type=u.MaterialSamplerType.SAMPLERTYPE_COLOR if role=='base_color' else u.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='normal' else u.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE);link(uvmul,'',n,'UVs');samples[role]=(n,'RGB' if role in ['base_color','normal'] else 'R')
  tint=node(m,u.MaterialExpressionVectorParameter,parameter_name='Tint',default_value=u.LinearColor(1,1,1,1));bc=node(m,u.MaterialExpressionMultiply);link(*samples['base_color'],bc,'A');link(tint,'',bc,'B');wet=scalar(m,'Wetness',0);dark=node(m,u.MaterialExpressionMultiply,const_b=.25);link(wet,'',dark,'A');one=node(m,u.MaterialExpressionOneMinus);link(dark,'',one,'');bc2=node(m,u.MaterialExpressionMultiply);link(bc,'',bc2,'A');link(one,'',bc2,'B');prop(bc2,'',u.MaterialProperty.MP_BASE_COLOR)
  rough=node(m,u.MaterialExpressionMultiply);link(*samples['roughness'],rough,'A');link(scalar(m,'Roughness',1),'',rough,'B');lerp=node(m,u.MaterialExpressionLinearInterpolate,const_b=.30);link(rough,'',lerp,'A');link(wet,'',lerp,'Alpha');clamp=node(m,u.MaterialExpressionClamp,min_default=.30,max_default=1);link(lerp,'',clamp,'');prop(clamp,'',u.MaterialProperty.MP_ROUGHNESS)
  if decal:
   alpha=node(m,u.MaterialExpressionMultiply);link(samples['base_color'][0],'A',alpha,'A');link(scalar(m,'Opacity',.85),'',alpha,'B');prop(alpha,'',u.MaterialProperty.MP_OPACITY)
  else:
   flat=node(m,u.MaterialExpressionVertexNormalWS) if world else node(m,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(0,0,1));mix=node(m,u.MaterialExpressionLinearInterpolate);link(flat,'',mix,'A');link(*samples['normal'],mix,'B');link(scalar(m,'DetailStrength',.8),'',mix,'Alpha');norm=node(m,u.MaterialExpressionNormalize);link(mix,'',norm,'');prop(norm,'',u.MaterialProperty.MP_NORMAL)
   met=node(m,u.MaterialExpressionMultiply);link(*samples['metallic'],met,'A');link(scalar(m,'Metallic',0),'',met,'B');prop(met,'',u.MaterialProperty.MP_METALLIC)
   em=node(m,u.MaterialExpressionMultiply);link(*samples['emission'],em,'A');link(scalar(m,'EmissionStrength',0),'',em,'B');ec=node(m,u.MaterialExpressionVectorParameter,parameter_name='EmissionTint',default_value=u.LinearColor(1,.13,.015,1));emc=node(m,u.MaterialExpressionMultiply);link(em,'',emc,'A');link(ec,'',emc,'B');prop(emc,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
   E.set_material_usage(m,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
  assert not E.recompile_material(m);A.save_loaded_asset(m);return m
 uvparent=parent('M_IdentityUV');worldparent=parent('M_IdentityWorld',True)
 for a in M['materials']+M['decals']:
  for prefix,par in ([('MI_',uvparent)] if a['role']=='decal' else [('MI_',worldparent),('MI_UV_',uvparent)]):
   mi=make(prefix+a['id'],u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew());E.set_material_instance_parent(mi,par)
   for role in a['maps']:
    E.set_material_instance_texture_parameter_value(mi,role,tex(a,role))
    actual=E.get_material_instance_texture_parameter_value(mi,role);assert actual and actual.get_path_name()==tex(a,role).get_path_name(),(a['id'],prefix,role,str(actual))
   E.set_material_instance_scalar_parameter_value(mi,'Metallic',1 if 'metallic' in a['maps'] else 0);E.set_material_instance_scalar_parameter_value(mi,'EmissionStrength',80.0 if 'emission' in a['maps'] else 0);E.update_material_instance(mi);A.save_loaded_asset(mi)
else:
 for a in M['props']:
  t=u.AssetImportTask();t.filename=str(R/a['fbx']);t.destination_path=ROOT+'/Meshes';t.destination_name=a['id'];t.automated=True;t.replace_existing=True;t.save=True;opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;opt.static_mesh_import_data.generate_lightmap_u_vs=False;t.options=opt;T.import_asset_tasks([t]);mesh=u.load_asset(ROOT+'/Meshes/'+a['id']);assert mesh,a['id']
  for i,s in enumerate(mesh.get_editor_property('static_materials')):
   n=str(s.get_editor_property('imported_material_slot_name'));mi=u.load_asset(ROOT+'/Materials/MI_UV_'+n);assert mi,n;mesh.set_material(i,mi)
  A.save_loaded_asset(mesh);b=mesh.get_bounding_box();sz=b.max-b.min;assert all(abs(x-y*100)<3 for x,y in zip(sorted([sz.x,sz.y,sz.z]),sorted(a['size_m']))),a['id'];report['meshes'].append({'id':a['id'],'size_cm':[sz.x,sz.y,sz.z]})
report['status']='PASS';(R/'reports'/('unreal_'+stage.lower()+'.json')).write_text(json.dumps(report,indent=2));print('REGIONAL_IMPORT_PASS',stage,flush=True)
