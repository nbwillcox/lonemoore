"""Refresh only the four refined story marks in the isolated test project."""
import unreal as u,json
from pathlib import Path
R=Path(r'J:\Lonemoore_Regional_Art');assert Path(u.Paths.project_dir()).resolve()==(R/'UnrealTest').resolve()
result=[]
for name in ['Deep_ClawScars','Crypts_ErasedName','Infernal_BrokenSeal','Hell_BrokenSigil']:
 t=u.AssetImportTask();t.filename=str(R/'textures'/name/(name+'_BC.png'));t.destination_path='/Game/RegionalArt/Textures';t.destination_name='T_'+name+'_BC';t.automated=True;t.replace_existing=True;t.save=True
 u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t]);tex=u.load_asset(t.destination_path+'/'+t.destination_name);assert tex
 tex.set_editor_property('srgb',True);tex.set_editor_property('compression_settings',u.TextureCompressionSettings.TC_DEFAULT);tex.set_editor_property('address_x',u.TextureAddress.TA_CLAMP);tex.set_editor_property('address_y',u.TextureAddress.TA_CLAMP);u.EditorAssetLibrary.save_loaded_asset(tex);result.append(tex.get_path_name())
(R/'reports/refined_decals.json').write_text(json.dumps({'status':'PASS','assets':result},indent=2));print('STORY_DECALS_PASS')
