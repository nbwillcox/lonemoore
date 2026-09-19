Adventure polish, **18 September 2026**: installed in the normal campaign, expanded prototype and authored-room playtest. Start **Play Lonemoore.cmd → New Game**. All seven existing classes are available, naming is required, and each character has separate Auto/Quick slots and multiple manual saves in its own folder. Companion substitution lets every starter form a full party.

New floors use **68 sections instead of 104 (34.6% fewer)** with **45–46 regular encounter sites**. A protected post-boss save shrine sits beside the descent stairs. Chests draw from all **33 catalog items**, favoring better quality at depth. Enemies are grounded using their original sprite transparency; shrine art matches its map tile; lighting is dimmer. **27 original effects** include a retro fireball, metallic attacks and separate cues for all 21 active skills. The current artwork, room kit and regional props are retained.

All **35 automated suites** passed, including 432 seeded encounter-distribution cases and full campaign routes for all seven starters. The packaged journey/save walkthrough passed **31 checks**; the packaged geometry review passed **18 floors and 72,758 physical boundary sweeps**, with no failures. Seven warmed stationary RTX 3060 captures at 3440 × 1369 output measured **72.3–75.9 FPS**, with **14.289–15.130 ms** 99th-percentile frames. Changed layouts prevent an identical-scene comparison; sustained play and subjective audio/atmosphere remain for player evaluation.

The installation verifies **49 runtime files in each of three targets**, leaving **66 existing save/settings files unchanged**. New named journeys are the supported player path; old flat saves are not listed. See the [current review](ArtReview/AdventurePolish/REVIEW.html), [handoff](ArtReview/AdventurePolish/HANDOFF.md), [validation](ArtReview/AdventurePolish/VALIDATION.md), [sound library](ArtReview/AdventurePolish/Audio/REVIEW.html) and [installation evidence](Saved/AdventurePolish/promotion.json). Earlier updates below are historical.

Room-kit polish, **18 September 2026**: installed in the normal campaign, expanded prototype and separate authored-room playtest. Rebuilt curved floor surfaces fix the disappearing foreground with **Nanite still enabled**. New floors distribute **70–72 regular encounter sites** across about 70% of ordinary exploration rooms, using the existing enemies and artwork. Early fights and companions are within two room exits. Existing room-kit saves gain encounters only in undiscovered, unprogressed rooms; cleared enemies stay cleared, and completed and corpse-recovery floors are protected. **24 new Blender props across nine themes** add regional detail while keeping the original surface art.

All **25 automated suites** passed, including 432 seeded encounter-distribution cases and full starting-class routes using normal recruitment, supplies and paid town recovery. The final packaged review passed **18 floors and 115,089 physical boundary sweeps**. The dressing review covered 18 floors and 48 detail captures with zero failures. All 24 prop types were visually inspected; two flat props remain angle-limited. The three repaired room types have 12 directional captures: ten expose continuous floor, and two face nearby walls. Floor coverage and 12,672 sampled walking-lane rays also passed.

Seven warmed stationary RTX 3060 captures at 3440 × 1369 output and Epic quality averaged **73.6–75.6 FPS**, with 99th-percentile frame times of **14.268–14.942 ms**. The five matching earlier scenes changed by at most 1.2%; additional Deep and Hell entrance captures measured 75.1 and 75.5 FPS. These are uncapped offscreen measurements using the existing automatic render-resolution setting. Sustained gameplay balance, moving-camera flicker and monitor tearing still require playtesting.

The promotion report verifies **49 runtime files in each of three installed targets** and **145/145 protected art, save and settings files unchanged**. Every `Saved` directory was excluded from installation. Four regenerated automation fixtures are documented separately; the original 149-row preservation baseline remains intact. Launch **Play Lonemoore.cmd** for the campaign or **Play Authored Room Dungeon.cmd** for the separate playtest. See the [current review](ArtReview/RoomKitPolish/REVIEW.html), [handoff](ArtReview/RoomKitPolish/HANDOFF.md), [performance comparison](Saved/RoomKitPolish/performance_comparison.json) and [installation evidence](Saved/RoomKitPolish/promotion.json). Earlier updates below are historical.

Authored room kit, **18 September 2026**: installed in both game launchers. The existing procedural generator now connects **25 editable Blender room designs** into 104 sections per new floor, with circular rooms, rounded turns, ruins, chapels, bridges, two Gothic approach halls, a guardian arena and a protected descent chamber. Matching portals and metric texture mapping replace overlapping dynamically built wall surfaces. Regional artwork, enemies, torch lighting, fog of war and themed stairs are retained. Explored, occupied, completed and corpse-recovery floors keep their saved layouts; new journeys and untouched floors use the new kit.

