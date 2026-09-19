"""Write release notes only after the recorded final build has been installed."""
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / 'Saved/AdventurePolish'
REVIEW = ROOT / 'ArtReview/AdventurePolish'

def read(name):
    return json.loads((EVIDENCE / name).read_text(encoding='utf-8-sig'))

promotion = read('promotion.json')
performance = read('performance_release_evidence.json')
tests = read('AutomationFinal/index.json')
assert promotion['status'] == performance['status'] == 'PASS'
assert len(promotion['targets']) == 3 and all(t['status'] == 'PASS' for t in promotion['targets'])
assert performance['sceneCount'] == 7
assert promotion['executableSha256'].lower() == performance['candidateExecutableSha256'].lower()
assert tests['succeeded'] + tests['succeededWithWarnings'] == 35 and tests['failed'] == 0
fps = '–'.join(f'{n:.1f}' for n in performance['fpsRange'])
p99 = '–'.join(f'{n:.3f}' for n in performance['p99MsRange'])
files = len(promotion['runtime'])
protected = promotion['protectedFiles']
REVIEW.mkdir(parents=True, exist_ok=True)

handoff = f'''# Adventure polish — 18 September 2026

Installed and ready through **Play Lonemoore.cmd**. Choose **New Game** for the smaller floors and named-character saves.

- Enemies sit lower, accounting for transparent space beneath the original sprite artwork. Shrine visuals are centered on their actual map tiles.
- Dungeon lighting is dimmer; the local player torch remains. The approved artwork and regional props are retained.
- Floors use **68 sections instead of 104**, a 34.6% reduction. Each still has **45–46 regular encounter sites**, with an early encounter and companion within two room exits.
- A save shrine sits beside the descent stairs. It becomes usable after the required key and guardian gates, then autosaves and adds a town return destination.
- All **seven existing classes** can start a journey. Enter a character name; each character has a separate folder, Auto, Quick and multiple manual saves. Companions adjust so every starter can form a party of five different classes.
- Ordinary chests use the full **33-item catalog**, including weapons, armor, food, potions and relics. Better quality becomes more likely deeper down; sanctuary chests favor equipment. Full bags leave the fixed reward available for later.
- **27 original effects** include a retro fireball, metallic attacks and a distinct cue for each of the 21 active skills. [Listen to the sound library](Audio/REVIEW.html).

Normal campaign storage: `Builds/Windows/DungeonCrawler/Saved/SaveGames/Characters/<character name>/`. Create another name for another journey. Manual Save opens the save list; choose **Create new manual save**. Load lets you select both the character and saved moment. Old flat saves remain on disk but do not appear in the new browser.

All **35 automated suites** and the packaged **18-floor review** passed. Seven warmed stationary RTX 3060 captures at 3440 × 1369 output measured **{fps} FPS**. These measurements do not represent a sustained playthrough. The new 68-section layouts differ from the earlier benchmark layouts.

The final runtime was installed and hash-checked in the normal campaign, expanded prototype and authored-room playtest: **{files} files per target**. Installation left **{protected} existing save/settings files unchanged**. The old RoomKitPolish candidate remains a separate baseline.

[Game screenshots and review](REVIEW.html) · [Validation details](VALIDATION.md) · [How to play](../../HOW_TO_PLAY.md)
'''
(REVIEW / 'HANDOFF.md').write_text(handoff, encoding='utf-8')

validation = f'''# Adventure polish validation

Final candidate: `Builds/AdventurePolishCandidate/Windows`. Editor build, game build and packaging completed successfully. The installed executable matches the native and performance capture executable: `{promotion['executableSha256']}`.

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

Seven RTX 3060 scenes, 3440 × 1369 output, existing Epic graphics settings and automatic render resolution: **{fps} mean FPS**, with **{p99} ms** 99th-percentile frame times. Each capture warmed for eight seconds, recorded 2,400 frames, and analyzed 1,300 central samples. VSync and frame limits were disabled for measurement only. Installed settings were unchanged. These stationary offscreen runs do not measure sustained traversal, loading hitches or monitor tearing. Earlier 104-section layouts are contextual comparisons, not identical workloads.

## Delivery and limits

Installation verified **{files} runtime files in each of three launch locations** against the candidate hashes; **{protected} existing saves/settings files** remained unchanged. The installation excluded all Saved directories.

The player-facing browser targets fresh named journeys. Retired 104-section room-kit saves are intentionally unsupported. Native clipboard shortcuts and pagination beyond six entries were not exercised in the rendered walkthrough. Automated failure fixtures cover creation rollback; physical power loss was not tested. A sustained human playtest remains the next check for pacing, atmosphere and the audio mix.

Evidence is in `Saved/AdventurePolish`: `AutomationFinal/index.json`, `packaged_adventure.log`, `packaged_all_floors.log`, `campaign_routes.txt`, `enemy_distribution_results.txt`, `chest_catalog_quality.txt`, `combat_audio_validation.json`, `combat_audio_import.json`, `performance_release_evidence.json`, and `promotion.json`. Native PNGs and text reports remain under the candidate's `DungeonCrawler/Saved/AdventurePolish` and `RoomKit` folders. Earlier failed development runs are retained as history; the final report is `AutomationFinal`.
'''
(REVIEW / 'VALIDATION.md').write_text(validation, encoding='utf-8')

