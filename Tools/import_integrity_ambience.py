import unreal as u, pathlib
root=pathlib.Path(u.Paths.project_dir())
for p in (root/'ArtReview/IntegrityKit/Audio').glob('*.wav'):
    t=u.AssetImportTask();t.filename=str(p);t.destination_path='/Game/Game/Audio';t.destination_name='A_'+p.stem;t.automated=True;t.replace_existing=True;t.save=True
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);sound=u.load_asset('/Game/Game/Audio/A_'+p.stem);sound.set_editor_property('looping',True);u.EditorAssetLibrary.save_loaded_asset(sound)
print('INTEGRITY_AMBIENCE_IMPORTED')
