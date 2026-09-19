"""Import new campaign stair meshes without modifying original artwork or materials."""
import unreal as u, json
from pathlib import Path
r=Path(u.Paths.project_dir()).resolve();a=u.EditorAssetLibrary;e=u.MaterialEditingLibrary;t=u.AssetToolsHelpers.get_asset_tools()
regions=[
 ('Cathedral_PilgrimLimestone','Cathedral_BellCarving','Cathedral_OxidizedIron','Cathedral_AltarDressing'),
 ('ReservoirAshlar','LimestoneCoping','CorrodedIron','VaultBrick'),
 ('Catacombs_NicheStone','Catacombs_CrownMemorial','Catacombs_AgedBronze','Catacombs_BurialRubble'),
 ('Warrens_PatchedRubble','Warrens_TimberBracing','Warrens_BatteredIron','Warrens_RootMoss'),
 ('Crypts_SepulchralAshlar','Crypts_FuneraryCarving','Crypts_TarnishedSilver','Crypts_CoffinStone'),
 ('Fortress_GarrisonBlocks','Fortress_MilitaryTrim','Fortress_WardenIron','Fortress_BarredPlate'),
 ('Deep_AncientIron','Deep_MineralCrust','Deep_AncientIron','Deep_ScorchedDrakeRock'),
 ('Infernal_SealMasonry','Infernal_RitualTrim','Infernal_HeatIron','Infernal_SealCarvedSlabs'),
 ('Hell_BasaltRampart','Hell_ObsidianCarving','Hell_InfernalMetal','Hell_ForgeSlag')]
report=[]
manifest=json.loads((r/'ArtReview/CampaignExpansion/Stairs/manifest.json').read_text())
for item in manifest:
    name=item['mesh'];reg=item['region'];p=r/'ArtReview/CampaignExpansion/Stairs'/f'{name}.fbx'
    task=u.AssetImportTask();task.filename=str(p);task.destination_path='/Game/CampaignExpansion/Meshes';task.destination_name='SM_'+name;task.automated=True;task.replace_existing=True;task.save=True
    opt=u.FbxImportUI();opt.import_mesh=True;opt.import_materials=False;opt.import_textures=False;opt.import_as_skeletal=False;opt.static_mesh_import_data.combine_meshes=True;opt.static_mesh_import_data.auto_generate_collision=False;task.options=opt
    t.import_asset_tasks([task]);mesh=u.load_asset(task.destination_path+'/'+task.destination_name);assert mesh
    role_paths={}
    for role,mat in zip(['Stone','Trim','Metal','Inset'],regions[reg]):
        path=('/Game/OldCitySewers/Materials/MI_Game_' if reg==1 else '/Game/RegionalArt/Materials/MI_')+mat
        role_paths[role]=u.load_asset(path);assert role_paths[role],path
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        slot_name=str(slot.get_editor_property('imported_material_slot_name'));role=next(k for k in role_paths if k in slot_name);mesh.set_material(i,role_paths[role])
    a.save_loaded_asset(mesh);report.append({'floor':item['floor'],'mesh':mesh.get_path_name(),'materials':[m.get_path_name() for m in role_paths.values()]})

def material(name,base,emission=None):
    path='/Game/CampaignExpansion/Materials/'+name
    m=u.load_asset(path) if a.does_asset_exist(path) else t.create_asset(name,'/Game/CampaignExpansion/Materials',u.Material,u.MaterialFactoryNew())
    e.delete_all_material_expressions(m)
    color=e.create_material_expression(m,u.MaterialExpressionConstant3Vector,-350,0);color.set_editor_property('constant',u.LinearColor(*base,1));e.connect_material_property(color,'',u.MaterialProperty.MP_BASE_COLOR)
    rough=e.create_material_expression(m,u.MaterialExpressionConstant,-350,160);rough.set_editor_property('r',.87);e.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    if emission:
        glow=e.create_material_expression(m,u.MaterialExpressionConstant3Vector,-400,350);glow.set_editor_property('constant',u.LinearColor(*emission,1))
        noise=e.create_material_expression(m,u.MaterialExpressionNoise,-650,550);noise.set_editor_property('scale',.018);noise.set_editor_property('levels',3)
        mul=e.create_material_expression(m,u.MaterialExpressionMultiply,-100,350);e.connect_material_expressions(glow,'',mul,'A');e.connect_material_expressions(noise,'',mul,'B');e.connect_material_property(mul,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    e.set_material_usage(m,u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    assert not e.recompile_material(m);a.save_loaded_asset(m)
material('M_FirePit',(.08,.015,.003),(5.0,.26,.008))
material('M_Chasm',(.006,.008,.011))
(r/'Saved/CampaignExpansion/import_report.json').write_text(json.dumps(report,indent=2))
print('CAMPAIGN_IMPORT_COMPLETE',len(report),'stairs, 2 pit materials')