All **23 automated suites** passed, and the final packaged review passed **18 floors and 115,089 physical boundary sweeps**. Source meshes passed 12,672 sampled walking-lane rays across all 25 rooms. Five warmed stationary captures on the RTX 3060 at 3440 × 1369 output and Epic quality averaged **73.7–76.0 FPS**, with 99th-percentile frame times of 14.446–14.867 ms. These are uncapped offscreen measurements with the existing automatic resolution setting, not a full gameplay benchmark. All **124 protected art/save/settings files are unchanged**; 49 runtime files per installed target match the validated candidate. Static review cannot establish the absence of monitor tearing or temporal flicker.

Launch **Play Authored Room Dungeon.cmd** for a separate prepared crypt playtest, or **Play Lonemoore.cmd** for the updated campaign. Review: [25-room gallery](ArtReview/RoomKit/REVIEW.html), [validation](ArtReview/RoomKit/VALIDATION.md), [editable source](ArtSource/RoomKit/README.md). Installation evidence: `Saved/RoomKitPass/promotion.json`. Earlier updates below are historical.

Fine-tuning pass, **17 September 2026**: installed in both game launchers. Repaired Warrens timber/stone intersections, separated wall supports and floor modules, enlarged world enemies (with ceiling clearance for oversized bosses), and added timed save confirmation. VSync is enabled and remains adjustable. All 20 native tests passed; all 18 floors passed 92,808 physical boundary sweeps. Five player saves and 117 original artwork files are unchanged; only VSync changed in existing settings. Average uncapped FPS was within 4% of the previous build in two matching scenes; pre-existing Warrens frame-time spikes remain. Screenshots and details: [Fine-tuning review](ArtReview/FineTunePass/REVIEW.html).

Optimization pass, **17 September 2026**: installed in the normal Windows game and expanded prototype. On this RTX 3060 at 3440 × 1369, five identical saved scenes improved from 57–65 FPS to 74–78 FPS (21–29%); 99th-percentile frame rates improved from 49–55 to 68–71 FPS. Includes persistent floor architecture, bounded radar rendering, static instance batches, 129 Nanite scenery meshes, occlusion/distance culling, reduced redundant updates, asynchronous regional loading and a lighter Epic TSR history buffer. Original stair detail is retained. All 19 native suites, the final packaged 18-floor review (92,808 physical sweeps), and the interaction/save review passed. All 124 protected art/save/settings files remain unchanged, and both delivered runtime packages match the validated candidate. Launch `Play Lonemoore.cmd`; comparison and details: `ArtReview/PerformancePass/REVIEW.html` and `ArtReview/PerformancePass/VALIDATION.md`.

Approved campaign expansion and gothic stairs, **17 September 2026**: implemented in the normal Windows build and the separate prototype package. All eighteen floors now support expanded randomized layouts, at least 10 times the former generated walkable area, exactly 10 times the regular encounter rosters, regional architecture, bridges/pits, fog of war and the approved player lighting. Eighteen regional gothic stair meshes replace the old steps; the bottom tread is no longer coplanar with the floor. Existing explored layouts and progress are preserved; untouched floors expand on entry. All 17 native suites, three full class routes and the packaged 18-floor review (92,808 physical sweeps) passed. All 124 protected artwork/save/settings files are unchanged. Launch `Play Lonemoore.cmd`; gallery and evidence: `ArtReview/CampaignExpansion/REVIEW.html` and `ArtReview/CampaignExpansion/VALIDATION.md`.

Regional architecture and surface identity, **16 September 2026**: the new walls, floors, ceilings and regional architectural forms are integrated in the normal Windows build. Hell includes three distinct floor treatments; the approved sewer pack is retained. All 12 regression suites, the packaged 18-floor review (6,882 boundary sweeps), and the shared key/lever/crate review passed with zero failures. Original assets, artwork, saves, campaign data and unrelated code were preserved; display preferences were restored. Review: `J:\Lonemoore_Regional_Identity\REVIEW.html`. Handoff: `ArtReview/RegionalIdentity/HANDOFF.md`.

