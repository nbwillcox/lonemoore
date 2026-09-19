import unreal as u,json
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Identity');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve()
G=Path(r'J:\First Person Dungeon Crawler Game');T=u.AssetToolsHelpers.get_asset_tools();report=[]
for name,role in [('Wall','ReservoirAshlar'),('Arch','LimestoneCoping'),('Floor','MaintenanceFlags'),('Vault','VaultBrick')]:
 task=u.AssetImportTask();task.filename=str(G/'ArtReview/IntegrityKit'/('I_'+name+'.fbx'));task.destination_path='/Game/RegionalIdentityBenchmark';task.destination_name='SM_Bench_'+name;task.automated=True;task.replace_existing=True;task.save=True
 opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;task.options=opt;T.import_asset_tasks([task]);m=u.load_asset(task.destination_path+'/'+task.destination_name);assert m
 m.set_material(0,u.load_asset('/Game/OldCitySewers/Materials/MI_Game_'+role));u.EditorAssetLibrary.save_loaded_asset(m);report.append(m.get_path_name())
(R/'reports/benchmark_import.json').write_text(json.dumps(report,indent=2));print('BENCHMARK_IMPORT_PASS')
