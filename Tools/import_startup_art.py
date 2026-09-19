"""Import the supplied loading screen; leave .art originals untouched."""
import pathlib
import unreal as u

root = pathlib.Path(u.Paths.project_dir())
task = u.AssetImportTask()
task.filename = str(root / '.art' / 'loadingscreen.png')
task.destination_path = '/Game/Game/ImportedArt'
task.destination_name = 'T_loadingscreen'
task.automated = True
task.replace_existing = True
task.save = True
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = u.load_asset('/Game/Game/ImportedArt/T_loadingscreen')
if not texture:
    raise RuntimeError('Loading screen import failed')
texture.set_editor_property('lod_group', u.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('mip_gen_settings', u.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property('compression_settings', u.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('never_stream', True)
texture.set_editor_property('max_texture_size', 4096)
u.EditorAssetLibrary.save_loaded_asset(texture)
print('STARTUP_ART_IMPORT_COMPLETE')
