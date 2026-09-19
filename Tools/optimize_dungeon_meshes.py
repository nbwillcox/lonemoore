"""Build Nanite on detailed opaque dungeon meshes and an async scene manifest."""
import unreal as u,json,shutil
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve();out=r/'Saved/PerformancePass';a=u.EditorAssetLibrary
editor=u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
roots=['/Game/Game/Environment','/Game/OldCitySewers/Meshes','/Game/RegionalArt/Meshes','/Game/RegionalIdentity/Meshes','/Game/ExpansionPrototype/Meshes','/Game/SharedInteractables/Meshes']
# Hero stairs appear once per floor. Keep their authored bevels and shallow inlays
# in the ordinary renderer; Nanite simplification visibly damages these details.
report=[]
for root in roots:
    for path in a.list_assets(root,recursive=True,include_folder=False):
        mesh=u.load_asset(path)
        if not isinstance(mesh,u.StaticMesh):continue
        verts=editor.get_number_verts(mesh,0)
        if verts<128:continue
        # Transparent effects and sprite planes continue through their existing renderer.
        opaque=True
        for slot in mesh.get_editor_property('static_materials'):
            mat=slot.get_editor_property('material_interface')
            while isinstance(mat,u.MaterialInstance):mat=mat.get_editor_property('parent')
            if mat and mat.get_editor_property('blend_mode') not in [u.BlendMode.BLEND_OPAQUE,u.BlendMode.BLEND_MASKED]:opaque=False
        if not opaque:continue
        source=r/'Content'/(path.split('.')[0].removeprefix('/Game/')+'.uasset')
        backup=out/'Backup/NaniteAssets'/source.relative_to(r/'Content');backup.parent.mkdir(parents=True,exist_ok=True)
        if not backup.exists():shutil.copy2(source,backup)
        settings=editor.get_nanite_settings(mesh);settings.enabled=True
        settings.fallback_percent_triangles=1.0
        settings.fallback_relative_error=0.0
        editor.set_nanite_settings(mesh,settings,True);assert a.save_loaded_asset(mesh)
        report.append({'asset':path,'vertices':verts,'nanite':True})
        print('DUNGEON_NANITE',path,verts)
(out/'nanite_assets.json').write_text(json.dumps(report,indent=2))

common=[]
for root in ['/Game/Game/Environment','/Game/Game/Materials','/Game/ExpansionPrototype/Meshes','/Game/ExpansionPrototype/Materials','/Game/SharedInteractables/Meshes','/Game/CampaignExpansion/Materials']:
    common.extend(a.list_assets(root,recursive=True,include_folder=False))
codes=['Cathedral','Sewer','Catacombs','Warrens','Crypts','Fortress','Deep','Infernal','Hell'];regions=[]
for i,code in enumerate(codes):
    assets=[]
    if i==1:
        for root in ['/Game/OldCitySewers/Meshes','/Game/OldCitySewers/Materials']:assets.extend(a.list_assets(root,recursive=True,include_folder=False))
    else:
        for root in ['/Game/RegionalArt/Meshes','/Game/RegionalArt/Materials','/Game/RegionalIdentity/Meshes']:
            assets.extend(p for p in a.list_assets(root,recursive=True,include_folder=False) if code+'_' in p or '/SM_Key_' in p)
    regions.append(sorted(set(assets)))
(r/'Content/Game/Data/scene_streaming.json').write_text(json.dumps({'common':sorted(set(common)),'regions':regions},indent=2))
print('DUNGEON_OPTIMIZED_ASSETS_COMPLETE',len(report))
u.SystemLibrary.quit_editor()
