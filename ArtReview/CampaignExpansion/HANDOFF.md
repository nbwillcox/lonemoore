# Expanded campaign and gothic stairs

The approved Cistern atmosphere has been extended to the eighteen-floor campaign. Launch `Play Lonemoore.cmd` from the project folder. The prototype launcher remains separate and keeps its own save slots.

## What is included

- At least ten times the former generated floor's walkable area and exactly ten copies of its regular encounter rosters. Bosses, enemy identities and original enemy artwork are retained.
- Seeded room shapes and connections, side routes, optional lever vaults, secret treasuries, supplies, waypoint shrines and permanent encounter clearing.
- Regional room profiles: chapels and cloisters, sewer chambers, burial galleries, warrens, crypts, military halls, caverns, ritual ruins and Hell. Existing regional wall/floor meshes, surface materials, props and decals remain in use.
- Water crossings in the sewers, chasm crossings elsewhere and fire-lit pits in Infernal/Hell regions. Drops remain impassable; bridges and rails use the same explicit movement boundaries.
- The approved lower ceilings, structural supports, warm player torch, ambient fill, distance-balanced world enemy sprites and persistent map/radar fog.
- Eighteen authored gothic stair meshes with regional materials, pointed framing, carved balustrades and crests. The lowest stair tread is above the floor rather than coplanar with it. The old stair mesh is no longer placed.
- Existing key gates, an additional boss seal where a floor has a boss, recruitment, hunts and Astra's final encounter. Floors that originally had no guardian retain key-gated progression without inventing another boss.

## Existing journeys

Loading a saved journey never regenerates its stored layouts. Explored floors, opened doors, discoveries, markers, defeated encounters, shrines, party data, inventories and corpse-recovery locations are preserved.

An older, unvisited floor expands when entered for the first time. Any evidence of progress or a corpse recovery prevents that conversion. New journeys generate expanded floors throughout. New stair art and player lighting also apply to older explored floors without changing their layout.

The normal campaign keeps `Lonemoore_` saves; the optional prototype keeps `ExpansionPrototype_` saves in its separate package. This work does not copy one journey over another. Saves and settings were backed up before editing and are hash-checked after validation.

## Sources and checks

- Stair source and exports: `ArtReview/CampaignExpansion/Stairs/Gothic_Stairs.blend`, eighteen FBX files and `manifest.json`.
- Asset construction/import: `Tools/build_gothic_stairs.py` and `Tools/import_campaign_expansion.py`.
- Geometry audit: `Tools/audit_gothic_stairs.py` examines upward-facing surfaces for overlap with the floor plane.
- Logical generation, save-continuity and campaign tests run in the `Dungeon` automation group. The campaign route now visits every regular encounter across all eighteen floors, for all three starting classes, using ordinary combat, supplies and town services.
- `-CampaignExpansionReview` produces actual game screenshots and physical boundary checks for all floors. `-ReviewFirst=N -ReviewLast=N` narrows the range using zero-based floor numbers. Review saves are disabled.
- `Tools/make_campaign_expansion_review.py` publishes a gallery from the packaged screenshots.

Evidence is under `Saved/CampaignExpansion`. The pre-rollout campaign executable/cooked containers and source/save backups are under `Saved/CampaignExpansion/Backup`. Restore a package as a consistent executable-and-content set if rollback is required.

This remains a packaged development build. Automated playthroughs establish workable routes and story outcomes; they do not replace the owner's playtest of pacing and atmosphere.
