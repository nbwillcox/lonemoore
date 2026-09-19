"""Import the isolated Blender room kit with five fixed role slots and Nanite.

Run only in the full editor after import_room_kit_materials.py. Collision remains
owned by the validated tile/socket contract, never automatic convex room hulls.
"""
import unreal as u
import json
from pathlib import Path

ROOT=Path(u.Paths.project_dir()).resolve()
OUT=ROOT/'Saved/RoomKitPass';OUT.mkdir(parents=True,exist_ok=True)
ROLES=['Wall','Floor','Vault','Trim','Iron']
editor=u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
lib=u.EditorAssetLibrary
materials={role:u.load_asset('/Game/RoomKit/Materials/MI_RK_Crypts_'+role) for role in ROLES}
assert all(materials.values()), 'Run room kit material import first'
records=[]
only_curved='RoomKitCurvedOnly' in u.SystemLibrary.get_command_line()
only_turns='RoomKitTurnsOnly' in u.SystemLibrary.get_command_line()
for fbx in sorted((ROOT/'ArtSource/RoomKit/Meshes').glob('SM_RK_*.fbx')):
    name=fbx.stem;path='/Game/RoomKit/Meshes/'+name
    if only_curved and name not in ['SM_RK_circular_ossuary','SM_RK_rounded_turn','SM_RK_cavern_hall']:continue
    if only_turns and name not in ['SM_RK_short_turn','SM_RK_rounded_turn']:continue
    # This is the newly authored RoomKit namespace only. A clean import avoids
    # Interchange retaining obsolete FBX material names after partial rebuilds.
    # Runtime references are stable asset paths; original regional assets remain.
    if lib.does_asset_exist(path):assert lib.delete_asset(path),f'Cannot replace room mesh {path}'
    task=u.AssetImportTask();task.filename=str(fbx)
    task.destination_path='/Game/RoomKit/Meshes';task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=False
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False
    opt.import_as_skeletal=False;opt.mesh_type_to_import=u.FBXImportType.FBXIT_STATIC_MESH
    data=opt.static_mesh_import_data;data.combine_meshes=True;data.auto_generate_collision=False
    data.generate_lightmap_u_vs=False;data.import_uniform_scale=1
    data.normal_import_method=u.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
    task.options=opt
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=u.load_asset(path);assert mesh, path
    slots=list(mesh.static_materials)
    # FBX drops unused roles. Preserve section meaning by remapping named source
    # slots to the fixed runtime contract; pad metadata, never hidden triangles.
    observed=[str(slot.material_slot_name).split('.')[0] for slot in slots]
    source_roles=[]
    for slot in observed:
        role=next((r for r in ROLES if slot==r or slot.startswith(r+'_')),None)
        assert role is not None,f'{name}: unknown role {slot}'
        source_roles.append(role)
    section_roles=[source_roles[editor.get_lod_material_slot(mesh,0,section)] for section in range(mesh.get_num_sections(0))]
    source_slots={role:slots[index] for index,role in enumerate(source_roles)}
    padded=[]
    for role in ROLES:
        # Retain imported UV density/streaming metadata for every used role.
        slot=source_slots.get(role,u.StaticMaterial());slot.material_interface=materials[role];slot.material_slot_name=role;padded.append(slot)
    mesh.set_editor_property('static_materials',padded)
    for section,role in enumerate(section_roles):
        index=ROLES.index(role)
        if editor.get_lod_material_slot(mesh,0,section)!=index:editor.set_lod_material_slot(mesh,index,0,section)
    settings=editor.get_nanite_settings(mesh);settings.enabled=True
    settings.fallback_percent_triangles=1.;settings.fallback_relative_error=0.
    editor.set_nanite_settings(mesh,settings,True)
    mesh.set_editor_property('never_stream',False)
    assert lib.save_loaded_asset(mesh)
    final_slots=[str(slot.material_slot_name) for slot in mesh.static_materials]
    assert final_slots==ROLES,(name,'role order',final_slots)
    final_sections=[editor.get_lod_material_slot(mesh,0,section) for section in range(mesh.get_num_sections(0))]
    assert final_sections==[ROLES.index(role) for role in section_roles],(name,'section role mismatch',section_roles,final_sections)
    if name!='SM_RK_portal_cap':assert all(index in final_sections for index in (0,1,2)),(name,'missing wall/floor/vault sections')
    box=mesh.get_bounding_box();size=box.max-box.min
    if name in ('SM_RK_short_turn','SM_RK_rounded_turn'):
        assert box.min.x>-300 and box.max.x>1390 and box.min.y<-1390 and box.max.y<300, f'Unexpected Blender/Unreal handedness: {box}'
    records.append(dict(id=name,asset=path,nanite=True,slots=ROLES,importedSlots=observed,sections=final_sections,sectionRoles=section_roles,boundsCm=[size.x,size.y,size.z],minCm=[box.min.x,box.min.y,box.min.z],maxCm=[box.max.x,box.max.y,box.max.z],collision='Runtime logical boundaries'))
    print('ROOM_KIT_IMPORT_MESH',name)
assert len(records)==(2 if only_turns else 3 if only_curved else 26)
if only_curved or only_turns:
    previous=json.loads((OUT/'geometry_import.json').read_text())['meshes']
    updated={r['id']:r for r in records};records=[updated.get(r['id'],r) for r in previous]
(OUT/'geometry_import.json').write_text(json.dumps(dict(meshes=records,roomCount=25,failures=0),indent=2))
print('ROOM_KIT_IMPORT_COMPLETE rooms=25 meshes=26 failures=0')
u.SystemLibrary.quit_editor()
