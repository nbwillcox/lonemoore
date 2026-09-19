# Performance pass — 17 September 2026

The optimized candidate is delivered to both `Builds/Windows` and
`Builds/ExpansionPrototype/Windows`. Launch `Play Lonemoore.cmd` normally.
Original artwork, player saves and display preferences are preserved.

## Matched packaged benchmarks

RTX 3060 12 GB; Ryzen 7 3700X; 32 GB RAM; DirectX 12 / SM6.
3440 × 1369 output, existing Epic settings, uncapped, VSync off. Same saved
layout, camera, party, exploration and enemy state in each before/after pair.
Both versions run offscreen, sequentially, with editor/compiler/cooker stopped.
Each 2,400-frame capture contributes 1,300 steady frames; startup and screenshot
frames are excluded. These fixed-camera samples do not establish a minimum
frame rate for every movement/combat sequence. Cold startup is excluded.

| Scene | Before FPS | After FPS | Improvement | 1% low before / after |
|---|---:|---:|---:|---:|
| Your existing save | 64.5 | 77.8 | +21% | 55.0 / 71.2 |
| Fully explored sewer floor | 57.5 | 74.0 | +29% | 51.3 / 68.6 |
| Cathedral chamber | 60.2 | 74.6 | +24% | 48.7 / 69.2 |
| Sewer bridge | 59.8 | 74.1 | +24% | 52.8 / 68.1 |
| Hellforge chamber | 61.5 | 76.4 | +24% | 53.3 / 70.9 |

FPS is 1,000 / mean frame milliseconds. The reported 1% low is 1,000 / the
99th-percentile frame milliseconds, not the mean of the slowest 1% of frames.

## Implemented

- Immutable architecture survives pickups, combat refreshes and gate changes.
  In the final packaged interaction review, same-floor refreshes took 6.4–7.7 ms;
  the architecture-build counter remained one throughout key/boss gate checks.
- Static hierarchical instance batches register once after population, with
  a single cluster-tree build. Visibility changes do not remove collision.
- Nanite enabled on 129 detailed opaque/masked meshes, with original assets
  backed up and full fallback geometry retained. Sprites retain their art.
  The 18 hero stair meshes keep their authored geometry after the visual review
  caught Nanite simplification of shallow inlays and bevels. Required Nanite
  material usage variants are prepared before cooking rather than on entry.
- HZB occlusion and smooth draw-distance limits on distant lights/scenery.
- Enemy billboard/material updates depend on actual camera/combat/light changes;
  unchanged doors no longer reset transforms and collision each tick.
- Radar/map drawing is restricted to the visible viewport. The small radar
  remains bounded even after the entire expanded floor is explored.
- Current-region meshes/materials preload asynchronously. Geometry, regional
  dressing and a short render warmup run in separate loading stages; controls
  remain blocked until the world and its collision are ready.
- Resource PSO precaching is enabled. This does not eliminate every first-use
  shader/pipeline event; the packaged logs retain first-use misses for diagnosis.
- Epic TSR history changes from 200% to 100% at the same output resolution.
  This reduces GPU cost with a possible loss of fine-detail sharpness/stability
  during motion. Cinematic engine configuration retains 200%.

## Validation

- 19 native suites completed; zero failed or skipped.
  Eight suites recorded an unrelated connectivity-probe timeout warning.
- Packaged 18-floor review: 92,808 physical boundary sweeps, zero failures.
- Separate sewer interaction review: 4,860 sweeps; no failures. Covers locked
  doors, original Rat King encounter, guardian seal, stairs, fog, player torch,
  enemy presentation, retained architecture and existing-save continuation.
- All 124 protected files match pre-pass SHA-256 hashes: 117 original artwork
  files, five player saves and two display-settings files.
- Candidate/main/prototype executable and cooked-content hashes match.
- Rendered before/after captures are included without retouching. Cathedral,
  sewer, Hell and the continued-save views were inspected for missing geometry,
  material faults, fog/radar changes and lighting regressions.

## Evidence and reproduction

- `Saved/PerformancePass/performance_results.json` and `Before/`, `After/` CSVs.
- `Saved/PerformancePass/Automation/index.json`, `tests.log`, `package_final.log`.
- `Saved/PerformancePass/campaign_review.log`, `prototype_review.log`.
- `Saved/PerformancePass/nanite_assets.json`, `preservation.json`.
- `Tools/Profile-Dungeon.ps1` runs only in the two isolated benchmark packages;
  `Tools/analyze_dungeon_performance.py Before After` reproduces the table.
- `Builds/PerformanceBaseline/Windows` retains the previous executable/content.
  Source, configuration, modified meshes, saves and settings were backed up
  under `Saved/PerformancePass/Backup`. Benchmark saves never use live save slots.

Epic references: [instancing and culling](https://dev.epicgames.com/documentation/unreal-engine/instanced-static-mesh-component-in-unreal-engine),
[Nanite geometry](https://dev.epicgames.com/documentation/unreal-engine/nanite-virtualized-geometry-in-unreal-engine),
[TSR history quality and cost](https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine).
