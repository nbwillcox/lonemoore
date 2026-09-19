# Old City Sewers — approved main integration

Integrated 16 September 2026 following explicit approval. The main project and normal Windows package now use the approved regional pack on both Old City Sewers floors. No Git repository was present; this updates the active project rather than merging a branch.

## Play

- `Play Old City Sewers Art Test.cmd` starts directly at the sewer entrance with campaign saving disabled for the entire test session. Close it when finished; test progress is intentionally not saved.
- `Play First Person Dungeon Crawler.cmd` starts the normal campaign, with the same sewer art and normal save behavior.
- Movement: W/S step, A/D turn, E inspect, M map.

## Integration

Added `Content/OldCitySewers`: 33 approved 2K texture maps, eight surface materials, four transparent decals and three decorative meshes. Main runtime instances use a new world-projected parent at a 200 cm repeat so existing scaled dungeon geometry does not stretch the textures. Authored props retain their UV materials. Texture streaming is enabled.

`DungeonSewerArt.cpp` applies region-index-1 surface choices and places drain grates, pipe outlets, hanging moss, wall marks and thin lower-wall brick dressing against existing boundaries. All added dressing has no collision. Existing water overlays receive the murky-water material. Water remains a static shallow-surface treatment, not a fluid simulation.

The environment hook changes sewer presentation only. Dungeon topology, campaign data, progression, existing collision, map/radar rules and 2D enemy artwork remain intact. The cook configuration includes the new content directory. Test flags in the controller provide isolated in-memory review runs; ordinary launches do not activate them.

Editable Blender sources, production scripts and exported source maps/props remain unchanged at `J:/Lonemoore_Art_OldCitySewers`. The manifest under `ArtReview/OldCitySewers` identifies that source root and current runtime material paths. Its copied visual specification and original handoff are historical approval snapshots; this document records the subsequent authorized integration.

## Validation

- Main editor and Windows development package built successfully using Unreal 5.8.2.
- All 12 project regression suites passed.
- The actual packaged game completed `-SewerArtReview` with zero failures: 377 capsule boundary sweeps on each sewer floor, nine regional material bindings, three decorative prop batches and 40 decal components per tested floor; added prop collision disabled; map movement lock and existing 2D combat passed.
- Packaged 1600 × 900 entrance, passage, map, cistern and combat captures were produced and visually inspected. These are actual Unreal game captures, not Blender previews.
- Preservation verification found all 113 original artwork files, four existing saves, campaign data and 18 unrelated source files unchanged. Review display preferences were restored before the interactive test.

Evidence: `Saved/SewerArtIntegration/Packaged`, `packaged_review.log`, `regression_tests.log`, `build_test_package.log`, `import_report.json`, and `preservation_results.json`.

This is a first-pass integration; the automated review and inspected captures do not substitute for the owner's interactive visual approval or broad performance profiling.

## Recovery and repeatability

`Saved/SewerArtIntegration/Backup` contains original changed-source/config copies and the prior Windows build in `WindowsBuild`. Save/config snapshots and `baseline.json` preserve the original state. Restore only the relevant integration files when rolling back so later unrelated changes are not overwritten.

`Tools/import_approved_sewer_art.py` builds the runtime material parent and instances. `Tools/verify_sewer_preservation.py` validates tracked originals; its `--restore-preferences` option restores the captured display preferences. `Tools/stage_approved_sewer_art.py` is the initial staging operation and deliberately refuses to overwrite an existing regional content root.