Regional art expansion, **16 September 2026**: eight additional regional surface packs, 24 decorative props, eight themed levers and six floating world-key designs are integrated in the normal Windows build. All 12 regression suites and the packaged 18-floor art/interaction/collision review passed. Original assets, artwork, campaign and saves were preserved. Use `Play Regional Art Test.cmd` to choose a region with campaign saving disabled. Sources and review: `J:\Lonemoore_Regional_Art`; handoff: `ArtReview/RegionalArt/HANDOFF.md`.

Shared interactable update, **16 September 2026**: improved 3D lever and loot crate, plus floating world keys, are in the normal Windows package. All 12 regression suites and the packaged interaction/animation review passed. The subsequent regional expansion was approved and integrated; see the regional art handoff above. Details: `SHARED_INTERACTABLES_UPDATE.md`.

Approved Old City Sewers art integrated into the main Windows build on **16 September 2026**. All 12 regression suites and packaged sewer review passed, including 754 collision sweeps. Start `Play Old City Sewers Art Test.cmd` for a direct test with campaign saving disabled. Normal campaigns also use the new sewer art. See `SEWER_ART_INTEGRATION.md`.

Current Windows build updated **16 September 2026**: supplied splash/loading art, explicit dungeon boundaries and persistent gates, integrated Blender environment kit, fog-of-war automap and line-of-sight radar. Launch with `Play First Person Dungeon Crawler.cmd`.

**Accessory hotfix:** selecting any ring or charm now shows **Equip to accessory 1** and **Equip to accessory 2**. Both slots accept either accessory type and preserve the other slot during swaps. The latest automation run passed all **12 suites**; evidence is under `Saved/AccessoryFix`.

All 11 automated suites and 1,800 seeded floor validations passed. The normal packaged launcher completed both 1600 × 900 and 2560 × 1080 gameplay/map reviews with zero failures. All 113 original art files, four existing saves and campaign data remain unchanged; display preferences were restored. See `INTEGRITY_UPDATE.md` for screenshots, measured performance and remaining art/ultrawide frame-pacing limits. The Phase 2 record below is historical.

Approved UI artwork was installed in the main Windows build on 2026-09-12. See `UI_ART_PASS.md`.

# Build status — Phase 2, 12 September 2026

**Playable Windows development build 0.2, Unreal Engine 5.8.2.**

Launch `Play First Person Dungeon Crawler.cmd`. The launcher respects saved display settings.
The packaged build is in `Builds/Windows`; source and editor binaries remain in this project.

Phase 2 includes startup splash/loading, class and transaction confirmations, pooled selling
across party bags and equipment, quality-adjusted power and upgrade comparisons, delayed
enemy turns, combat/town history, a smaller party HUD, parchment buttons/settings book,
display/audio/key controls, equipment icons, corrected town hit regions and key art,
consistent encounter groups, cleared-area prompts, guardian rewards and randomized names.
The authored lists contain 100 male and 100 female names; new journeys save their choices.

## Verification

- 2,126 content checks passed; zero failures.
- All six native suites passed, including Phase 2 economy/presentation rules.
- All three starters completed the 18-floor automated routes: 54 floor results, plus both endings and postgame assertions.
- Windows BuildCookRun completed successfully.
- Final editor module compilation and linking succeeded; the editor binaries are synchronized.
- Packaged review captured 27 actual rendered screens and exited normally.
- Packaged pointer checks, class selection, sale cancellation/confirmation, equipped sale, active/paused quicksave and quickload restoration passed.
- No Error, Fatal or failed-assertion entries were found in the packaged review log.
- Both existing packaged saves match their pre-packaging SHA-256 hashes. Backups remain in `Saved/Phase2Safety/PackagedSaveGames`.
- Supplied `.art` sources were not edited. The new splash is in `Content/Splash`; key-sheet sampling uses a separate material. Existing material instancing flags were fixed.

## Evidence

- `Saved/Validation/Phase2Automation/index.json`
- `Saved/Validation/phase2_tests.log`
- `Saved/Validation/phase2_package.log`
- `Saved/Validation/phase2_packaged_review.log`
- `Builds/Windows/DungeonCrawler/Saved/Validation/Screenshots`
- `Saved/Phase2Safety/save_hashes.json`

## Next owner playtest

Review selling, upgrade gains, guardian rewards, combat timing, UI size and audio together.
Enemy health/damage tuning is deferred until that loop is reviewed. Dungeon 3D models,
dressing, distinct regional graphics and descent doorways are recorded for **Phase 3** in
`PHASE_2.md`. Parchment styling and audio loudness still need owner taste/comfort review.
The requested overall playtime has not been established by automated routes.
