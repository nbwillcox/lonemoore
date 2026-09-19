# Prototype atmosphere revision — validation, September 17, 2026

## Result

The separate Windows prototype builds, packages and completes its rendered review with zero failures (exit code 0). The revision removes the miniature arches, lowers ceilings, adds full-height structural supports and three furnishing meshes, and adds warm player light plus ambient fill. Existing artwork, room plans and progression rules are retained. [Visual comparison](AtmosphereRevision/REVIEW.html).

| Check | Evidence |
|---|---|
| Full native regression suite | 15 succeeded, 0 warnings, 0 failed, 0 skipped. `Saved/ExpansionAtmosphere/Automation/index.json` |
| Random generation | 100 distinct seeds; each reproduced exactly. All objectives and traversable areas reachable in the legal dependency order. `Saved/ExpansionAtmosphere/seed_results.txt` |
| Deliberate bypass injection | 200 checks removing key or boss seals were rejected by validation. Secret and optional routes participate in the reachability search. |
| Floor size | Reviewed seed: 2,112 walkable cells / 158 original = 13.4 times. Tested range: 1,913–2,508; every seed exceeds ten times the original area. |
| Enemy count | 20 regular encounters / 2 original. The same original rosters repeat ten times; exact tenfold regular enemy count verified for an equivalent party/run. The existing Rat King is retained once. |
| Complete clear | 1,025 actual movement steps, 21 actual battles, 6 healing potions in this run, key collection, optional lever/secret vaults, boss victory, both seals and real stair descent. Level-three warrior/mage/cleric fixture uses ordinary stats, combat and supplies. `Saved/ExpansionAtmosphere/playable_route.txt` |
| Persistence | Version-three layout, discovery, markers, opened seals and cleared encounters survive real save/load. Existing save migration tests pass. |
| Packaged physical collision | 4,826 capsule sweeps; zero disagreement with movement boundaries, including bridge drops. |
| Packaged progression | Absent key rejected; Rat King roster loaded; placing the fixture on the stairs with a living boss still fails to descend; the seal opens only after victory. |
| Revised art | Three new meshes imported: wall stores, pump station and pipe bank. In the reviewed seed: 234 furnishing clusters, 85 full-height supports, no standalone decorative arches, 2,207 instances of existing sewer decoration and 374 decals. Existing doorway frames remain. |
| Ceiling heights | 5.6m passages, 6–9.5m ordinary chambers, 8.5m guardian room, 14m reservoir. Applied when rendering existing or newly generated prototype saves. |
| Player light | Warm point light with gentle flicker and softer ambient fill, attached to the player camera. World enemy sprites dim with distance; original enemy textures and combat art are unchanged. Torch-on/off captures are included. |
| Continued user save | The actual existing `ExpansionPrototype_Auto.sav` loads successfully in the revised packaged game with save writes disabled. Its floor plan remains identical to its stored layout; screenshot `12_existing_save_revised.png` shows the restored journey. |
| Preservation | 124 original-art, campaign-save, prototype-save and display-setting files checked by SHA-256; zero changes. All four campaign saves and the prototype autosave are preserved. Report: `Saved/ExpansionAtmosphere/preservation.json`. The generator, navigation, combat, exploration and save-model source files are unchanged from the pre-revision backup. |

The packaged review log is `Saved/ExpansionAtmosphere/packaged_review.log` and ends with `EXPANSION_REVIEW_COMPLETE failures=0`. Results and screenshots are under `Builds/ExpansionPrototype/Windows/DungeonCrawler/Saved/ExpansionPrototype`. A copy of the results is `Saved/ExpansionAtmosphere/packaged_runtime_results.txt`. Original and revised views are compared in `ArtReview/ExpansionPrototype/AtmosphereRevision/REVIEW.html`.

## Practical limits

- Automated checks establish connectivity, collision agreement, progression, persistence and a complete scripted clear. They do not establish the owner's preference for scale, darkness, enemy spacing or exploration pace.
- Physical rendered sweeps cover the reviewed seed. The hundred-seed checks cover logical topology and dependencies.
- Screenshots use a save-disabled camera fixture. Their party and revealed-map state are staged for visual review; the full-clear test and playable launcher use the disclosed ordinary level-three party.
- Movement retains the current grid. Wall runs and room silhouettes vary; other-region caverns, fire pits and more art are subsequent work after this review.
- The normal packaged campaign remains in `Builds/Windows`. This prototype lives separately in `Builds/ExpansionPrototype/Windows` and uses `ExpansionPrototype_` save slots.

## Reproduce

`Tools/BuildExpansionPrototype.ps1 -Package` builds the editor, runs regression tests and packages into the separate prototype folder. `Play Expanded Dungeon Prototype.cmd` launches the playable build; choose Continue to retain the existing prototype journey.

Original asset sources are `ArtReview/ExpansionPrototype/Expansion_Sewer_Kit.blend` and its five FBX exports. The three new furnishing meshes are authored in `ArtReview/ExpansionPrototype/AtmosphereRevision/Sewer_Furnishings.blend`, with matching FBX files. Construction/import scripts: `Tools/build_expansion_atmosphere.py` and `Tools/import_expansion_atmosphere.py`. `Tools/make_expansion_atmosphere_review.py` publishes the packaged before/after gallery.
