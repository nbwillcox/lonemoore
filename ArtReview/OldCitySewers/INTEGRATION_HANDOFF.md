# Old City Sewers — integration handoff

**STAGED FOR VISUAL APPROVAL. Do not merge, migrate, deploy, or replace live assets until the owner approves this sample.**

Pack: `J:\Lonemoore_Art_OldCitySewers`
Active game was treated as read-only. The test project is `UnrealTest/LonemooreArtTest.uproject`; it contains no gameplay code or campaign data. No branch switch or merge was performed. No interaction with the running game editor was required.

## Region authority

Campaign ID for this pack: `old_city_sewers`; current game `regionIndex: 1`. Covers The Drowned Conduit and The Rat King's Cistern, preserving the shared lore: “The old channels carry no rain. Every drop seeps upward from below.” Moisture is rising seepage, accumulated water and old pipe leakage, not a new rain-fed region.

See `VISUAL_SPEC.md` and `reports/reference_snapshot.json` for the exact read-only project sources. The actual sewer concept image was inspected. It was not pasted into any material. No new illustration was needed; image generation was available but not run.

## Assets and placement

| Asset | Intended use | Scale |
|---|---|---|
| ConduitBrick | Drainage wall bases and smaller passages | 2 m square tile; 0.4 x 0.25 m bond |
| ReservoirAshlar | Large cistern/conduit walls | 2 m square tile; about 0.67 x 0.33 m blocks |
| MaintenanceFlags | Raised, readable walking surface | 2 m square tile; about 0.67 x 0.5 m slabs |
| VaultBrick | Curved ceiling lining | 2 m square tile; about 0.286 x 0.167 m bond |
| LimestoneCoping | Real arch stones, coping and sill edges | 2 m square tile; no painted masonry joints |
| CorrodedIron | Pipe and grate | 2 m square tile; metallic mask only on remaining iron |
| MossMat | Localized hanging growth | 2 m square tile; no emission |
| MurkyWater | Shallow side channel/pools below walking plane | 2 m square tile; opaque review water |
| RisingDamp decal | Lower walls, placed bottom-up | 2 x 1 m |
| MineralBloom decal | Seepage band, irregular pale salts | 1.2 x 1.2 m |
| PipeJointLeak decal | Below corroded pipe seams | 0.6 x 1.2 m |
| MortarSpall decal | Small damaged mortar/stone surface patches | 0.75 x 0.5 m |
| SM_OCS_PipeOutlet | Decorative wall outlet | About 0.45 x 0.99 x 0.45 m; hollow barrel and bolt geometry |
| SM_OCS_DrainGrate | Channel cover | About 0.905 x 1.12 x 0.06 m; open bar geometry |
| SM_OCS_HangingMoss | Wet wall joints and vault margins | About 1.01 x 0.10 x 0.89 m; mount pivot at top |

The pipe's editable-source pivot is the barrel centre. Grate pivot is at floor level; moss pivot is at its upper mounting point. Exact bounds, triangle counts, map paths and slots are in `manifest.json`.

Keep decoration beside the walking footprint. Preserve central space for the game's existing 2D enemy billboards. Do not create 3D enemies, change encounter placement, change collision, or connect this corridor to procedural generation as part of this art pack.

## Source and texture conventions

`bake_textures.py` is the deterministic procedural source (seed 1947). It raster-bakes pigment, physical height-derived tangent normals, roughness and iron-only metallic masks at native 2048 square resolution. All fields repeat periodically; base color contains no rendered lights, cast shadows, perspective or AO. The raw physical height fields in `sources/*_height_m.npy` are in metres and are kept as editable authoring data, not imported displacement textures. This is a procedural field bake, not a high-poly sculpt bake.

`build_blender.py` produces `sources/OldCitySewers.blend`, the neutral variant, prop FBX/GLB files, and the corridor FBX. Blender node graphs explicitly use the exported maps. `.blend` files reference the adjacent texture folder through relative paths; retain the pack directory structure. `render_gallery.py` produces the individual neutral 3x3 inspections and `Material_Prop_Review.blend`.

