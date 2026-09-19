"""Read-only Blender geometry audit against authored navigation lanes.

Run: blender --background --python Tools/validate_room_kit_walkways.py
Scene meshes are only read; this script never saves a .blend or exports assets.
"""
import datetime
import hashlib
import json
from pathlib import Path

import bpy
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "Content/Game/Data/room_kit.json"
DESTINATION = ROOT / "Saved/RoomKitPass/walkways.json"
HEIGHTS = (0.8, 1.55, 2.35)
OFFSETS = (-0.30, 0.0, 0.30)


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def scene_tree():
    """Index every authored mesh, including integral decorative geometry."""
    vertices, triangles, identities = [], [], []
    depsgraph = bpy.context.evaluated_depsgraph_get()
    object_names = []
    for original in bpy.context.scene.objects:
        if original.type != "MESH":
            continue
        object_names.append(original.name)
        evaluated = original.evaluated_get(depsgraph)
        mesh = evaluated.to_mesh()
        try:
            mesh.calc_loop_triangles()
            base = len(vertices)
            vertices.extend(evaluated.matrix_world @ vertex.co for vertex in mesh.vertices)
            for triangle in mesh.loop_triangles:
                triangles.append(tuple(base + index for index in triangle.vertices))
                polygon = mesh.polygons[triangle.polygon_index]
                material = mesh.materials[polygon.material_index] if polygon.material_index < len(mesh.materials) else None
                identities.append({"object": original.name, "polygon": triangle.polygon_index,
                                   "material": material.name if material else ""})
        finally:
            evaluated.to_mesh_clear()
    return BVHTree.FromPolygons(vertices, triangles, all_triangles=True), identities, object_names


manifest_hash = sha256(MANIFEST)
manifest = json.loads(MANIFEST.read_text())
cell_metres = manifest["cellCm"] / 100.0
origin_x, origin_y = manifest["centerCell"]
report = {
    "generatedUtc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    "manifestSha256": manifest_hash,
    "method": "Read-only world-space triangle BVH ray casts along every adjacent walkable cell pair, in both directions; all scene mesh objects included.",
    "limitations": "Samples horizontal lane rays at three heights and three lateral offsets; this is not a swept capsule or verification of separately placed Unreal furnishing assets.",
    "heightsMetres": HEIGHTS,
    "lateralOffsetsMetres": OFFSETS,
    "excludedObjects": [],
    "rooms": [],
}
for template in manifest["templates"]:
    name = template["id"]
    path = ROOT / "ArtSource/RoomKit/Scenes" / ("SM_RK_" + name + ".blend")
    before = sha256(path)
    bpy.ops.wm.open_mainfile(filepath=str(path), load_ui=False)
    tree, identities, objects = scene_tree()
    rows = template["rows"]
    room = {"id": name, "scene": str(path.relative_to(ROOT)), "sceneSha256": before,
            "meshObjects": objects, "legalAdjacentPairs": 0, "rays": 0, "failures": []}
    for y, row in enumerate(rows):
        for x, tile in enumerate(row):
            if tile != ".":
                continue
            for dx, dy in ((1, 0), (0, 1)):
                nx, ny = x + dx, y + dy
                if ny >= len(rows) or nx >= len(row) or rows[ny][nx] != ".":
                    continue
                room["legalAdjacentPairs"] += 1
                a = Vector(((x - origin_x) * cell_metres, (y - origin_y) * cell_metres, 0))
                b = Vector(((nx - origin_x) * cell_metres, (ny - origin_y) * cell_metres, 0))
                direction = (b - a).normalized()
                perpendicular = Vector((-direction.y, direction.x, 0))
                for height in HEIGHTS:
                    for lateral in OFFSETS:
                        offset = perpendicular * lateral + Vector((0, 0, height))
                        for reverse in (False, True):
                            start, end = (b + offset, a + offset) if reverse else (a + offset, b + offset)
                            vector = end - start
                            hit, normal, index, distance = tree.ray_cast(start, vector.normalized(), vector.length)
                            room["rays"] += 1
                            if hit is not None:
                                room["failures"].append({
                                    "fromCell": [nx, ny] if reverse else [x, y],
                                    "toCell": [x, y] if reverse else [nx, ny],
                                    "heightMetres": height, "lateralOffsetMetres": lateral,
                                    "hitMetres": [round(value, 5) for value in hit],
                                    "distanceMetres": round(distance, 5), **identities[index],
                                })
    room["sourceUnchanged"] = before == sha256(path)
    room["passed"] = room["sourceUnchanged"] and not room["failures"]
    report["rooms"].append(room)
    print(f"ROOM_KIT_WALKWAYS {name} pairs={room['legalAdjacentPairs']} rays={room['rays']} failures={len(room['failures'])}", flush=True)

report["manifestUnchanged"] = manifest_hash == sha256(MANIFEST)
report["roomCount"] = len(report["rooms"])
report["rayCount"] = sum(room["rays"] for room in report["rooms"])
report["failureCount"] = sum(len(room["failures"]) for room in report["rooms"])
report["failedRooms"] = [room["id"] for room in report["rooms"] if not room["passed"]]
report["passed"] = report["manifestUnchanged"] and not report["failedRooms"] and report["roomCount"] == 25
DESTINATION.parent.mkdir(parents=True, exist_ok=True)
DESTINATION.write_text(json.dumps(report, indent=2) + "\n")
print(f"ROOM_KIT_WALKWAYS_COMPLETE rooms={report['roomCount']} rays={report['rayCount']} failures={report['failureCount']} passed={report['passed']}", flush=True)
