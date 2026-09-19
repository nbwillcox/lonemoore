"""Import the isolated, low-cost regional prop pack into the full Unreal editor.

Approved RoomKit/RegionalArt/Sewer textures are read-only sources. All new meshes,
material instances and the accent shader live below /Game/RegionalDressing.
Runtime uses the imported default material slots and its existing HISM culling.
"""
from pathlib import Path
import json, hashlib, time
import unreal as u

ROOT = Path(u.Paths.project_dir()).resolve()
SOURCE = ROOT / 'ArtSource/RegionalDressing'
OUT = ROOT / 'Saved/RegionalDressingPass'
OUT.mkdir(parents=True, exist_ok=True)
MANIFEST = json.loads((SOURCE / 'manifest.json').read_text())
assert len(MANIFEST['props']) == 24
ROLES = ['Wall', 'Trim', 'Iron', 'Bone', 'Accent']
REGIONS = ['Cathedral', 'Sewer', 'Catacombs', 'Warrens', 'Crypts', 'Fortress', 'Deep', 'Infernal', 'Hell']
A = u.EditorAssetLibrary
E = u.MaterialEditingLibrary
T = u.AssetToolsHelpers.get_asset_tools()
S = u.get_editor_subsystem(u.StaticMeshEditorSubsystem)
DEST = '/Game/RegionalDressing/Materials/'
LECTERN_ONLY = '-DressingLecternOnly' in u.SystemLibrary.get_command_line()
if LECTERN_ONLY:
    report = json.loads((OUT / 'import_report.json').read_text())
    assert report.get('status') == 'PASS' and len(report.get('meshes', [])) == 24 and len(report.get('materials', [])) == 45, 'A complete prior import is required for a selective update'
    report.update(status='RUNNING', failures=0, selectiveUpdate='cathedral_lectern')
    report.pop('error', None)
else:
    report = dict(status='RUNNING', failures=0, materials=[], meshes=[])


def save_report():
    (OUT / 'import_report.json').write_text(json.dumps(report, indent=2))


def make(name, cls, factory):
    result = u.load_asset(DEST + name) or T.create_asset(name, DEST.rstrip('/'), cls, factory)
    assert result, DEST + name
    return result


def node(material, cls, **properties):
    expression = E.create_material_expression(material, cls, 0, 0)
    for key, value in properties.items():
        expression.set_editor_property(key, value)
    return expression


def link(first, output, second, pin):
    assert E.connect_material_expressions(first, output, second, pin), (first, second, pin)


def scalar(material, name, value):
    return node(material, u.MaterialExpressionScalarParameter, parameter_name=name, default_value=value)


def color(material, name, value):
    return node(material, u.MaterialExpressionVectorParameter, parameter_name=name, default_value=u.LinearColor(*value))


def product(material, first, second, first_output='', second_output=''):
    result = node(material, u.MaterialExpressionMultiply)
    link(first, first_output, result, 'A'); link(second, second_output, result, 'B')
    return result


