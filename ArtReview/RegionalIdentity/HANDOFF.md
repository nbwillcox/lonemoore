# Regional identity — integrated main-game handoff

Status: integrated in the normal Windows game and validated in the packaged executable on 16 September 2026. User approval explicitly authorized this main-game integration. REVIEW.html separates actual game captures from the earlier isolated Unreal and Blender comparisons.

## Assets and region mapping

227 new runtime assets under /Game/RegionalIdentity: 107 native 2K texture maps, 50 architectural meshes, two parent materials and 68 regional instances. Editable production remains in this sibling art workspace: eight .blend scenes, procedural physical-height sources, exported FBX and PNG files, repeatable scripts, and the full placement/scale manifest. The original 698 main-project assets and 113 reference art files were not overwritten.

Floor numbers here are player-facing: 1 Cathedral; 2–3 approved Old City Sewers (retained); 4–5 Catacombs; 6–7 Warrens; 8–9 Crypts; 10–11 Fortress; 12–13 Deep; 14–15 Infernal; 16–18 Hell. Hell uses cooled slag, foundry plates and broken dark stone across its three floors. Stable region IDs and all established lore remain unchanged.

## Runtime behavior

DungeonIdentityArt.h/.cpp maps cosmetic module paths. DungeonEnvironment.cpp adds NoCollision regional wall, niche, floor, ceiling, arch and upper-cap instances at the existing module transforms. Original physical meshes and seals retain their original collision settings. Their old surfaces are hidden only after a replacement mesh loads. Missing replacements log an error and retain the original visible geometry. Moving secret doors keep their original actors and collision, with regional cosmetic children attached. Tall walls use the world-projected material variants at the intended physical scale. DungeonRegionalArt.cpp removes obsolete flat lower-wall repair strips so the new facing remains visible. Existing regional props, decals, crates, themed levers, floating keys and 2D enemies remain in use.

Only three existing rendering/review source files and the cook-directory list in DefaultGame.ini changed; two rendering helper files were added. Layout generation, progression, collision definitions, map/radar, combat, inventory, balance, saves and campaign data were not edited. Only Materials, Textures and Meshes were promoted; isolated test maps and the sewer benchmark were excluded.

## Texture and geometry conventions

All materials use native 2048×2048 textures at a 2 m repeat (1024 pixels/m). Base color is sRGB, with no painted lighting or perspective. Normal maps are DirectX, linear BC5 with no Unreal green flip; Blender flips green for its own preview. Roughness, metal and emission masks are linear. UV and world-projected parents expose tint, wetness, roughness, detail strength and physical tiling. Deep recesses, burial shelves, braces, cavern facets and ceiling forms use geometry. No 4K assets or upscaled source maps are used. Tile boundaries, UV scale, normal conventions and isolated lighting were checked before integration.

## Completed validation

All 12 existing regression suites passed; the Windows build and cook/package completed successfully. The actual packaged game reviewed all 18 floors, captured 108 views, and passed 6,882 boundary capsule sweeps with zero mismatches. Every non-sewer region loaded visible collision-free architectural batches; sewers retained the approved pack. Themed key identity, pickup and bounded floating motion, lever states and gates, map movement blocking and existing 2D enemy combat passed. The separate shared-prop review passed crate loot, key registry cleanup and lever behavior. The packaged logs contain no Error/Fatal or failed-check entries. Contact sheets and full-size game captures were visually inspected; reports/visual_review.json records the scope.

Save and preference preservation was hash-checked after restoring display preferences. All 25 baseline save files, campaign data, original assets/reference art and unrelated source/config files match the baseline. This is packaged development-build validation and visual inspection, not a claim of a full manual campaign playthrough.

## Playing and later changes

Use the normal Play First Person Dungeon Crawler.cmd launcher. Play Regional Art Test.cmd starts a selected region with campaign saving disabled. The main project is J:\First Person Dungeon Crawler Game. Current integration evidence is in Saved/RegionalIdentity; the accepted pre-integration executable and cooked containers are backed up in Saved/RegionalIdentity/Backup/AcceptedPackage, and source/config backups are under the same Backup directory. Restore the package as one consistent set if rollback is needed; do not mix executable and cooked container revisions.

Production scripts are mirrored in scripts/. Do not rerun setup_identity_pass.py over completed work. Import/material generation remains guarded to the isolated Unreal test project. integrate_identity_art.py is a one-time guarded promotion with hash checks and backups. finalize_identity_integration.py updates this handoff only after packaged, preservation and visual checks pass. Image generation was available but not used; the delivered work is procedural texture production and Blender geometry.
