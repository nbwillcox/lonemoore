# Lonemoore regional art handoff

Approved and integrated on 2026-09-16. Open REVIEW.html for labeled Blender, isolated Unreal and actual packaged-game comparisons. Run the game's `Play Regional Art Test.cmd` to select any of the nine existing regions with campaign saving disabled. The normal launcher uses the updated packaged build.

## Delivered

- Eight new surface packs: Cathedral, Catacombs, Warrens, Crypts, Fortress, Deep, Infernal and Hell. Approved Old City Sewers remains unchanged.
- 64 distinct native 2048×2048 material sets, 32 RGBA decals, 24 decorative meshes, eight regional lever variants, six world-key meshes and eight showcase corridor meshes. 268 texture-map PNGs. No 4K assets, upscaling or image generation.
- Eight editable .blend sources in `sources`, 46 FBX exports in `props`, maps in `textures`, and repeatable production/import scripts in `scripts`.
- Three reusable Unreal parents, 128 world/UV surface instances and 32 decal instances. Main content excludes showcase meshes/maps.
- The approved shared crate remains active. Rusted hardware serves the cathedral/sewers; lower regions have burial, military, mineral, ritual and horned forge details. World-key style follows the existing key identity. Astra and the narrative Hell Key are untouched.

## Scale and maps

Blender metres, Unreal centimetres. World surfaces repeat over 200 cm (1024 texels/metre), with projection preventing stretching on existing architecture. UV props use the same nominal repeat; curved bevels use local projection and real geometry for their silhouette. The existing game grid is 400 cm, eye height 155 cm and horizontal FOV 76°. Mesh dimensions and triangle counts are listed individually in manifest.json. Keys use 1.4× world presentation scale and a 20 cm, 2.4 second vertical hover.

Base colour is sRGB pigment without lighting or cast shadows. DirectX normals, roughness, metallic and emission are linear. Unreal normal compression is TC_NORMALMAP with green flip disabled; Blender inverts green before its Normal Map node. Metallic and emission maps exist only where used. Decals use straight RGBA with fully transparent borders. World and UV material parents expose tint, wetness, roughness, normal detail and emission; world tiling uses TextureSizeCm, UV tiling uses Tiling. Wetness defaults to zero; roughness is clamped at 0.30 to avoid mirrorlike surfaces.

## Reproduction

Workspace root is currently J:/Lonemoore_Regional_Art; scripts declare it explicitly. Change that root when relocating the pack. Existing Blender texture paths point into this workspace.

1. Run setup_regional_art.py only to initialize a new isolated workspace and save baseline evidence. It reads the authoritative campaign and reference files.
2. Run bake_regional_textures.py, then polish_regional_surfaces.py, then polish_regional_decals.py using Python with NumPy/Pillow. The polish steps are required to reproduce final surfaces and story marks.
3. Run Blender 5.2 in background with build_regional_models.py. Optional region-code arguments after `--` select a subset. Sources and exports remain editable and deterministic.
4. Run import_regional_art.py in UnrealTest/RegionalArtTest.uproject using UE 5.8.2 PythonScript commandlet: default stage imports textures/materials; `-ArtStage=Meshes` imports meshes and checks physical bounds/material slots. The importer rejects a different project. Then run preview_regional_unreal.py in the isolated editor for lighting captures.
5. Run review_regional_exports.py after any map polish, inspect the 3×3 sheets and neutral/torchlit views, then rebuild the main game only when integration is authorized. The initial activation script refuses an existing main content root.

## Integration

New art lives under /Game/RegionalArt. DungeonRegionalArt.cpp maps existing architecture roles to the region's material set and places non-colliding dressing using existing wall boundaries. It skips sewer decoration so the approved sewer pack stays authoritative. Later floors rotate prop/decal choices and use selected alternate wall panels. DungeonGame.cpp resolves regional lever meshes and existing key identities; animation and interaction logic stay separate. Config/DefaultGame.ini includes the new content in packaged cooking.

Generation, progression, collision, campaign lore, map/radar and 2D enemies were not redesigned. Original materials and assets were preserved. Source/config and the previous packaged binaries/content are backed up under Saved/RegionalArt/Backup. To roll back presentation, restore the backed-up original source/config, remove the three newly introduced DungeonRegionalArt source files from compilation, and rebuild; the original assets remain present. Never replace the user's save files with review state.

## Validation evidence

- Export validation: native 2K dimensions, normalized forward-facing normals, transparent decal borders and coverage; PASS.
- All 46 mesh imports verified against physical bounds and material-slot bindings; all texture/instance parameter bindings validated in Unreal 5.8.2.
- 16 isolated Unreal captures plus Blender neutral model sheets, material 3×3 sheets and torchlit corridors. Visible export scale, curved shading, corrosion regularity and decal-shape issues were corrected during inspection.
- Seam checks cover 192 base-colour/normal/roughness maps and 12 optional metallic/emission maps. Decal sheets show actual straight-alpha transparency rather than a baked checkerboard.
- Packaged all-floor review: Floors: 18; total boundary sweeps: 6882; failures: 0
- Runtime review checks boundary capsule sweeps, no-collision dressing, key loading/hover/pickup/removal, lever state/gates, map input blocking and existing 2D combat. Screenshots include entrance, passage, key, lever, map and combat for every floor.
- The save-disabled review fixture gives its hero first initiative for static combat captures, preventing accumulated enemy damage from interrupting the 18-floor visual inspection. This affects only the automated RegionalArtReview flag. Normal character stats and combat balance are unchanged. The corrected fixture was rebuilt into editor and packaged executables; existing cooked content remains valid because no assets or reflected types changed.
- The separate packaged shared-interactable review also passed, including crate loot collection, anchored key motion, lever gates and clearing/rebuilding the hover registry.
- Original-file hash verification: {"original_uassets": 229, "art": 113, "saves": 25, "campaign": 1, "unrelated_source": 21}; zero mismatches. Review display preferences restored.

Reports and captures prove the checks described; they do not replace an owner visual review or a full manual campaign playthrough. The first-pass art remains stylized and uses the game's existing architectural meshes. The isolated showcase corridors are not new dungeon layouts.
