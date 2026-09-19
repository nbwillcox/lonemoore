# Phase 2 — gameplay and interface

Owner request: 12 September 2026. Dungeon 3D graphics are deferred to Phase 3.

## Implemented in build 0.2

- Startup splash and incremental artwork loading screen.
- Permanent class choice confirmation, purchase and single-item sale confirmation.
- Town hit regions aligned to the illustrated alley, smith, board and entrance.
- Merchant pools all eligible party bags and equipment; equipped sales carry an explicit warning.
- Actual quality-adjusted equipment power, current/next upgrade power and hero totals.
- Strictly increasing quality power, including low-power armor and daggers.
- Timed enemy turns with disabled action controls and visible preparation/result time.
- Shared deterministic encounter roster for exploration and combat, including groups.
- Empty searched/opened sections and cleared encounters use ordinary passage prompts.
- Compact party strip, recent events and paginated journey history saved with the run.
- Parchment-style buttons and a settings book with graphics, audio and key pages.
- Full equipment slot icons from supplied atlases.
- Separate saved master/music/effects levels, display options, brightness and key remapping.
- Advanced graphics controls for shadows, textures, view distance, anti-aliasing, effects and post processing.
- Guardian equipment plus 500 + 125 per floor gold; full bags retain unclaimed equipment.
- Unneeded healer actions disabled and guarded in gameplay rules.
- 100 male and 100 female names; names assigned at new game and retained in saves.
- Key artwork uses the appropriate column in the supplied key sheet.
- Stronger player lantern for navigability; movement timing preserved.

## Phase 3 — recorded, not implemented

- Rebuild/refine dungeon 3D models, materials and layout presentation.
- Vines, torches, dirt, spider webs, clutter and regional environmental storytelling.
- Stronger doorway and descent geometry, with distinct transitions into deeper levels.
- Distinct visual identity for each depth/region and a comprehensive lighting art pass.

## Balance and remaining review

Enemy health and damage are unchanged. Tune after playtesting selling, upgrade clarity,
quality gains and guardian rewards together. Human review of economy, audio loudness,
hotspot placement, icon mapping and parchment style remains necessary.
Original art sources remain untouched. Existing save fields remain compatible; older
heroes without saved names retain their class labels. Phase 2 does not claim final art.

## Verification

- All 2,126 campaign content checks passed.
- All six native automation suites passed, including all-starter campaign routes and Phase 2 regressions.
- Editor build succeeds on the installed Unreal Engine 5.8.2.
- Editor rendered review passed pointer checks and active/paused quicksave/quickload checks.
- Windows packaging succeeded. The packaged review exited cleanly after capturing 27 screens.
- Packaged pointer checks, active/paused quicksave, quickload confirmation and equipped-item sale passed.
- Inspected packaged loading, party layout, settings pages, timed combat and transaction screens.
- Both existing packaged saves are byte-for-byte unchanged after packaging and the review.
- Existing packaged saves were copied into `Saved/Phase2Safety/PackagedSaveGames` before packaging.
