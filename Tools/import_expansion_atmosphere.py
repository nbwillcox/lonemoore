import unreal as u, json
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve();t=u.AssetToolsHelpers.get_asset_tools();a=u.EditorAssetLibrary;e=u.MaterialEditingLibrary
materials={'Oak':u.load_asset('/Game/SharedInteractables/Materials/M_Oak'),'Iron':u.load_asset('/Game/OldCitySewers/Materials/MI_Game_CorrodedIron'),'Stone':u.load_asset('/Game/OldCitySewers/Materials/MI_Game_ReservoirAshlar')}
report=[]
for p in sorted((r/'ArtReview/ExpansionPrototype/AtmosphereRevision').glob('*.fbx')):
    task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/ExpansionPrototype/Meshes';task.destination_name='SM_'+p.stem;task.automated=True;task.replace_existing=True;task.save=True
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;opt.static_mesh_import_data.generate_lightmap_u_vs=False;task.options=opt
    t.import_asset_tasks([task]);mesh=u.load_asset(task.destination_path+'/'+task.destination_name);assert mesh
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('imported_material_slot_name'));key=next(k for k in materials if k in name);assert materials[key];mesh.set_material(i,materials[key])
    a.save_loaded_asset(mesh);report.append(mesh.get_path_name())
assert len(report)==3
# Existing portraits remain untouched; this material is only used by world enemies in the prototype.
path='/Game/ExpansionPrototype/Materials';m=u.load_asset(path+'/M_ExpansionEnemy') or t.create_asset('M_ExpansionEnemy',path,u.Material,u.MaterialFactoryNew())
e.delete_all_material_expressions(m);m.set_editor_property('blend_mode',u.BlendMode.BLEND_MASKED);m.set_editor_property('two_sided',True);m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
tex=e.create_material_expression(m,u.MaterialExpressionTextureSampleParameter2D,-500,0);tex.set_editor_property('parameter_name','Portrait');tex.set_editor_property('texture',u.load_asset('/Game/Game/ImportedArt/T_rat1'))
level=e.create_material_expression(m,u.MaterialExpressionScalarParameter,-500,200);level.set_editor_property('parameter_name','WorldLight');level.set_editor_property('default_value',.55)
mul=e.create_material_expression(m,u.MaterialExpressionMultiply,-200,0);e.connect_material_expressions(tex,'RGB',mul,'A');e.connect_material_expressions(level,'',mul,'B');e.connect_material_property(mul,'',u.MaterialProperty.MP_EMISSIVE_COLOR);e.connect_material_property(tex,'A',u.MaterialProperty.MP_OPACITY_MASK)
e.recompile_material(m);a.save_loaded_asset(m)
(r/'Saved/ExpansionAtmosphere/import_report.json').write_text(json.dumps(report+[m.get_path_name()],indent=2));print('ATMOSPHERE_IMPORT_PASS')