current = f'''Adventure polish, **18 September 2026**: installed in the normal campaign, expanded prototype and authored-room playtest. Start **Play Lonemoore.cmd → New Game**. All seven existing classes are available, naming is required, and each character has separate Auto/Quick slots and multiple manual saves in its own folder. Companion substitution lets every starter form a full party.

New floors use **68 sections instead of 104 (34.6% fewer)** with **45–46 regular encounter sites**. A protected post-boss save shrine sits beside the descent stairs. Chests draw from all **33 catalog items**, favoring better quality at depth. Enemies are grounded using their original sprite transparency; shrine art matches its map tile; lighting is dimmer. **27 original effects** include a retro fireball, metallic attacks and separate cues for all 21 active skills. The current artwork, room kit and regional props are retained.

All **35 automated suites** passed, including 432 seeded encounter-distribution cases and full campaign routes for all seven starters. The packaged journey/save walkthrough passed **31 checks**; the packaged geometry review passed **18 floors and 72,758 physical boundary sweeps**, with no failures. Seven warmed stationary RTX 3060 captures at 3440 × 1369 output measured **{fps} FPS**, with **{p99} ms** 99th-percentile frames. Changed layouts prevent an identical-scene comparison; sustained play and subjective audio/atmosphere remain for player evaluation.

The installation verifies **{files} runtime files in each of three targets**, leaving **{protected} existing save/settings files unchanged**. New named journeys are the supported player path; old flat saves are not listed. See the [current review](ArtReview/AdventurePolish/REVIEW.html), [handoff](ArtReview/AdventurePolish/HANDOFF.md), [validation](ArtReview/AdventurePolish/VALIDATION.md), [sound library](ArtReview/AdventurePolish/Audio/REVIEW.html) and [installation evidence](Saved/AdventurePolish/promotion.json). Earlier updates below are historical.

'''
status_path = ROOT / 'BUILD_STATUS.md'
history = status_path.read_text(encoding='utf-8-sig')
if history.startswith('Adventure polish, '):
    history = history[history.index('Room-kit polish, '):]
status_path.write_text(current + history, encoding='utf-8')

issues_path = ROOT / 'KNOWN_ISSUES.md'
issues = issues_path.read_text(encoding='utf-8-sig')
old_line = next(line for line in issues.splitlines() if line.startswith('- Seven warmed stationary RTX'))
new_line = f'- Seven warmed stationary RTX 3060 captures at 3440 × 1369 output measured {fps} FPS, with 99th-percentile frame times of {p99} ms. Layouts changed from 104 to 68 sections, so the prior results provide context rather than identical-scene comparisons. These uncapped offscreen measurements do not establish full-route performance or eliminate every hitch, monitor tear or temporal flicker. See the [current performance evidence](Saved/AdventurePolish/performance_release_evidence.json); earlier measurements remain in [build history](BUILD_STATUS.md).'
issues_path.write_text(issues.replace(old_line, new_line), encoding='utf-8')

design = read('generation_and_loot_design.json')
design['status'] = 'PACKAGED_VALIDATED_AND_INSTALLED'
design['releaseEvidence'] = 'promotion.json'
(EVIDENCE / 'generation_and_loot_design.json').write_text(json.dumps(design, indent=2) + '\n', encoding='utf-8')
print(f'RELEASE_NOTES_READY runtime_files={files} targets=3 protected={protected} fps={fps}')
