"""Blender: repair timber/stone intersections, preserving source art and UVs.

The accepted source is read-only. Export only the two repaired wall modules.
"""
import bpy, json
from pathlib import Path

root = Path(r'J:\First Person Dungeon Crawler Game')
out = root / 'Saved/FineTunePass/Geometry'
out.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=r'J:\Lonemoore_Regional_Identity\sources\Warrens_Architecture.blend')
report = []
for part in ['Wall', 'Niche']:
    name = 'SM_ID_Warrens_' + part
    obj = bpy.data.objects[name]
    obj.hide_set(False)
    obj.hide_viewport = False
    timber = {i for i, m in enumerate(obj.data.materials) if 'ScavengedOak' in m.name}
    assert timber, [m.name for m in obj.data.materials]
    vertices = {v for p in obj.data.polygons if p.material_index in timber for v in p.vertices}
    # Rock relief reaches 38cm; the old horizontal timber face was only 37cm.
    # Deepen the timber symmetrically so even its beveled edge clears the rock.
    for i in vertices:
        obj.data.vertices[i].co.y *= 1.4
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=str(out / (name + '.fbx')), use_selection=True,
        axis_forward='-Y', axis_up='Z', apply_unit_scale=True, add_leaf_bones=False,
        object_types={'MESH'}, mesh_smooth_type='FACE')
    report.append(dict(mesh=name, timber_vertices=len(vertices), timber_depth_scale=1.4))
(out / 'repairs.json').write_text(json.dumps(report, indent=2))
bpy.ops.wm.save_as_mainfile(filepath=str(out / 'Warrens_Repaired.blend'))
print('WARRENS_GEOMETRY_REPAIRED', report)

