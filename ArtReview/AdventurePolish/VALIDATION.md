# Adventure polish validation

Final candidate: `Builds/AdventurePolishCandidate/Windows`. Editor build, game build and packaging completed successfully. The installed executable matches the native and performance capture executable: `FA44F9B753DA02C82168EC067DB09F62E9BC1F74FAE28E381AAA4A4643AD9D0F`.

## Gameplay and saves

- Final automation: **35 passed, 0 failed, 0 unfinished**. Five successful suites carry engine connectivity or optional death-ledger warnings; no functional failures remain.
- Every starting class completed the 18-floor campaign route: **126 floor results**, with recruitment, supplies, paid town recovery and ending assertions. This checks solvability, not full-run comfort or balance.
- **432 seeded layouts** passed encounter coverage, roster distribution, early fights and companion access. Every floor has 68 sections and 45–46 regular encounter sites.
- Chest samples: 4,096 each at floors 1, 10 and 18 reached all 33 items; mean quality rose from **0.859 → 2.064 → 3.130** on the 0–4 quality scale. Full-bag retry, deterministic contents, no duplicate claims and save/load checks passed.
- Descent checkpoints passed key/guardian requirements, activation and town return across all 18 floors.
- The final packaged journey review passed **31 checks** and produced **12 native screenshots**. It exercised keyboard naming, all seven class controls, two separate characters, Auto/Quick/two manual saves, selected-snapshot loading, shrine and enemy alignment, checkpoint locking, real disk autosave/reload and town return. Review characters were isolated and removed.

## Geometry and presentation

The final packaged room review passed **18 floors and 72,758 physical boundary sweeps**, with zero failures. Key and living-guardian gates blocked descent; legal completion opened the intended route. Architecture was reused during exploration and gate changes. Six representative room types were captured across the campaign.

Enemy placement uses alpha-bound metadata for 30 original sprites; original artwork was not altered. Native checks verified feet placement and exact shrine tile centers. Player and room fill lighting were reduced while retaining local visibility. Representative rendered views were inspected; this is not an exhaustive moving-camera flicker test.

## Audio and performance

All **27 sound assets imported**, and native startup confirmed they were cached before play. The 21 active skills have separate cues. Waveforms are deterministic, mono, unclipped and at least 6.3 dB below full-scale peak; total decoded PCM is 1,773,702 bytes. Invalid casts remain silent and multi-target skills play one cue per action. The final offscreen reviews use sound disabled, so these checks establish assets and routing rather than subjective listening or the final audible mix.

Seven RTX 3060 scenes, 3440 × 1369 output, existing Epic graphics settings and automatic render resolution: **72.3–75.9 mean FPS**, with **14.289–15.130 ms** 99th-percentile frame times. Each capture warmed for eight seconds, recorded 2,400 frames, and analyzed 1,300 central samples. VSync and frame limits were disabled for measurement only. Installed settings were unchanged. These stationary offscreen runs do not measure sustained traversal, loading hitches or monitor tearing. Earlier 104-section layouts are contextual comparisons, not identical workloads.

## Delivery and limits

Installation verified **49 runtime files in each of three launch locations** against the candidate hashes; **66 existing saves/settings files** remained unchanged. The installation excluded all Saved directories.

The player-facing browser targets fresh named journeys. Retired 104-section room-kit saves are intentionally unsupported. Native clipboard shortcuts and pagination beyond six entries were not exercised in the rendered walkthrough. Automated failure fixtures cover creation rollback; physical power loss was not tested. A sustained human playtest remains the next check for pacing, atmosphere and the audio mix.

Evidence is in `Saved/AdventurePolish`: `AutomationFinal/index.json`, `packaged_adventure.log`, `packaged_all_floors.log`, `campaign_routes.txt`, `enemy_distribution_results.txt`, `chest_catalog_quality.txt`, `combat_audio_validation.json`, `combat_audio_import.json`, `performance_release_evidence.json`, and `promotion.json`. Native PNGs and text reports remain under the candidate's `DungeonCrawler/Saved/AdventurePolish` and `RoomKit` folders. Earlier failed development runs are retained as history; the final report is `AutomationFinal`.
