"""Build UV-based room materials from the existing approved regional PBR artwork.

All output stays under /Game/RoomKit. Original materials and textures are read-only.
Run in the full Unreal editor before import_room_kit.py.
"""
import json
from pathlib import Path
import unreal as u

root = Path(u.Paths.project_dir()).resolve()
asset = u.EditorAssetLibrary
edit = u.MaterialEditingLibrary
tools = u.AssetToolsHelpers.get_asset_tools()
destination = '/Game/RoomKit/Materials/'
roles = ['Wall', 'Floor', 'Vault', 'Trim', 'Iron']
identity = {
    'Cathedral': ['LimeWall', 'ProcessionalInlay', 'PlasterSoffit', 'CarvedLimestone'],
    'Catacombs': ['BurialRubble', 'ChippedFlags', 'BurialSoffit', 'ChalkLimestone'],
    'Warrens': ['EarthShale', 'PackedEarth', 'RootSoil', 'ScavengedOak'],
    'Crypts': ['VigilMarble', 'MortuaryInlay', 'CofferedStone', 'SepulchralBorder'],
    'Fortress': ['DefenseBlocks', 'GarrisonPavers', 'BarracksPlanks', 'ForgedStraps'],
    'Deep': ['ChasmRock', 'FracturedBedrock', 'Overburden', 'MineralEdges'],
    'Infernal': ['SealMonolith', 'ShatteredPavement', 'ScorchedSoffit', 'RitualEdges'],
    'Hell': ['ColumnBasalt', 'HellforgePlates', 'HangingBasalt', 'BrokenHaloStone'],
}
iron = dict(zip(identity, ['OxidizedIron', 'AgedBronze', 'BatteredIron', 'TarnishedSilver',
                           'WardenIron', 'AncientIron', 'HeatIron', 'InfernalMetal']))
parent_path = destination + 'M_RoomKitUV'
parent = u.load_asset(parent_path) or asset.duplicate_asset(
    '/Game/RegionalIdentity/Materials/M_IdentityUV', parent_path)
assert parent
edit.set_material_usage(parent, u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
edit.set_material_usage(parent, u.MaterialUsage.MATUSAGE_NANITE)
assert not edit.recompile_material(parent)
assert asset.save_loaded_asset(parent)
report = []

def instance(code, role, source=None):
    name = f'MI_RK_{code}_{role}'
    path = destination + name
    mi = u.load_asset(path)
    if not mi:
        mi = (asset.duplicate_asset(source, path) if source else
              tools.create_asset(name, destination.rstrip('/'), u.MaterialInstanceConstant,
                                 u.MaterialInstanceConstantFactoryNew()))
    assert mi, path
    edit.set_material_instance_parent(mi, parent)
    edit.set_material_instance_scalar_parameter_value(mi, 'Tiling', 1.0)
    edit.set_material_instance_scalar_parameter_value(mi, 'EmissionStrength', 0.0)
    edit.set_material_instance_scalar_parameter_value(mi, 'DetailStrength', .72)
    edit.set_material_instance_scalar_parameter_value(mi, 'Metallic', 1.0 if role == 'Iron' else 0.0)
    edit.set_material_instance_scalar_parameter_value(mi, 'Wetness', 0.0)
    return mi

def finish(mi, code, role, source):
    textures = {}
    for key in ['base_color', 'normal', 'roughness']:
        tex = edit.get_material_instance_texture_parameter_value(mi, key)
        assert tex, (code, role, key)
        textures[key] = tex.get_path_name()
    edit.update_material_instance(mi)
    assert asset.save_loaded_asset(mi)
    report.append(dict(region=code, role=role, material=mi.get_path_name(),
                       source=source, textures=textures, uv_repeat_m=2))

for code, surfaces in identity.items():
    for role, suffix in zip(roles, surfaces + [iron[code]]):
        folder = 'RegionalArt' if role == 'Iron' else 'RegionalIdentity'
        source = f'/Game/{folder}/Materials/MI_UV_{code}_{suffix}'
        assert u.load_asset(source), source
        mi = instance(code, role, source)
        # Copy inherited values explicitly before changing the parent hierarchy.
        original = u.load_asset(source)
        for key in ['base_color', 'normal', 'roughness', 'metallic']:
            tex = edit.get_material_instance_texture_parameter_value(original, key)
            if tex:
                edit.set_material_instance_texture_parameter_value(mi, key, tex)
        finish(mi, code, role, source)

for role, surface in zip(roles, ['ReservoirAshlar', 'MaintenanceFlags', 'VaultBrick',
                                'LimestoneCoping', 'CorrodedIron']):
    mi = instance('Sewer', role)
    for key, suffix in [('base_color', 'BC'), ('normal', 'N'), ('roughness', 'R'), ('metallic', 'M')]:
        tex = u.load_asset(f'/Game/OldCitySewers/Textures/T_{surface}_{suffix}')
        if tex:
            edit.set_material_instance_texture_parameter_value(mi, key, tex)
        elif key != 'metallic':
            raise AssertionError((surface, key))
    edit.set_material_instance_scalar_parameter_value(mi, 'Wetness', .16 if role != 'Iron' else .04)
    finish(mi, 'Sewer', role, '/Game/OldCitySewers/Textures/T_' + surface)

out = root / 'Saved/RoomKitPass'
out.mkdir(parents=True, exist_ok=True)
(out / 'materials.json').write_text(json.dumps(dict(status='PASS', materials=report), indent=2))
print('ROOM_KIT_MATERIALS_COMPLETE count=' + str(len(report)), flush=True)
if '-KeepEditorForRoomKit' not in u.SystemLibrary.get_command_line():
    u.SystemLibrary.quit_editor()
