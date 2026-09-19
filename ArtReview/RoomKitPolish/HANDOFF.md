# Room-kit polish handoff — 18 September 2026

**Installed and verified.** The normal campaign, expanded prototype and separate authored-room playtest now use the validated polish build. [Open the current review](REVIEW.html) for native captures and comparisons. The [original room-kit review](../RoomKit/REVIEW.html) remains as history.

The curved floors were rebuilt with smaller, bounded triangles while preserving their footprint and texture scale. Circular rooms, rounded turns and caverns now show continuous foreground floor with Nanite enabled. The existing 25-room assembly, torch lighting, fog of war, themed stairs and key/guardian gates are retained.

Fresh floors now place **70–72 regular encounter sites** in about 70% of ordinary exploration rooms. These are encounter groups, not a count of individual enemies. Each original roster is reused at least ten times; enemy artwork, types, stats and rewards are unchanged. The first eligible fight and recruitable companion are within two room exits. For existing room-kit saves, an additive repair touches only completely undiscovered rooms without recorded progress. It protects cleared fights, nearby occupied rooms, completed floors and corpse recovery; repeat loads do not accumulate enemies. Older saved layouts retain their architecture.

**24 new Blender props across nine themes** add votive stands, bone shelves, sewer fittings, barricades, crypt furnishings, armory pieces, drake remains and infernal decorations. The props use 45 regional material instances based on the existing artwork. Original art is unchanged. [Editable prop library](../../ArtSource/RegionalDressing/README.md).

| Installed runtime | Verified files |
| --- | ---: |
| `Builds/Windows` — normal campaign | 49 |
| `Builds/ExpansionPrototype/Windows` — expanded prototype | 49 |
| `Builds/RoomKitCandidate/Windows` — authored-room playtest | 49 |

All three runtime trees match the validated candidate by SHA-256. **145/145 protected art, save and settings files are unchanged**, and every `Saved` directory was excluded from installation. Four generated performance-test fixtures changed when their automation suite ran; their baseline/current hashes and generator provenance are recorded separately. The original 149-row baseline was not rewritten. [Promotion and preservation report](../../Saved/RoomKitPolish/promotion.json).

Validation passed **25 automated suites**, including 432 seeded encounter cases, safe migration checks and all starting-class story routes. Route tests recruit companions and use ordinary carried/purchased supplies, selling and paid town recovery; they do not grant free stats or bypass combat. The packaged room/gate review passed **18 floors and 115,089 physical boundary sweeps**. All 25 room meshes passed floor-coverage checks and 12,672 sampled walking-lane rays. The packaged dressing review covered 18 floors, 48 detail captures and 114 native checks with zero failures.

The final floor review inspected 12 directional screenshots: **ten visibly expose continuous floor; two face nearby walls; zero visible floor defects**. All 24 prop types were visually inspected with no observed floating gap, obvious central-passage blockage or missing/default material. The procession banner and armory remain nearly edge-on in their selected views, limiting frontal-detail judgment. These still-image checks do not establish every instance, moving-camera position or streaming state. [Floor observations](../../Saved/RoomKitPolish/packaged_floor_visual_review.md) · [Prop observations](../../Saved/RoomKitPolish/packaged_prop_visual_review.md).

Seven warmed stationary scenes on an **RTX 3060**, at **3440 × 1369 output, Epic quality and the existing automatic render-resolution setting**, averaged **73.6–75.6 FPS**. Their 99th-percentile frame times ranged from **14.268–14.942 ms**. Five scenes matched the prior build within **1.2%**; the additional Deep and Hell entrances measured **75.1** and **75.5 FPS**, without older matched comparisons. Each scene uses 1,300 warmed samples from an uncapped offscreen run; VSync was disabled only for benchmark processes. [Measured comparison](../../Saved/RoomKitPolish/performance_comparison.json).

Launch **Play Lonemoore.cmd** for the campaign or **Play Authored Room Dungeon.cmd** for the separate crypt playtest. Choose **3** for the reviewed crypt seed, **2** for a new random layout or **1** to continue that playtest. Campaign saves and playtest saves remain separate.

Sustained player pacing, resource balance and the requested 15–25-hour campaign length remain untuned. The measurements do not establish loading-hitch behavior across a full route or the absence of monitor tearing and temporal flicker. Those require a moving, on-display playtest. [Current known issues](../../KNOWN_ISSUES.md).
