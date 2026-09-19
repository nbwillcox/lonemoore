# Campaign expansion validation — 17 September 2026

Delivered in the normal Windows development build and the separate prototype build. Open `Play Lonemoore.cmd` for the campaign or `Play Expanded Dungeon Prototype.cmd` for the Cistern starting point. The executable and cooked content match across both packages; their saves remain separate.

## Results

| Check | Result |
|---|---|
| Editor and Windows game builds | Passed |
| Windows cook, stage and archive | Passed, exit 0 |
| Native regression suites | 17 passed, 0 failed or skipped |
| Expanded floor generation | 1,800 layouts: 100 unique, reproducible layouts per floor |
| Deliberate key/boss bypass injections | All 2,700 rejected |
| Walkable area | 1,793–2,848 cells across the tested seeds; every floor exceeds 10 times its former generated area |
| Regular enemies | Exactly 10 times each floor's original encounter rosters for the same party and run |
| Full campaign routes | All 18 floors with all 3 starting classes: 54 floor results passed, recruitment and both endings passed |
| Packaged campaign review | 18 floors, 92,808 physical boundary sweeps, 0 failures, exit 0 |
| Packaged prototype review | 4,860 physical sweeps, 0 failures, exit 0 |
| Existing prototype autosave | Loaded successfully; exact saved floor plan retained; save writes disabled |
| New stair geometry | All 18 meshes have no upward-facing surface coplanar with the floor; first tread top is 18 cm above it |
| Protected files | All 124 tracked original-art, save and settings files unchanged after both packaged reviews |

The full campaign route visits every regular encounter and uses normal combat, supplies, upgrades and town services. The separate camera fixture opens seals after testing their prerequisites so the stairs can be inspected; it does not write saves.

The gallery contains 72 actual packaged captures: stairs, side details, chambers and crossings for every floor. Regional views were inspected, including the corrected fire-pit materials and the continued prototype save. [Continued prototype screenshot](Prototype_Continued_Save.png).

## Save continuity

Loading never regenerates a stored layout. Explored floors, markers, opened doors, defeated enemies, shrines and corpse-recovery locations remain intact. An untouched older floor expands on first entry; new journeys use expanded layouts throughout. New stairs and player lighting also appear on older explored floors.

The isolated save migration test passed, and the four existing campaign saves passed the legacy-load regression. SHA-256 comparisons after packaging and both runtime reviews found no changes to the 124 protected files. The old campaign executable/content, source, configuration, saves and settings are backed up under `Saved/CampaignExpansion/Backup`.

## Evidence

All paths below are relative to the project folder:

- `Saved/CampaignExpansion/Automation/index.json` and `tests.log`: native suite results.
- `Saved/CampaignExpansion/seed_results.txt`: per-floor areas, enemy counts, uniqueness and bypass checks.
- `Saved/CampaignExpansion/campaign_routes.txt`: three complete class routes.
- `Saved/CampaignExpansion/runtime_results.txt` and `packaged_review.log`: all-floor packaged checks.
- `Saved/CampaignExpansion/prototype_runtime_results.txt` and `prototype_review.log`: prototype checks and existing-save resume.
- `Saved/CampaignExpansion/stair_geometry_audit.json`: geometry inspection.
- `Saved/CampaignExpansion/preservation.json`: protected-file and delivered-package hashes.
- `Saved/CampaignExpansion/package.log`: successful package creation.

Four passing native suites recorded unrelated engine HTTP connectivity timeouts. Packaged logs contain engine startup warnings for the TLS certificate store, an unspecified default pawn class and the motion-vector console variable; neither packaged review logged errors, failed checks, missing scene assets or material-usage fallback warnings. These automated runs and rendered captures verify routes and integration; they do not establish the duration or subjective pacing of a manual campaign playthrough.