All primary maps are 2K with a target density of 1024 texels/m. No 4K and no upscaling. Regular meshes use 1 UV unit per 2 metres. Vault UV distance follows the actual curved arc length. FBX exports metres with unit metadata; Unreal import is centimetres, uniform scale 1. GLBs are geometry-only interchange companions: assign regional materials from the manifest; their materials are not embedded.

| Map | Color space | Unreal setting |
|---|---|---|
| BC | sRGB RGB | Default compression, sRGB enabled |
| N | Linear DirectX tangent-space | Normalmap compression, green flip disabled |
| R | Linear scalar | Grayscale compression, sRGB disabled |
| M | Linear scalar, iron only | Grayscale compression, sRGB disabled |
| Decal BC | sRGB RGB + straight linear alpha | Default compression; clamp addressing |

Blender converts the DirectX normal green channel in its material graph; do not invert the exported normal a second time in Unreal. Alpha images have transparent borders, fractional coverage and no black matte. No normal is supplied for stains that do not require relief. Decals affect base color/roughness/opacity and inherit underlying geometry detail.

## Unreal reconstruction and controls

`unreal_import.py` uses the installed UE 5.8.2 Python APIs, guarded to refuse any project except this pack's `UnrealTest`. It imports source PNG/FBX, reconstructs materials, creates instances under `/Game/OldCitySewers`, checks prop dimensions, and creates `/Game/OldCitySewers/Showcase`. It does not attempt to transfer Blender shader nodes.

- `M_OCS_Surface`: reusable regional parent.
- `M_OCS_Decal`: deferred-decal parent and four regional decal instances.
- `M_OCS_ReviewDecalCard`: comparison utility only, not needed by the integration task.
- `M_ReviewEmber`: presentation-only light marker, not a regional gameplay prop.

Exposed controls: Tiling (1 = authored 2 m repeat), Tint (white = unchanged), Wetness (0–1, darkens pigment and approaches roughness 0.26), Roughness (multiplier, 1 = baked map), DetailStrength (0–1; default 0.65, water 0.4), Metallic (1 for iron, 0 for other surfaces), and decal Opacity (0–1). Keep upper walls predominantly dry and rough. Use moisture controls in localized instances, not globally over every stone.

Test textures use `never_stream=True` for consistent inspection. Before any approved production integration, the integration owner should enable streaming and budget residency on target hardware. No performance or packaged-game claims are made here.

The Blender corridor uses transparent surface cards to preview decal placement; these cards are excluded from the Unreal corridor FBX. Unreal places actual deferred DecalActors. The full showcase FBX is a static art-review assembly, not a collision/layout module kit. Reuse the individual props and materials after approval; do not substitute the combined showcase for the rebuilt generator's layout.

## Visual review

`Blender_*` images are Cycles renders. `Unreal_*` images are offscreen captures from the separate Unreal test project. Both use the current game's 76-degree horizontal FOV and 155 cm eye height at 1600 x 900, without HUD. Their lighting rigs are deliberately review-specific, not identical to current game lighting.

Read `reports/FINAL_QA.md` for the final inspection result. Automated evidence: `texture_validation.json`, `seams.json`, `unreal_validation.json`, and the actual run logs. No live-game, packaged-build, generator, collision, map, radar or progression validation was performed or implied.

Water is deliberately an opaque shallow-water approximation with a subtle normal map. No flow simulation, refraction, buoyancy, or water collision is supplied. Torch baskets/ember cores serve as presentation lighting fixtures, without flame animation or gameplay behavior.

## Reproduction

From this art workspace, run `scripts/Build-Pack.ps1 -Stage All`. You can run individual `Textures`, `Blender`, `Gallery`, or `Unreal` stages. The provided defaults identify the installed Blender 5.2, Python runtime with numpy/Pillow, and UE 5.8.2; override paths only for another art workstation. The `Unreal` stage always targets the isolated test project.

After visual approval, the dungeon-building task can prepare an integration change against its own current branch using the manifest. That future change must independently preserve collision, saves, gameplay and generator behavior. Approval is not assumed or granted by this document.
