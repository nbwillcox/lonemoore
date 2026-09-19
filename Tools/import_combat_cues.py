import unreal as u
from pathlib import Path
root=Path(u.Paths.project_dir());tasks=[]
for name in ['critical','divine','warlock','spell','grenade']:
 t=u.AssetImportTask();t.filename=str(root/'ArtReview/Audio'/(name+'.wav'));t.destination_path='/Game/Game/Audio';t.destination_name='A_'+name;t.automated=True;t.replace_existing=True;t.save=True;tasks.append(t)
u.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
print('COMBAT_CUES_IMPORTED')
