"""Import the supplied main-menu parchment without modifying the source artwork."""
import unreal as u
task = u.AssetImportTask()
task.filename = u.Paths.convert_relative_path_to_full(u.Paths.project_dir() + '.art/.ui-ux/main-menu-parchment.png')
task.destination_path = '/Game/Game/ImportedArt'
task.destination_name = 'T_main_menu_parchment'
task.automated = True
task.replace_existing = True
task.save = True
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture = u.load_asset('/Game/Game/ImportedArt/T_main_menu_parchment')
texture.set_editor_property('lod_group', u.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('mip_gen_settings', u.TextureMipGenSettings.TMGS_NO_MIPMAPS)
texture.set_editor_property('compression_settings', u.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('never_stream', True)
u.EditorAssetLibrary.save_loaded_asset(texture)
print('MENU_PARCHMENT_IMPORT_COMPLETE')