def build_accent_parent():
    """Four texture fetches, opaque, no WPO, no lights and no gameplay ticking.

    Shared shader time only scrolls sewer/lava surface UVs and modulates a small
    emissive pulse. Other regions set flow/pulse/emission to zero.
    """
    material = make('M_DressingAccent', u.Material, u.MaterialFactoryNew())
    E.delete_all_material_expressions(material)
    material.set_editor_property('blend_mode', u.BlendMode.BLEND_OPAQUE)
    material.set_editor_property('two_sided', False)
    defaults = u.load_asset('/Game/RoomKit/Materials/MI_RK_Cathedral_Wall')
    assert defaults
    uv = node(material, u.MaterialExpressionTextureCoordinate)
    clock = node(material, u.MaterialExpressionTime)
    flow = color(material, 'Flow', (0, 0, 0, 0))
    mask = node(material, u.MaterialExpressionComponentMask, r=True, g=True, b=False, a=False)
    link(flow, '', mask, '')
    motion = product(material, clock, mask)
    coords = node(material, u.MaterialExpressionAdd)
    link(uv, '', coords, 'A'); link(motion, '', coords, 'B')
    samples = {}
    for role in ['base_color', 'normal', 'roughness', 'emission']:
        tex = E.get_material_instance_texture_parameter_value(defaults, 'roughness' if role == 'emission' else role)
        assert tex, role
        kind = (u.MaterialSamplerType.SAMPLERTYPE_COLOR if role == 'base_color' else
                u.MaterialSamplerType.SAMPLERTYPE_NORMAL if role == 'normal' else
                u.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE)
        sample = node(material, u.MaterialExpressionTextureSampleParameter2D,
                      parameter_name=role, texture=tex, sampler_type=kind)
        link(coords, '', sample, 'UVs'); samples[role] = sample
    base = product(material, samples['base_color'], color(material, 'Tint', (1, 1, 1, 1)), 'RGB')
    assert E.connect_material_property(base, '', u.MaterialProperty.MP_BASE_COLOR)
    assert E.connect_material_property(samples['normal'], 'RGB', u.MaterialProperty.MP_NORMAL)
    rough = product(material, samples['roughness'], scalar(material, 'RoughnessScale', 1), 'R')
    clamp = node(material, u.MaterialExpressionClamp, min_default=.12, max_default=1.)
    link(rough, '', clamp, '')
    assert E.connect_material_property(clamp, '', u.MaterialProperty.MP_ROUGHNESS)
    assert E.connect_material_property(scalar(material, 'Metallic', 0), '', u.MaterialProperty.MP_METALLIC)
    # A dim ember floor keeps modeled flame silhouettes readable between seams.
    glow_mask = node(material, u.MaterialExpressionLinearInterpolate, const_a=.22)
    link(samples['emission'], 'R', glow_mask, 'B'); link(scalar(material, 'MaskStrength', .78), '', glow_mask, 'Alpha')
    phase = product(material, clock, scalar(material, 'PulseSpeed', 1.6))
    wave = node(material, u.MaterialExpressionSine)
    link(phase, '', wave, '')
    amplitude = product(material, wave, scalar(material, 'PulseStrength', 0))
    pulse = node(material, u.MaterialExpressionAdd, const_b=1.)
    link(amplitude, '', pulse, 'A')
    emission = product(material, glow_mask, scalar(material, 'EmissionStrength', 0))
    emission = product(material, emission, color(material, 'EmissionTint', (1, .25, .035, 1)))
    emission = product(material, emission, pulse)
    assert E.connect_material_property(emission, '', u.MaterialProperty.MP_EMISSIVE_COLOR)
    E.set_material_usage(material, u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
    E.set_material_usage(material, u.MaterialUsage.MATUSAGE_NANITE)
    errors = E.recompile_material(material)
    assert not errors, errors
    assert A.save_loaded_asset(material)
    return material


def copy_textures(original, instance):
    for key in ['base_color', 'normal', 'roughness', 'metallic', 'emission']:
        texture = E.get_material_instance_texture_parameter_value(original, key)
        if texture:
            E.set_material_instance_texture_parameter_value(instance, key, texture)


def finish_instance(instance, region, role, source):
    textures = {}
    for key in ['base_color', 'normal', 'roughness']:
        texture = E.get_material_instance_texture_parameter_value(instance, key)
        assert texture, (region, role, key)
        textures[key] = texture.get_path_name()
    E.update_material_instance(instance)
    assert A.save_loaded_asset(instance)
    report['materials'].append(dict(region=region, role=role, material=instance.get_path_name(),
                                    source=source, textures=textures))


try:
    if LECTERN_ONLY:
        materials = {(region, role): u.load_asset(DEST + f'MI_RD_{region}_{role}') for region in REGIONS for role in ROLES}
        assert all(materials.values()), 'Selective import reuses the existing material instances'
        previous_ids = {item['id'] for item in report['meshes']}
        assert previous_ids == {item['id'] for item in MANIFEST['props']}
        for item in report['meshes']:
            if item['id'] != 'cathedral_lectern':
                original = next(p for p in MANIFEST['props'] if p['id'] == item['id'])
                assert item['sourceFbxSha256'] == hashlib.sha256((ROOT / original['fbx']).read_bytes()).hexdigest(), 'Unexpected non-lectern source change'
    else:
        parent = u.load_asset(DEST + 'M_DressingUV') or A.duplicate_asset(
            '/Game/RoomKit/Materials/M_RoomKitUV', DEST + 'M_DressingUV')
        assert parent
        E.set_material_usage(parent, u.MaterialUsage.MATUSAGE_INSTANCED_STATIC_MESHES)
        E.set_material_usage(parent, u.MaterialUsage.MATUSAGE_NANITE)
        assert not E.recompile_material(parent)
        assert A.save_loaded_asset(parent)
        accent_parent = build_accent_parent()
        materials = {}
        accent_sources = {
            'Cathedral': 'Cathedral_AltarDressing', 'Catacombs': 'Catacombs_ChalkBone',
            'Warrens': 'Warrens_RootMoss', 'Crypts': 'Crypts_CrimsonCloth',
            'Fortress': 'Fortress_BarredPlate', 'Deep': 'Deep_MineralCrust',
            'Infernal': 'Infernal_HeatFissures', 'Hell': 'Hell_EmberSeams'}
        for region in REGIONS:
            for role in ROLES:
                name = f'MI_RD_{region}_{role}'
                source = (f'/Game/RoomKit/Materials/MI_RK_{region}_{role}' if role in ['Wall', 'Trim', 'Iron']
                          else '/Game/RegionalArt/Materials/MI_UV_Catacombs_ChalkBone' if role == 'Bone'
                          else '/Game/OldCitySewers/Materials/MI_MossMat' if region == 'Sewer'
                          else '/Game/RegionalArt/Materials/MI_UV_' + accent_sources[region])
                original = u.load_asset(source)
                assert original, source
                instance = make(name, u.MaterialInstanceConstant, u.MaterialInstanceConstantFactoryNew())
                E.set_material_instance_parent(instance, accent_parent if role == 'Accent' else parent)
                copy_textures(original, instance)
                if region == 'Sewer' and role == 'Accent':
                    for key, suffix in [('base_color', 'BC'), ('normal', 'N'), ('roughness', 'R')]:
                        texture = u.load_asset('/Game/OldCitySewers/Textures/T_MossMat_' + suffix)
                        assert texture, ('Sewer', key)
                        E.set_material_instance_texture_parameter_value(instance, key, texture)
                E.set_material_instance_scalar_parameter_value(instance, 'Metallic', 1. if role == 'Iron' else 0.)
                E.set_material_instance_scalar_parameter_value(instance, 'EmissionStrength', 0.)
                E.set_material_instance_scalar_parameter_value(instance, 'Tiling', 1.)
                E.set_material_instance_scalar_parameter_value(instance, 'DetailStrength', .75)
                E.set_material_instance_scalar_parameter_value(instance, 'Wetness', .1 if region == 'Sewer' else 0.)
                E.set_material_instance_vector_parameter_value(instance, 'Tint', u.LinearColor(1, 1, 1, 1))
                if role == 'Bone' and region == 'Hell':
                    E.set_material_instance_vector_parameter_value(instance, 'Tint', u.LinearColor(.34, .24, .17, 1))
                if role == 'Accent':
                    emission = dict(Cathedral=3., Sewer=0., Catacombs=0., Warrens=0., Crypts=0., Fortress=0., Deep=.3, Infernal=4., Hell=6.)[region]
                    tint = (0.12, .5, .025, 1) if region == 'Sewer' else (1, 1, 1, 1)
                    glow = (.06, .45, .65, 1) if region == 'Deep' else (1, .08, .009, 1) if region == 'Infernal' else (1, .25, .025, 1)
                    flow = (.009, .004, 0, 0) if region == 'Sewer' else (.003, .007, 0, 0) if region in ['Infernal', 'Hell'] else (0, 0, 0, 0)
                    E.set_material_instance_scalar_parameter_value(instance, 'EmissionStrength', emission)
                    E.set_material_instance_scalar_parameter_value(instance, 'PulseStrength', .055 if region in ['Cathedral', 'Infernal', 'Hell'] else 0.)
                    E.set_material_instance_scalar_parameter_value(instance, 'RoughnessScale', .28 if region == 'Sewer' else .9)
                    E.set_material_instance_vector_parameter_value(instance, 'Tint', u.LinearColor(*tint))
                    E.set_material_instance_vector_parameter_value(instance, 'EmissionTint', u.LinearColor(*glow))
                    E.set_material_instance_vector_parameter_value(instance, 'Flow', u.LinearColor(*flow))
                finish_instance(instance, region, role, source)
                materials[(region, role)] = instance
    assert len(report['materials']) == 45
    save_report()

    runtime = json.loads((ROOT / 'Content/Game/Data/regional_dressing.json').read_text()) if LECTERN_ONLY else dict(version=1, materialRoles=ROLES, assets=[])
    if LECTERN_ONLY: assert len(runtime['assets']) == 24
    for prop in MANIFEST['props']:
        if LECTERN_ONLY and prop['id'] != 'cathedral_lectern': continue
        name = 'SM_RD_' + prop['id']
        path = '/Game/RegionalDressing/Meshes/' + name
        assert path.startswith('/Game/RegionalDressing/Meshes/SM_RD_')
        # Only assets from this new namespace may be replaced on an iterative run.
        if A.does_asset_exist(path):
            assert A.delete_asset(path), path
        task = u.AssetImportTask()
        task.filename = str(ROOT / prop['fbx']); task.destination_path = '/Game/RegionalDressing/Meshes'
        task.destination_name = name; task.automated = True; task.replace_existing = True; task.save = False
        options = u.FbxImportUI()
        options.import_mesh = True; options.import_materials = False; options.import_textures = False
        options.import_as_skeletal = False; options.mesh_type_to_import = u.FBXImportType.FBXIT_STATIC_MESH
        data = options.static_mesh_import_data
        data.combine_meshes = True; data.auto_generate_collision = False
        data.generate_lightmap_u_vs = False; data.import_uniform_scale = 1.
        data.normal_import_method = u.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
        task.options = options
        T.import_asset_tasks([task]); mesh = u.load_asset(path)
        assert mesh, path
        slots = list(mesh.static_materials)
        observed = [str(slot.material_slot_name).split('.')[0] for slot in slots]
        source_roles = [next((r for r in ROLES if label == r or label.startswith(r + '_')), None) for label in observed]
        assert all(source_roles), (name, observed)
        section_roles = [source_roles[S.get_lod_material_slot(mesh, 0, section)] for section in range(mesh.get_num_sections(0))]
        by_role = {role: slots[index] for index, role in enumerate(source_roles)}
        padded = []
        for role in ROLES:
            slot = by_role.get(role, u.StaticMaterial())
            slot.material_interface = materials[(prop['region'], role)]; slot.material_slot_name = role
            padded.append(slot)
        mesh.set_editor_property('static_materials', padded)
        for section, role in enumerate(section_roles):
            if S.get_lod_material_slot(mesh, 0, section) != ROLES.index(role):
                S.set_lod_material_slot(mesh, ROLES.index(role), 0, section)
        settings = S.get_nanite_settings(mesh); settings.enabled = True
        settings.fallback_percent_triangles = 1.; settings.fallback_relative_error = 0.
        S.set_nanite_settings(mesh, settings, True)
        mesh.set_editor_property('never_stream', False)
        assert A.save_loaded_asset(mesh)
        assert [str(slot.material_slot_name) for slot in mesh.static_materials] == ROLES
        assert [S.get_lod_material_slot(mesh, 0, section) for section in range(mesh.get_num_sections(0))] == [ROLES.index(role) for role in section_roles]
        box = mesh.get_bounding_box(); size = box.max - box.min
        bounds = [size.x, size.y, size.z]
        assert all(abs(x-y) < 2. for x, y in zip(bounds, prop['boundsCm'])), (name, bounds, prop['boundsCm'])
        placement = 'patch' if prop['placement'] == 'floor_patch' else 'wall' if prop['placement'] == 'wall' else 'floor'
        item = dict(id=prop['id'], name=prop['name'], regionIndex=prop['regionIndex'], mesh=mesh.get_path_name(),
                    placement=placement, boundsCm=bounds, minCm=[box.min.x, box.min.y, box.min.z],
                    maxCm=[box.max.x, box.max.y, box.max.z], maxScale=1.,
                    cornerScale=min(1., 100. / max(bounds[0], bounds[1])),
                    landmark=max(bounds[:2]) > 170., triangles=prop['triangles'],
                    materialSlots=ROLES, collision=False, addsPointLights=False)
        record = dict(**item, nanite=True, sections=section_roles,
                      sourceFbxSha256=hashlib.sha256((ROOT / prop['fbx']).read_bytes()).hexdigest())
        if LECTERN_ONLY:
            runtime['assets'][next(i for i, value in enumerate(runtime['assets']) if value['id'] == prop['id'])] = item
            report['meshes'][next(i for i, value in enumerate(report['meshes']) if value['id'] == prop['id'])] = record
        else:
            runtime['assets'].append(item)
            report['meshes'].append(record)
        save_report()
        print('REGIONAL_DRESSING_IMPORTED', prop['id'], flush=True)
    assert len(runtime['assets']) == len(report['meshes']) == 24
    (ROOT / 'Content/Game/Data/regional_dressing.json').write_text(json.dumps(runtime, indent=2))
    report['status'] = 'PASS'; save_report()
    print('REGIONAL_DRESSING_IMPORT_COMPLETE meshes=24 materials=45 failures=0' + (' selective=lectern changed=1' if LECTERN_ONLY else ''), flush=True)
except Exception as error:
    report['status'] = 'FAILED'; report['failures'] += 1; report['error'] = str(error); save_report()
    raise
finally:
    if '-KeepEditorForDressing' not in u.SystemLibrary.get_command_line():
        # Let asynchronous mesh/shader/derived-data work drain on ordinary editor
        # ticks. Sleeping here blocks the very main-thread work being awaited.
        _exit_started = time.monotonic()
        _exit_handle = None
        def finish_after_editor_ticks(delta_seconds):
            global _exit_handle
            if time.monotonic() - _exit_started < 12.:
                return
            u.unregister_slate_post_tick_callback(_exit_handle)
            print('REGIONAL_DRESSING_EDITOR_SETTLED_EXIT', report['status'], flush=True)
            u.SystemLibrary.quit_editor()
        _exit_handle = u.register_slate_post_tick_callback(finish_after_editor_ticks)
