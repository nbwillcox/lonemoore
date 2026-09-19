"""Import authored Blender modules with real triangle collision and region materials."""
import unreal as u, pathlib, json
root=pathlib.Path(u.Paths.project_dir());dest='/Game/Game/Environment'
tools=u.AssetToolsHelpers.get_asset_tools();spec=[]
for p in sorted((root/'ArtReview/IntegrityKit').glob('*.fbx')):
    task=u.AssetImportTask();task.filename=str(p);task.destination_path=dest;task.destination_name='SM_'+p.stem
    task.automated=True;task.replace_existing=True;task.save=True
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False
    opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.generate_lightmap_u_vs=False;opt.static_mesh_import_data.auto_generate_collision=False
    task.options=opt;tools.import_asset_tasks([task])
    mesh=u.load_asset(dest+'/SM_'+p.stem)
    if not mesh:raise RuntimeError('Import missing '+str(p))
    body=mesh.get_editor_property('body_setup');body.set_editor_property('collision_trace_flag',u.CollisionTraceFlag.CTF_USE_COMPLEX_AS_SIMPLE)
    bounds=mesh.get_bounding_box();size=bounds.max-bounds.min
    spec.append(dict(name=p.stem,size_cm=[size.x,size.y,size.z]))
    u.EditorAssetLibrary.save_loaded_asset(mesh)
def material(name,color,rough=.85,glow=False):
    m=u.load_asset('/Game/Game/Materials/M_'+name) or tools.create_asset('M_'+name,'/Game/Game/Materials',u.Material,u.MaterialFactoryNew())
    u.MaterialEditingLibrary.delete_all_material_expressions(m)
    c=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionVectorParameter,0,0);c.set_editor_property('parameter_name','Tint');c.set_editor_property('default_value',u.LinearColor(*color,1))
    noise=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionNoise,0,200);noise.set_editor_property('scale',.18);noise.set_editor_property('levels',2);noise.set_editor_property('output_min',.82);noise.set_editor_property('output_max',1.08)
    bias=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionAdd,200,200);bias.set_editor_property('const_b',0)
    mul=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionMultiply,400,0)
    u.MaterialEditingLibrary.connect_material_expressions(noise,'',bias,'A');u.MaterialEditingLibrary.connect_material_expressions(bias,'',mul,'B');u.MaterialEditingLibrary.connect_material_expressions(c,'',mul,'A')
    u.MaterialEditingLibrary.connect_material_property(mul,'',u.MaterialProperty.MP_BASE_COLOR)
    v=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionConstant,0,350);v.set_editor_property('r',rough);u.MaterialEditingLibrary.connect_material_property(v,'',u.MaterialProperty.MP_ROUGHNESS)
    if glow:u.MaterialEditingLibrary.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    u.MaterialEditingLibrary.set_material_usage(m,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    u.MaterialEditingLibrary.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)
for name,color,rough in [('I_Limestone',(.32,.29,.23),.88),('I_Sewer',(.10,.17,.13),.35),('I_Crypt',(.23,.23,.26),.84),('I_Fortress',(.16,.19,.22),.75),('I_Infernal',(.16,.07,.055),.8),('I_Bone',(.42,.38,.27),.75),('I_Rust',(.18,.08,.025),.64),('I_Water',(.025,.09,.07),.15),('I_Soot',(.025,.023,.02),.99)]:material(name,color,rough)
material('I_Ember',(2.2,.55,.08),.8,True)
material('I_Fungus',(.08,.28,.18),.7,True)
(root/'Saved/IntegrityUpdate/import_bounds.json').write_text(json.dumps(spec,indent=2))
assert abs(next(x for x in spec if x['name']=='I_Floor')['size_cm'][0]-400)<2
assert 300<next(x for x in spec if x['name']=='I_Door')['size_cm'][0]<310
print('INTEGRITY_IMPORT_VERIFIED',len(spec),'meshes; 400cm grid; 302cm door')
