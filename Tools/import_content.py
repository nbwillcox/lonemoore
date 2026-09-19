"""Run with UnrealEditor-Cmd -run=pythonscript -script=<this path>."""
import unreal as u, pathlib,json
root=pathlib.Path(u.Paths.project_dir())
tools=u.AssetToolsHelpers.get_asset_tools()
for folder in ['ImportedArt','Environment','Materials','Maps','Audio']:u.EditorAssetLibrary.make_directory('/Game/Game/'+folder)
manifest=[];tasks=[]
for p in sorted((root/'.art').rglob('*.png')):
    if 'concept' in p.stem.lower() or p.stem.lower().endswith('_bak'):continue
    name='T_'+p.stem
    if not u.EditorAssetLibrary.does_asset_exist('/Game/Game/ImportedArt/'+name):
        task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/Game/ImportedArt';task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True;tasks.append(task)
    manifest.append(dict(source=str(p.relative_to(root)),asset='/Game/Game/ImportedArt/'+name+'.'+name))
tools.import_asset_tasks(tasks)
for entry in manifest:
    tex=u.load_asset(entry['asset'])
    if tex:
        tex.set_editor_property('max_texture_size',2048 if 'locations' in entry['source'] else 1024)
        tex.set_editor_property('never_stream',True)
        tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_EDITOR_ICON)
        u.EditorAssetLibrary.save_loaded_asset(tex)
(root/'Content/Game/Data/asset_manifest.json').write_text(json.dumps(manifest,indent=2))
tasks=[]
for p in (root/'ArtReview/ModularKit').glob('*.fbx'):
    if u.EditorAssetLibrary.does_asset_exist('/Game/Game/Environment/SM_'+p.stem):continue
    task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/Game/Environment';task.destination_name='SM_'+p.stem;task.automated=True;task.replace_existing=True;task.save=True
    options=u.FbxImportUI();options.import_mesh=True;options.import_materials=False;options.import_textures=False;options.import_as_skeletal=False
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.generate_lightmap_u_vs=False;options.static_mesh_import_data.auto_generate_collision=True
    task.options=options;tasks.append(task)
tools.import_asset_tasks(tasks)
def mat(name,color,emissive=False):
    path='/Game/Game/Materials/'+name
    m=u.load_asset(path) or tools.create_asset(name,'/Game/Game/Materials',u.Material,u.MaterialFactoryNew())
    u.MaterialEditingLibrary.delete_all_material_expressions(m)
    node=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionVectorParameter,0,0);node.set_editor_property('parameter_name','Tint');node.set_editor_property('default_value',u.LinearColor(*color,1))
    u.MaterialEditingLibrary.connect_material_property(node,'',u.MaterialProperty.MP_BASE_COLOR)
    if emissive:u.MaterialEditingLibrary.connect_material_property(node,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    rough=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionConstant,0,100);rough.set_editor_property('r',.86);u.MaterialEditingLibrary.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    u.MaterialEditingLibrary.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)
mat('M_Stone',(.3,.27,.22));mat('M_Metal',(.08,.09,.1));mat('M_Wood',(.15,.075,.025));mat('M_Glow',(.2,2.0,4.0),True)
path='/Game/Game/Materials/M_Billboard';m=u.load_asset(path) or tools.create_asset('M_Billboard','/Game/Game/Materials',u.Material,u.MaterialFactoryNew())
u.MaterialEditingLibrary.delete_all_material_expressions(m);m.set_editor_property('blend_mode',u.BlendMode.BLEND_MASKED);m.set_editor_property('two_sided',True);m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
n=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionTextureSampleParameter2D,0,0);n.set_editor_property('parameter_name','Portrait');n.set_editor_property('texture',u.load_asset('/Game/Game/ImportedArt/T_rat1'))
u.MaterialEditingLibrary.connect_material_property(n,'RGB',u.MaterialProperty.MP_EMISSIVE_COLOR);u.MaterialEditingLibrary.connect_material_property(n,'A',u.MaterialProperty.MP_OPACITY_MASK)
u.MaterialEditingLibrary.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m)
audio=[]
for p in (root/'ArtReview/Audio').glob('*.wav'):
    task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/Game/Audio';task.destination_name='A_'+p.stem;task.automated=True;task.replace_existing=True;task.save=True;audio.append(task)
tools.import_asset_tasks(audio)
for name in ['town','dungeon','combat','boss','ending']:
    sound=u.load_asset('/Game/Game/Audio/A_'+name)
    if sound:sound.set_editor_property('looping',True);u.EditorAssetLibrary.save_loaded_asset(sound)
sub=u.get_editor_subsystem(u.LevelEditorSubsystem)
sub.new_level('/Game/Game/Maps/Boot')
sub.save_current_level()
u.EditorAssetLibrary.save_directory('/Game/Game',only_if_is_dirty=True,recursive=True)
print('DUNGEON_IMPORT_COMPLETE',len(manifest))
