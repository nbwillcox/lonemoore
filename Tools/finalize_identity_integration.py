"""Publish the integration handoff only after packaged and visual validation."""
from pathlib import Path
import hashlib
import html
import json
import re
import shutil

G = Path(r'J:\First Person Dungeon Crawler Game')
R = Path(r'J:\Lonemoore_Regional_Identity')
E = G / 'Saved/RegionalIdentity'
D = G / 'ArtReview/RegionalIdentity'
S = G / 'Builds/Windows/DungeonCrawler/Saved/RegionalIdentity'
build = (E / 'build.log').read_text(errors='replace')
assert 'BUILD SUCCESSFUL' in build
tests = (G / 'Saved/Validation/tests.log').read_text(errors='replace')
assert 'Automation Test Queue Empty 12 tests performed' in tests
assert 'Result={Fail' not in tests
regional = (E / 'packaged_review.log').read_text(errors='replace')
shared = (E / 'shared_packaged_review.log').read_text(errors='replace')
assert 'REGIONAL_REVIEW_COMPLETE floors=18 sweeps=6882 failures=0' in regional
assert 'SHARED_PROP_REVIEW_COMPLETE failures=0' in shared
for log in [regional, shared]:
    assert not re.search(r'\b(?:Error|Fatal):|REGIONAL_QA FAIL|SHARED_PROP FAIL|Assertion failed:', log)
preservation = json.loads((E / 'preservation.json').read_text())
assert not preservation['mismatches'] and preservation['preferences_restored']
visual = json.loads((E / 'visual_review.json').read_text())
assert visual['status'] == 'PASS'
captures = sorted(S.glob('Floor_*.png'))
assert len(captures) == 108

report = dict(status='INTEGRATED_PACKAGED_VALIDATED', engine='Unreal Engine 5.8.2',
              regression_suites=12, floors=18, boundary_sweeps=6882, failures=0,
              packaged_screenshots=len(captures), shared_interactable_review='PASS',
              preservation=preservation, visual_review=visual,
              game_executable='Builds/Windows/DungeonCrawler/Binaries/Win64/DungeonCrawler.exe')
report['package_sha256'] = hashlib.sha256((G / report['game_executable']).read_bytes()).hexdigest()
(E / 'validation_summary.json').write_text(json.dumps(report, indent=2))
for name in ['validation_summary.json', 'preservation.json', 'visual_review.json', 'integration.json', 'accepted_package_backup.json']:
    shutil.copy2(E / name, R / 'reports' / name)
for name in ['runtime_results.txt']:
    shutil.copy2(S / name, R / 'reports' / name)
shutil.copy2(E / 'SharedPropUpdate/runtime_results.txt', R / 'reports/shared_runtime_results.txt')
shutil.copy2(G / 'Saved/Validation/tests.log', E / 'regression_tests.log')

M = json.loads((R / 'manifest.json').read_text())
M['status'] = report['status']
M['runtime_changes'] = 'Integrated in main: regional visual modules, world-scaled wall materials, moving secret-door visual children, and an explicit cook directory. Original collision components remain authoritative.'
M['packaged_validation'] = {k: v for k, v in report.items() if k not in ['visual_review', 'preservation']}
M['integration_handoff'] = 'HANDOFF.md'
(R / 'manifest.json').write_text(json.dumps(M, indent=2))
integration = json.loads((E / 'integration.json').read_text())
integration['status'] = report['status']
for row in integration['files']:
    assert hashlib.sha256((G / row['path']).read_bytes()).hexdigest() == row['sha256'], row['path']
(E / 'integration.json').write_text(json.dumps(integration, indent=2))
shutil.copy2(E / 'integration.json', R / 'reports/integration.json')

gameplay = R / 'previews/gameplay'
gameplay.mkdir(exist_ok=True)
for path in captures:
    shutil.copy2(path, gameplay / path.name)
for path in (D / 'Runtime').glob('*.jpg'):
    shutil.copy2(path, gameplay / path.name)
