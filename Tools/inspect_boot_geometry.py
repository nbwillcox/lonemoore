import unreal as u
u.get_editor_subsystem(u.LevelEditorSubsystem).load_level('/Game/Game/Maps/Boot')
for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
    if isinstance(a,u.StaticMeshActor):
        c=a.static_mesh_component
        print('BOOT_MESH',a.get_name(),c.static_mesh.get_name() if c.static_mesh else '',a.get_actor_location(),c.get_collision_enabled())
