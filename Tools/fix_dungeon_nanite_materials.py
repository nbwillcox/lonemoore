import unreal as u, json, shutil
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve();out=r/'Saved/PerformancePass';a=u.EditorAssetLibrary
paths=a.list_assets('/Game',recursive=True,include_folder=False)
changed=[]
for path in paths:
    if '/Materials/' not in path:continue
    mat=u.load_asset(path)
    if not isinstance(mat,u.Material):continue
    if mat.get_editor_property('blend_mode') not in [u.BlendMode.BLEND_OPAQUE,u.BlendMode.BLEND_MASKED]:continue
    if mat.get_editor_property('used_with_nanite'):continue
    source=r/'Content'/(path.split('.')[0].removeprefix('/Game/')+'.uasset')
    backup=out/'Backup/NaniteMaterials'/source.relative_to(r/'Content');backup.parent.mkdir(parents=True,exist_ok=True)
    if not backup.exists():shutil.copy2(source,backup)
    u.MaterialEditingLibrary.set_material_usage(mat,u.MaterialUsage.MATUSAGE_NANITE)
    assert mat.get_editor_property('used_with_nanite')
    assert a.save_loaded_asset(mat)
    changed.append(path)
report=json.loads((out/'nanite_assets.json').read_text())
report=[x for x in report if '/CampaignExpansion/Meshes/' not in x['asset']]
(out/'nanite_assets.json').write_text(json.dumps(report,indent=2))
(out/'nanite_materials.json').write_text(json.dumps(changed,indent=2))
print('DUNGEON_NANITE_MATERIALS_READY',len(changed),'meshes',len(report))
u.SystemLibrary.quit_editor()