page = (R / 'REVIEW.html').read_text(encoding='utf8')
old = '<p><b>Integration pending.</b> These are isolated Unreal tests, not captures from a newly packaged main game. Existing keys, levers, crates, decals and enemies are retained by the proposed integration.</p>'
new = '<p><b>Integrated and tested in the main Windows game.</b> All 18 floors passed the packaged review, including 6,882 collision checks, themed key pickup and floating motion, levers and gates, map behavior and existing 2D enemy combat. All 12 regression suites and the crate/lever/key review passed. Existing saves and display preferences were preserved.</p>'
assert old in page or new in page
page = page.replace(old, new)
if '<!-- MAIN_GAME_REVIEW -->' not in page:
    section = '<!-- MAIN_GAME_REVIEW --><section><h2>Actual packaged game</h2><p>These views include the normal game camera, lighting, interface and regional dressing. The controlled comparisons below remain labeled as isolated Unreal renders.</p>'
    for start in [1, 7, 13]:
        section += f'<img loading="lazy" src="previews/gameplay/Passage_{start:02d}-{start+5:02d}.jpg" alt="Packaged floors {start} through {start+5}">'
    section += '<details><summary>All 18 floors: entrance, passage, key, lever, map and combat</summary>'
    for floor in range(1, 19):
        section += f'<h3>Floor {floor}</h3><div class="pair">'
        for role in ['Entrance', 'Passage', 'Key', 'Lever', 'Map', 'Combat']:
            src = f'previews/gameplay/Floor_{floor:02d}_{role}.png'
            section += f'<div><a href="{src}"><img loading="lazy" src="{src}"></a><small>{html.escape(role)} · packaged main game</small></div>'
        section += '</div>'
    section += '</details></section>'
    page = page.replace('<img src="previews/Hell_vs_Sewers_Unreal.jpg"', section + '<img src="previews/Hell_vs_Sewers_Unreal.jpg"', 1)
(R / 'REVIEW.html').write_text(page, encoding='utf8')
for link in re.findall(r'<img[^>]+src="([^"]+)"', page):
    assert (R / link).is_file(), link

