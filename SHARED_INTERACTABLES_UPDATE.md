# Shared interactable art — 16 September 2026

Historical record of the initial shared-interactable pass: the lever and crate replaced their placeholders, and the original world-key illustrations gained a smooth hover. The subsequent regional expansion is now approved and integrated. It retains this crate and hover behavior while adding regional lever and 3D world-key models. Inventory key artwork remains original. See `ArtReview/RegionalArt/HANDOFF.md` for the current build.

## Models and sources

`ArtReview/SharedProps/Shared_Interactables.blend` is the editable Blender source. `Tools/build_shared_props.py` rebuilds the geometry, deterministic 2K procedural material maps, FBX exports and neutral studio preview. `Tools/import_shared_props.py` imports to the new `/Game/SharedInteractables` content root with separate wood/iron slots, sRGB base colour, linear roughness/metallic and DirectX normals. Original mesh/material assets remain intact.

- Crate: 114 × 82.5 × 75.15 cm, 11,212 triangles. Separate planks, raised frame rails, lid seams, straps, bolts and latch. Replaces the shared `C` / `$` loot-container presentation; existing loot and removal rules remain unchanged. It is not a new breakable object or a replacement for unrelated background camp arrangements.
- Lever: 55 × 55 × 107.5 cm, 5,056 triangles. Base bolts, open bearing frame, collars, axle, handle and wooden grip. Existing switches and gates retain their behavior. The handle is a static mesh; this request did not introduce lever-pull animation.
- Keys: vertical sinusoidal float with a 2.4-second period, 20 cm total travel and a raised resting position. Each key stays anchored in XY and faces the camera. Only world keys animate; inventory icons, enemies, key types and pickup logic are untouched. Hover references clear on rebuild and collection. Motion pauses outside the dungeon screen.

All new models use the existing non-colliding presentation path. Model geometry determines silhouettes and seams; textures contain no lighting or cast shadows. Base-colour encoding was corrected after comparing Blender and Unreal views.

## Verification and recovery

The targeted `-SharedPropReview` runs the actual controller and checks model loading, separate material slots, absence of added collision, measured key travel, pickup/inventory, removal after collection, lever-operated gates, crate loot and stable hover registration across floor transitions/rebuilds. It runs with campaign writes disabled.

Evidence is under `Saved/SharedPropUpdate`: import report, regression log, build/package logs, runtime result text, in-game screenshots and preservation report. The `Packaged` subfolder holds the final packaged-game review. The Blender studio image is explicitly a Blender preview; game captures are labeled separately.

Original files and hashes are recorded under `Saved/SharedPropUpdate/Backup` and `baseline.json`. `Tools/verify_shared_prop_preservation.py` checks original assets, artwork, saves and campaign data; `--restore-preferences` restores saved display preferences after automated review. The earlier sewer integration backup still contains the pre-sewer Windows build; it is not a snapshot of this newer shared-prop build.

Use the normal `Play First Person Dungeon Crawler.cmd` launcher after packaging. The existing `Play Old City Sewers Art Test.cmd` also shows the new shared interactables during its save-disabled sewer session.

Final result: Windows packaging succeeded; all 12 regression suites and 19 targeted packaged-game checks passed with zero failures. Final packaged lever/crate captures were visually inspected. Preservation verification matched 217 original assets, 113 original artwork files, all 25 tracked existing save files (including prior test saves), and campaign data; review display preferences were restored.
