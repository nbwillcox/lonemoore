"""Enable instanced geometry on the two new campaign pit materials."""
import unreal as u

for name in ['M_FirePit', 'M_Chasm']:
    material = u.load_asset('/Game/CampaignExpansion/Materials/' + name)
    assert material, name
    u.MaterialEditingLibrary.set_material_usage(material, u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    assert not u.MaterialEditingLibrary.recompile_material(material), name
    assert u.EditorAssetLibrary.save_loaded_asset(material), name
    print('CAMPAIGN_PIT_MATERIAL_FIXED', name)