handoff = '''# Regional identity — integrated main-game handoff

Status: integrated in the normal Windows game and validated in the packaged executable on 16 September 2026. User approval explicitly authorized this main-game integration. REVIEW.html separates actual game captures from the earlier isolated Unreal and Blender comparisons.

## Assets and region mapping

227 new runtime assets under /Game/RegionalIdentity: 107 native 2K texture maps, 50 architectural meshes, two parent materials and 68 regional instances. Editable production remains in this sibling art workspace: eight .blend scenes, procedural physical-height sources, exported FBX and PNG files, repeatable scripts, and the full placement/scale manifest. The original 698 main-project assets and 113 reference art files were not overwritten.

Floor numbers here are player-facing: 1 Cathedral; 2–3 approved Old City Sewers (retained); 4–5 Catacombs; 6–7 Warrens; 8–9 Crypts; 10–11 Fortress; 12–13 Deep; 14–15 Infernal; 16–18 Hell. Hell uses cooled slag, foundry plates and broken dark stone across its three floors. Stable region IDs and all established lore remain unchanged.

## Runtime behavior

DungeonIdentityArt.h/.cpp maps cosmetic module paths. DungeonEnvironment.cpp adds NoCollision regional wall, niche, floor, ceiling, arch and upper-cap instances at the existing module transforms. Original physical meshes and seals retain their original collision settings. Their old surfaces are hidden only after a replacement mesh loads. Missing replacements log an error and retain the original visible geometry. Moving secret doors keep their original actors and collision, with regional cosmetic children attached. Tall walls use the world-projected material variants at the intended physical scale. DungeonRegionalArt.cpp removes obsolete flat lower-wall repair strips so the new facing remains visible. Existing regional props, decals, crates, themed levers, floating keys and 2D enemies remain in use.

Only three existing rendering/review source files and the cook-directory list in DefaultGame.ini changed; two rendering helper files were added. Layout generation, progression, collision definitions, map/radar, combat, inventory, balance, saves and campaign data were not edited. Only Materials, Textures and Meshes were promoted; isolated test maps and the sewer benchmark were excluded.

## Texture and geometry conventions

All materials use native 2048×2048 textures at a 2 m repeat (1024 pixels/m). Base color is sRGB, with no painted lighting or perspective. Normal maps are DirectX, linear BC5 with no Unreal green flip; Blender flips green for its own preview. Roughness, metal and emission masks are linear. UV and world-projected parents expose tint, wetness, roughness, detail strength and physical tiling. Deep recesses, burial shelves, braces, cavern facets and ceiling forms use geometry. No 4K assets or upscaled source maps are used. Tile boundaries, UV scale, normal conventions and isolated lighting were checked before integration.

## Completed validation

All 12 existing regression suites passed; the Windows build and cook/package completed successfully. The actual packaged game reviewed all 18 floors, captured 108 views, and passed 6,882 boundary capsule sweeps with zero mismatches. Every non-sewer region loaded visible collision-free architectural batches; sewers retained the approved pack. Themed key identity, pickup and bounded floating motion, lever states and gates, map movement blocking and existing 2D enemy combat passed. The separate shared-prop review passed crate loot, key registry cleanup and lever behavior. The packaged logs contain no Error/Fatal or failed-check entries. Contact sheets and full-size game captures were visually inspected; reports/visual_review.json records the scope.

Save and preference preservation was hash-checked after restoring display preferences. All 25 baseline save files, campaign data, original assets/reference art and unrelated source/config files match the baseline. This is packaged development-build validation and visual inspection, not a claim of a full manual campaign playthrough.

## Playing and later changes

Use the normal Play First Person Dungeon Crawler.cmd launcher. Play Regional Art Test.cmd starts a selected region with campaign saving disabled. The main project is J:\\First Person Dungeon Crawler Game. Current integration evidence is in Saved/RegionalIdentity; the accepted pre-integration executable and cooked containers are backed up in Saved/RegionalIdentity/Backup/AcceptedPackage, and source/config backups are under the same Backup directory. Restore the package as one consistent set if rollback is needed; do not mix executable and cooked container revisions.

Production scripts are mirrored in scripts/. Do not rerun setup_identity_pass.py over completed work. Import/material generation remains guarded to the isolated Unreal test project. integrate_identity_art.py is a one-time guarded promotion with hash checks and backups. finalize_identity_integration.py updates this handoff only after packaged, preservation and visual checks pass. Image generation was available but not used; the delivered work is procedural texture production and Blender geometry.
'''
(R / 'HANDOFF.md').write_text(handoff, encoding='utf8')
for name in ['HANDOFF.md', 'manifest.json']:
    shutil.copy2(R / name, D / name)
shutil.copy2(E / 'validation_summary.json', D / 'validation_summary.json')
for path in (G / 'Tools').glob('*identity*.py'):
    shutil.copy2(path, R / 'scripts' / path.name)
shutil.copy2(G / 'Tools/Review-RegionalIdentity.ps1', R / 'scripts/Review-RegionalIdentity.ps1')
status = G / 'BUILD_STATUS.md'
intro = 'Regional architecture and surface identity, **16 September 2026**: the new walls, floors, ceilings and regional architectural forms are integrated in the normal Windows build. Hell includes three distinct floor treatments; the approved sewer pack is retained. All 12 regression suites, the packaged 18-floor review (6,882 boundary sweeps), and the shared key/lever/crate review passed with zero failures. Original assets, artwork, saves, campaign data and unrelated code were preserved; display preferences were restored. Review: `J:\\Lonemoore_Regional_Identity\\REVIEW.html`. Handoff: `ArtReview/RegionalIdentity/HANDOFF.md`.\n\n'
if not status.read_text().startswith('Regional architecture and surface identity,'):
    status.write_text(intro + status.read_text(), encoding='utf8')
print('MAIN_GAME_INTEGRATION_COMPLETE', json.dumps({k: v for k, v in report.items() if k not in ['preservation', 'visual_review']}))
