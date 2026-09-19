"""Import the two repaired wall meshes, retaining materials and Nanite batching."""
import unreal as u, shutil, json
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve(); out=r/'Saved/FineTunePass'
a=u.EditorAssetLibrary; editor=u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
report=[]
for part in ['Wall','Niche']:
    name='SM_ID_Warrens_'+part; path='/Game/RegionalIdentity/Meshes/'+name
    original=r/'Content/RegionalIdentity/Meshes'/(name+'.uasset')
    backup=out/'Backup/Meshes'/original.name; backup.parent.mkdir(parents=True,exist_ok=True)
    if not backup.exists():shutil.copy2(original,backup)
    old=u.load_asset(path); mats=[s.material_interface for s in old.static_materials]
    task=u.AssetImportTask();task.filename=str(out/'Geometry'/(name+'.fbx'))
    task.destination_path='/Game/RegionalIdentity/Meshes';task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=False
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False
    opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False
    opt.static_mesh_import_data.generate_lightmap_u_vs=False;task.options=opt
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=u.load_asset(path);assert mesh
    assert len(mesh.static_materials)==len(mats)
    for i,mat in enumerate(mats):mesh.set_material(i,mat)
    settings=editor.get_nanite_settings(mesh);settings.enabled=True
    settings.fallback_percent_triangles=1.;settings.fallback_relative_error=0.
    editor.set_nanite_settings(mesh,settings,True)
    assert a.save_loaded_asset(mesh)
    report.append(dict(mesh=path,nanite=True,materials=[m.get_path_name() for m in mats]))
(out/'geometry_import.json').write_text(json.dumps(report,indent=2))
print('FINETUNE_GEOMETRY_IMPORT_COMPLETE')
u.SystemLibrary.quit_editor()
