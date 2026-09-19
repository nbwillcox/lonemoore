import unreal as u
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve();tools=u.AssetToolsHelpers.get_asset_tools();report=[]
for p in sorted((r/'ArtReview/ExpansionPrototype').glob('*.fbx')):
    task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/ExpansionPrototype/Meshes';task.destination_name='SM_'+p.stem;task.automated=True;task.replace_existing=True;task.save=True
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False
    opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;opt.static_mesh_import_data.generate_lightmap_u_vs=False;task.options=opt
    tools.import_asset_tasks([task]);mesh=u.load_asset(task.destination_path+'/'+task.destination_name);assert mesh,p
    role='CorrodedIron' if p.stem in ('BridgeRail','PumpWheel') else 'MaintenanceFlags' if p.stem=='BridgeDeck' else 'ReservoirAshlar'
    mesh.set_material(0,u.load_asset('/Game/OldCitySewers/Materials/MI_Game_'+role));u.EditorAssetLibrary.save_loaded_asset(mesh)
    box=mesh.get_bounding_box();report.append(f'{p.stem}: {box.max-box.min}')
assert len(report)==5
(r/'Saved/ExpansionPrototype/import_report.txt').write_text('\n'.join(report));print('EXPANSION_IMPORT_PASS',report)
