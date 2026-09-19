"""Integrate owner-approved OCS assets in the actual project; preserve old regional materials."""
import unreal as u, pathlib,json
R=pathlib.Path(u.Paths.project_dir()).resolve();assert R==pathlib.Path(r'J:\First Person Dungeon Crawler Game')
D='/Game/OldCitySewers';E=u.MaterialEditingLibrary;A=u.EditorAssetLibrary;T=u.AssetToolsHelpers.get_asset_tools();report={'errors':[],'materials':[]}
def node(m,cls,x,y,**kw):
 n=E.create_material_expression(m,cls,x,y)
 for k,v in kw.items():n.set_editor_property(k,v)
 return n
def link(a,ao,b,bi):assert E.connect_material_expressions(a,ao,b,bi),(a.get_name(),ao,b.get_name(),bi)
def prop(a,ao,p):assert E.connect_material_property(a,ao,p)
def scalar(m,k,v,x,y):return node(m,u.MaterialExpressionScalarParameter,x,y,parameter_name=k,default_value=v)
def made(name,cls,factory):return u.load_asset(D+'/Materials/'+name) or T.create_asset(name,D+'/Materials',cls,factory)
def tex(name,role):return u.load_asset(D+'/Textures/T_'+name+'_'+role)
# New parent: the generated modules are scaled, so physical world projection keeps a 2m repeat.
m=made('M_OCS_WorldSurface',u.Material,u.MaterialFactoryNew());E.delete_all_material_expressions(m);m.set_editor_property('tangent_space_normal',False)
size=node(m,u.MaterialExpressionVectorParameter,-1100,-100,parameter_name='TextureSizeCm',default_value=u.LinearColor(200,200,200,1))
samples={}
for i,(role,suffix) in enumerate([('base_color','BC'),('normal','N'),('roughness','R'),('metallic','M')]):
 obj=node(m,u.MaterialExpressionTextureObjectParameter,-1100,200+i*300,parameter_name=role,texture=tex('CorrodedIron',suffix))
 call=node(m,u.MaterialExpressionMaterialFunctionCall,-700,200+i*300)
 fn='WorldAlignedNormal' if role=='normal' else 'WorldAlignedTexture'
 assert call.set_material_function(u.load_asset('/Engine/Functions/Engine_MaterialFunctions01/Texturing/'+fn))
 inputs=list(E.get_material_expression_input_names(call));outputs=list(E.get_material_expression_output_names(call));print('OCS_FUNCTION',fn,inputs,outputs)
 link(obj,'',call,next(n for n in inputs if n.lower().startswith('textureobject')));link(size,'',call,next(n for n in inputs if n.lower().startswith('texturesize')))
 if role=='normal':
  world=node(m,u.MaterialExpressionStaticBool,-900,650,value=True);link(world,'',call,'WorldSpace')
 output=next(n for n in outputs if 'xyz' in n.lower());samples[role]=(call,output)
tint=node(m,u.MaterialExpressionVectorParameter,-450,-100,parameter_name='Tint',default_value=u.LinearColor(1,1,1,1));bc=node(m,u.MaterialExpressionMultiply,-200,100);link(*samples['base_color'],bc,'A');link(tint,'',bc,'B')
wet=scalar(m,'Wetness',0,-450,1450);dark=node(m,u.MaterialExpressionMultiply,-200,1450,const_b=.3);link(wet,'',dark,'A');one=node(m,u.MaterialExpressionOneMinus,0,1450);link(dark,'',one,'');finalbc=node(m,u.MaterialExpressionMultiply,220,100);link(bc,'',finalbc,'A');link(one,'',finalbc,'B');prop(finalbc,'',u.MaterialProperty.MP_BASE_COLOR)
rough=node(m,u.MaterialExpressionComponentMask,-450,750,r=True);link(*samples['roughness'],rough,'');rm=scalar(m,'Roughness',1,-450,900);rr=node(m,u.MaterialExpressionMultiply,-200,750);link(rough,'',rr,'A');link(rm,'',rr,'B');blend=node(m,u.MaterialExpressionLinearInterpolate,0,750,const_b=.26);link(rr,'',blend,'A');link(wet,'',blend,'Alpha');prop(blend,'',u.MaterialProperty.MP_ROUGHNESS)
vertex=node(m,u.MaterialExpressionVertexNormalWS,-450,450);strength=scalar(m,'DetailStrength',.65,-450,580);mix=node(m,u.MaterialExpressionLinearInterpolate,-150,450);link(vertex,'',mix,'A');link(*samples['normal'],mix,'B');link(strength,'',mix,'Alpha');norm=node(m,u.MaterialExpressionNormalize,100,450);link(mix,'',norm,'');prop(norm,'',u.MaterialProperty.MP_NORMAL)
metal=node(m,u.MaterialExpressionComponentMask,-450,1100,r=True);link(*samples['metallic'],metal,'');mm=scalar(m,'Metallic',0,-450,1250);mul=node(m,u.MaterialExpressionMultiply,-150,1100);link(metal,'',mul,'A');link(mm,'',mul,'B');prop(mul,'',u.MaterialProperty.MP_METALLIC)
E.set_material_usage(m,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);err=E.recompile_material(m);assert not err,str(err);A.save_loaded_asset(m)
manifest=json.loads((R/'ArtReview/OldCitySewers/manifest.json').read_text())
for a in manifest['materials']:
 mi=made('MI_Game_'+a['id'],u.MaterialInstanceConstant,u.MaterialInstanceConstantFactoryNew());E.set_material_instance_parent(mi,m)
 for key,path in a['maps'].items():E.set_material_instance_texture_parameter_value(mi,key,u.load_asset(D+'/Textures/T_'+pathlib.Path(path).stem))
 E.set_material_instance_scalar_parameter_value(mi,'Metallic',1 if a['role']=='metal' else 0);E.set_material_instance_scalar_parameter_value(mi,'DetailStrength',.4 if a['role']=='water' else .65);E.update_material_instance(mi);A.save_loaded_asset(mi);report['materials'].append(mi.get_path_name())
# UV parent stays at the sample's correct density on the three authored unscaled props.
base=u.load_asset(D+'/Materials/M_OCS_Surface');E.set_material_usage(base,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES);assert not E.recompile_material(base);A.save_loaded_asset(base)
for p in A.list_assets(D+'/Textures',recursive=False):
 t=u.load_asset(p);t.set_editor_property('never_stream',False);A.save_loaded_asset(t)
report['world_repeat_cm']=200;report['streaming']=True;report['status']='PASS';(R/'Saved/SewerArtIntegration/import_report.json').write_text(json.dumps(report,indent=2));print('OCS_MAIN_ART_PREPARED')

