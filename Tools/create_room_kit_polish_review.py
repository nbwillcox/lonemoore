"""Assemble the polish review from real captures and recorded evidence.

Missing or incomplete evidence remains pending. This tool does not launch Unreal,
modify game assets, or promote a candidate. Run with the bundled Pillow Python.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import html
import hashlib
import json
from pathlib import Path
import re
import shutil

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--capture-dir', type=Path, default=ROOT / 'Builds/RoomKitPolishCandidate/Windows/DungeonCrawler/Saved/RoomKit')
parser.add_argument('--evidence-dir', type=Path, default=ROOT / 'Saved/RoomKitPolish')
parser.add_argument('--output-dir', type=Path, default=ROOT / 'ArtReview/RoomKitPolish')
args = parser.parse_args()
OUT = args.output_dir.resolve()
if not OUT.is_relative_to(ROOT / 'ArtReview'):
    raise SystemExit('Output must remain inside the project ArtReview folder.')
for directory in [OUT, OUT / 'native', OUT / 'comparison', OUT / 'source', OUT / 'evidence']:
    directory.mkdir(parents=True, exist_ok=True)
issues = []
evidence_links = []
published_images = []


def esc(value):
    return html.escape(str(value), quote=True)


def read_json(path):
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding='utf-8-sig'))
    except (json.JSONDecodeError, OSError) as error:
        issues.append(f'{path.name}: incomplete or unreadable ({type(error).__name__}).')
        return None


def read_text(path):
    return path.read_text(encoding='utf-8-sig', errors='replace') if path.exists() else ''


def evidence(path, name=None):
    if not path.exists():
        return ''
    target = OUT / 'evidence' / (name or path.name)
    shutil.copy2(path, target)
    href = 'evidence/' + target.name
    if href not in [item[1] for item in evidence_links]:
        evidence_links.append((target.name, href))
    return href


def publish_image(path, folder, stem=None, size=(1400, 850)):
    """Copy a genuine full-resolution image and create a display thumbnail."""
    stem = stem or path.stem
    try:
        with Image.open(path) as original:
            image = original.convert('RGB')
            image.thumbnail(size, Image.Resampling.LANCZOS)
            target = OUT / folder / (stem + '.jpg')
            image.save(target, quality=91)
        full = OUT / folder / (stem + ('_original' if path.suffix.lower() == '.jpg' else '') + path.suffix.lower())
        shutil.copy2(path, full)
        published_images.append(dict(source=str(path), thumbnail=str(target.relative_to(OUT)), original=str(full.relative_to(OUT))))
        return str(target.relative_to(OUT)).replace('\\', '/'), str(full.relative_to(OUT)).replace('\\', '/')
    except (OSError, ValueError):
        issues.append(f'{path.name}: image is not yet complete.')
        return None


def picture(path, folder, caption, tag='Native Unreal capture', stem=None):
    result = publish_image(path, folder, stem)
    if not result:
        return pending(caption + ' — capture pending')
    thumbnail, full = result
    return (f'<figure><a href="{esc(full)}"><img src="{esc(thumbnail)}" alt="{esc(caption)}" loading="lazy"></a>'
            f'<figcaption>{esc(caption)}<span>{esc(tag)}</span></figcaption></figure>')


def pending(text):
    return f'<div class="pending-card">{esc(text)}</div>'


checks = []


def check(label, status, detail, path=None, name=None):
    href = evidence(path, name) if path else ''
    checks.append(dict(label=label, status=status, detail=detail, evidence=href))


campaign = read_json(ROOT / 'Content/Game/Data/campaign.json') or {}
floors = campaign.get('floors', [])
regions = {}
for floor in floors:
    regions.setdefault(floor['regionIndex'], floor['region'])
fallback_regions = ['Last Dawn Cathedral', 'Old City Sewers', 'Forgotten Catacombs', 'Goblin Warrens', 'Ancient Crypts', 'Buried Fortress', 'The Deep', 'Infernal Ruins', 'Hell']
for index, name in enumerate(fallback_regions):
    regions.setdefault(index, name)

props_manifest = read_json(ROOT / 'ArtSource/RegionalDressing/manifest.json') or {}
props = props_manifest.get('props', [])
source_qa_path = ROOT / 'ArtSource/RegionalDressing/validation.json'
source_qa = read_json(source_qa_path)
if source_qa and source_qa.get('status') == 'PASS' and source_qa.get('props') == 24 and source_qa.get('failures') == 0:
    check('Blender prop library', 'PASS', f"24 props across nine themes; {source_qa.get('triangles', 0):,} triangles total. Source assets only.", source_qa_path, 'prop_source_validation.json')
else:
    check('Blender prop library', 'PENDING', 'Complete source validation has not been recorded.', source_qa_path, 'prop_source_validation.json')

import_path = ROOT / 'Saved/RegionalDressingPass/import_report.json'
imported = read_json(import_path)
source_hashes = {record['id']: next((file['sha256'] for file in record.get('files', []) if file['path'].lower().endswith('.fbx')), None) for record in (source_qa or {}).get('records', [])}
current_import_hashes = bool(imported and len(imported.get('meshes', [])) == 24 and all(source_hashes.get(mesh['id']) == mesh.get('sourceFbxSha256') and source_hashes.get(mesh['id']) for mesh in imported['meshes']))
if imported and imported.get('status') == 'PASS' and imported.get('failures') == 0 and len(imported.get('meshes', [])) == 24 and len(imported.get('materials', [])) == 45 and current_import_hashes:
    check('Props imported into Unreal', 'PASS', '24 meshes and 45 regional material instances recorded.', import_path, 'prop_import.json')
elif imported and imported.get('status') == 'FAILED':
    check('Props imported into Unreal', 'FAIL', str(imported.get('error', 'Import reported a failure.')), import_path, 'prop_import.json')
else:
    check('Props imported into Unreal', 'PENDING', 'A complete import report matching the current source hashes is not yet present.', import_path, 'prop_import.json')

tests_path = args.evidence_dir / 'AutomationFinal/index.json'
tests = read_json(tests_path)
if tests:
    passed = tests.get('succeeded', 0) + tests.get('succeededWithWarnings', 0)
    failed = tests.get('failed', 0)
    unfinished = tests.get('notRun', 0) + tests.get('inProcess', 0)
    status = 'FAIL' if failed else 'PASS' if passed >= 25 and not unfinished else 'PENDING'
    check('Final automated game tests', status, f'{passed} passed, {failed} failed, {unfinished} unfinished.', tests_path, 'automation_final.json')
else:
    check('Final automated game tests', 'PENDING', 'The final automation report is not present.')

enemy_path = args.evidence_dir / 'enemy_distribution_results.txt'
enemy_text = read_text(enemy_path)
enemy_rows = re.findall(r'^floor=(\d+) seeds=(\d+) encounter_sites=(\d+)\.\.(\d+) latest_first_room=(\d+) (PASS|FAIL)\s*$', enemy_text, re.M)
enemy_cases = re.search(r'cases=(\d+);', enemy_text)
enemy_complete = (len(enemy_rows) == 18 and {int(row[0]) for row in enemy_rows} == set(range(18)) and
                  all(row[5] == 'PASS' and int(row[1]) == 24 and int(row[4]) <= 2 for row in enemy_rows) and
                  enemy_cases is not None and int(enemy_cases[1]) == 432)
if enemy_complete:
    check('Encounter distribution', 'PASS', '432 seeded floors checked; fights spread across at least 70% of exploration rooms, with an early encounter within two room exits.', enemy_path)
else:
    check('Encounter distribution', 'FAIL' if re.search(r'\bFAIL\b', enemy_text) else 'PENDING', 'The complete 18-floor distribution report is not yet available.', enemy_path)
diagnosis = read_json(args.evidence_dir / 'enemy_diagnosis.json') or {}
old_sites = [f.get('previousModularEncounters') for f in diagnosis.get('floors', []) if isinstance(f.get('previousModularEncounters'), int)]
new_min = min(int(row[2]) for row in enemy_rows) if enemy_complete else None
new_max = max(int(row[3]) for row in enemy_rows) if enemy_complete else None
enemy_html = '<p>The denser encounter layout is awaiting its completed validation report.</p>'
if enemy_complete:
    enemy_html = f'<p><strong>{new_min}–{new_max} encounters per floor</strong>, spread through the rooms. The first fight arrives within two room exits. Existing enemy artwork and roster combinations are reused.</p>'
    if old_sites:
        scale = max(max(old_sites), new_max)
        enemy_html += (f'<div class="bars"><div><span>Previous room layout</span><div class="track"><b style="width:{max(old_sites)/scale*100:.1f}%"></b></div><strong>{min(old_sites)}–{max(old_sites)}</strong></div>'
                       f'<div><span>Updated distribution</span><div class="track new"><b style="width:{new_max/scale*100:.1f}%"></b></div><strong>{new_min}–{new_max}</strong></div></div>')
    enemy_html += '<p class="small">Counts are encounter locations, not individual enemy sprites. Entrance and recovery spaces remain calm. Existing cleared rooms and saved progress require the separate migration checks.</p>'
    evidence(args.evidence_dir / 'enemy_diagnosis.json')
    evidence(args.evidence_dir / 'enemy_migration_results.txt')

coverage_path = args.evidence_dir / 'floor_coverage.json'
coverage = read_json(coverage_path)
if coverage:
    good = coverage.get('passed') is True and coverage.get('roomCount') == 25 and coverage.get('failures') == []
    check('Floor surface coverage', 'PASS' if good else 'FAIL', f"{coverage.get('roomCount', 0)} room meshes checked through export and reimport. This measures geometry coverage; it is separate from the rendered comparison.", coverage_path)
else:
    check('Floor surface coverage', 'PENDING', 'The floor mesh audit is not yet available.')

placement_path = args.evidence_dir / 'placement_simulation.json'
placement = read_json(placement_path)
placement_current = bool(placement and placement.get('inputs') and all((ROOT / item['path']).exists() and hashlib.sha256((ROOT / item['path']).read_bytes()).hexdigest() == item['sha256'] for item in placement['inputs'].values()))
if placement and placement_current:
    good = placement.get('status') == 'PASS' and placement.get('failures') == []
    check('Prop placement bounds simulation', 'PASS' if good else 'FAIL', f"{placement.get('configurations', 0):,} static configurations; {placement.get('acceptedPlacements', 0):,} accepted placements; {placement.get('overlappingNewPropAabbs', 0)} new-prop box overlaps. This is a source-data simulation, separate from native checks.", placement_path)
    evidence(args.evidence_dir / 'placement_simulation.md')
    evidence(args.evidence_dir / 'audit_dressing_placement.py')
else:
    check('Prop placement bounds simulation', 'PENDING', 'A bounds simulation matching the current source files is not yet recorded.', placement_path)

capture_files = sorted(args.capture_dir.glob('Floor_*_*.png'))
prop_captures = {}
ordinary_captures = []
for path in capture_files:
    prop_match = re.fullmatch(r'Floor_(\d+)_Prop_SM_RD_(.+)', path.stem)
    if prop_match:
        prop_captures.setdefault(prop_match[2], path)
    else:
        ordinary_captures.append(path)
runtime_path = args.capture_dir / 'runtime_results.txt'
runtime = read_text(runtime_path)
summary = re.search(r'Floors=(\d+); physical sweeps=(\d+); captured_room_types=(\d+); failures=(\d+)', runtime)
if summary:
    count, sweeps, room_types, failures = map(int, summary.groups())
    status = 'FAIL' if failures else 'PASS' if count == 18 else 'PENDING'
    check('Packaged room and gate checks', status, f'{count}/18 floors; {sweeps:,} physical boundary sweeps; {failures} failures.', runtime_path)
else:
    check('Packaged room and gate checks', 'PENDING', 'The completed 18-floor candidate report is not yet present.')

# Import/source reports do not establish placement. A dedicated native log must
# contain a completed dressing run; screenshots alone never become a check pass.
dressing_log = None
dressing_summary = None
for path in sorted(args.evidence_dir.glob('*.log'), key=lambda p: p.stat().st_mtime, reverse=True):
    if 'dressing' not in path.name.lower():
        continue
    matches = list(re.finditer(r'ROOM_KIT_DRESSING_REVIEW_COMPLETE floors=(\d+) failures=(\d+)', read_text(path)[-150000:]))
    if matches:
        dressing_log, dressing_summary = path, matches[-1]
        break
if dressing_summary:
    count, failures = map(int, dressing_summary.groups())
    check('Automated prop placement review', 'FAIL' if failures else 'PASS' if count == 18 else 'PENDING', f'{count}/18 floors completed; {failures} failures. Automated camera review, not a hands-on playtest.', dressing_log, 'native_dressing_review.log')
else:
    check('Automated prop placement review', 'PENDING', f'{len(prop_captures)} distinct prop captures found; the completed native review log is still pending.')

before_dir = args.evidence_dir / 'FloorDiagnosis/Nanite'
after_dir = args.evidence_dir / 'FloorDiagnosis/Retessellated'
before_files = sorted(before_dir.glob('Floor_*_FloorCheck_*.png'))
after_files = {p.name: p for p in after_dir.glob('Floor_*_FloorCheck_*.png')}
# Final packaged views supersede the earlier editor diagnostic at the same angle.
# If only some final views exist, each remaining fallback stays explicitly labeled.
packaged_floor_files = {p.name: p for p in args.capture_dir.glob('Floor_*_FloorCheck_*.png')}
after_files.update(packaged_floor_files)
pairs = [p for p in before_files if p.name in after_files]
packaged_comparison_count = sum(before.name in packaged_floor_files for before in pairs[:4])
floor_comparisons = []
# Lead with the paired angle, never substitute a different scene as an "after".
for before in pairs[:4]:
    match = re.search(r'FloorCheck_(.+)_(\d+)$', before.stem)
    label = match[1].replace('_', ' ').capitalize() + ' · view ' + str(int(match[2]) + 1) if match else 'Matching room view'
    after_tag = 'Packaged candidate · native floor diagnostic' if before.name in packaged_floor_files else 'Editor diagnostic · fallback until packaged view is captured'
    floor_comparisons.append('<div class="comparison-pair">' + picture(before, 'comparison', 'Before · ' + label, 'Earlier editor · native floor diagnostic', 'before_' + before.stem) + picture(after_files[before.name], 'comparison', 'After · ' + label, after_tag, 'after_' + before.stem) + '</div>')
if not pairs:
    first = before_files[0] if before_files else None
    floor_comparisons.append('<div class="comparison-pair">' + (picture(first, 'comparison', 'Before · floor surface', 'Earlier editor · native floor diagnostic', 'before_' + first.stem) if first else pending('Before capture pending')) + pending('Matching updated floor capture pending') + '</div>')

group_descriptions = [
    'Votive candles, a fallen bell and a scripture stand.',
    'A leaking pipe, slime-crusted drain and wet refuse.',
    'Loose remains and shelves filled with bones.',
    'Scavenged barricades, spear caches and pale fungi.',
    'Sarcophagi, vigil lanterns and crimson standards.',
    'Shield racks, weapons, chains and shackles.',
    'Horned drake skulls, ribcages and mineral outcrops.',
    'Binding stones and spiked offering braziers.',
    'Lava basins, glowing fissures and charred trophies.',
]
group_html = []
for region_index in range(9):
    group_props = [p for p in props if p.get('regionIndex') == region_index]
    cards = []
    for prop in group_props:
        path = prop_captures.get(prop['id'])
        if path:
            cards.append(picture(path, 'native', prop['name'], 'Native Unreal prop view · automated camera'))
    pictures = '<div class="prop-grid">' + ''.join(cards) + '</div>' if cards else pending('Native prop views are pending for this region.')
    group_html.append(f'<article class="region"><h3>{esc(regions[region_index])}</h3><p>{esc(group_descriptions[region_index])}</p><p class="small">{len(cards)} of {len(group_props)} prop types have native captures.</p>{pictures}</article>')

scene_labels = dict(Entrance='Dungeon entrance', Circular='Circular chamber', Bridge='Bridge over the void', RoundedTurn='Rounded passage', BossApproach='Approach to the guardian', LockedArena='Guardian arena', GuardianSeal='Sealed descent', SealedStairs='Gothic staircase')
journey = []
for scene in ['Entrance', 'Circular', 'Bridge', 'BossApproach', 'GuardianSeal', 'SealedStairs']:
    choices = [p for p in ordinary_captures if p.stem.endswith('_' + scene)]
    if choices:
        selected = choices[0]
        floor_no = int(re.match(r'Floor_(\d+)', selected.stem)[1])
        floor_name = floors[floor_no-1]['name'] if 0 < floor_no <= len(floors) else f'Floor {floor_no}'
        journey.append(picture(selected, 'native', scene_labels[scene], floor_name + ' · automated native capture'))
journey_html = '<div class="native-grid">' + ''.join(journey) + '</div>' if journey else pending('Final candidate room, bridge and guardian captures are pending.')

perf_path = args.evidence_dir / 'performance_results.json'
performance = read_json(perf_path)
perf_entries = performance if isinstance(performance, list) else next((performance[k] for k in ['scenes', 'results', 'measurements'] if isinstance(performance, dict) and isinstance(performance.get(k), list)), [])
perf_rows = []
for item in perf_entries:
    if not isinstance(item, dict):
        continue
    fps = item.get('fps', item.get('avg_fps', item.get('averageFps')))
    if not isinstance(fps, (int, float)) or fps <= 0:
        continue
    slow = item.get('p99_fps', item.get('slowFrameFps'))
    perf_rows.append(f'<tr><td>{esc(item.get("label", "Candidate"))}</td><td>{esc(item.get("scene", "Recorded view"))}</td><td>{fps:.1f}</td><td>{float(slow):.1f}</td></tr>' if isinstance(slow, (int, float)) else f'<tr><td>{esc(item.get("label", "Candidate"))}</td><td>{esc(item.get("scene", "Recorded view"))}</td><td>{fps:.1f}</td><td>—</td></tr>')
if perf_rows:
    performance_html = '<p>RTX 3060, 3440 × 1369 output, Epic quality and the existing automatic render-resolution setting. Measured stationary scenes after warm-up, with the frame cap and VSync disabled only for measurement. These figures do not represent every gameplay route. Slow-frame FPS is derived from the 99th-percentile frame time.</p><div class="table-wrap"><table><thead><tr><th>Build</th><th>Scene</th><th>Average FPS</th><th>Slow-frame FPS</th></tr></thead><tbody>' + ''.join(perf_rows) + '</tbody></table></div>'
    evidence(perf_path)
else:
    performance_html = '<p class="pending-card">Final performance measurements are pending.</p>'

promotion_path = args.evidence_dir / 'promotion.json'
promotion = read_json(promotion_path)
promoted = bool(isinstance(promotion, dict) and str(promotion.get('Status', promotion.get('status', ''))).upper() == 'PASS' and
                'RoomKitPolishCandidate' in str(promotion.get('Candidate', promotion.get('candidate', ''))) and
                promotion.get('SavedDirectoriesExcluded') is True and promotion.get('PlayerSavesModified') is False and
                promotion.get('Targets') and all(t.get('SHA256Verified') is True for t in promotion['Targets']))
if promoted:
    check('Installed build and saved games', 'PASS', 'Promotion report records matching runtime hashes and excludes saved-game directories.', promotion_path)
else:
    check('Installed build and saved games', 'PENDING', 'No completed promotion and save-preservation report is recorded for this candidate.', promotion_path)
release_status = 'Installed update recorded' if promoted else 'Candidate review · installation not yet recorded'

blender = ROOT / 'ArtSource/RegionalDressing/CONTACT_SHEET.jpg'
if blender.exists():
    blender_html = picture(blender, 'source', 'The 24-prop source library', 'Blender studio previews — separate from the game lighting above')
else:
    blender_html = pending('Blender source contact sheet is not yet available.')

for audit in ['packaged_floor_visual_review.json', 'packaged_prop_visual_review.json', 'preservation.json', 'performance_comparison.json']:
    evidence(args.evidence_dir / audit)
check_rows = ''.join(f'<tr><td>{esc(c["label"])}</td><td class="{c["status"].lower()}">{c["status"]}</td><td>{esc(c["detail"])}' + (f' <a href="{esc(c["evidence"])}">Report</a>' if c['evidence'] else '') + '</td></tr>' for c in checks)
evidence_html = ''.join(f'<li><a href="{esc(href)}">{esc(name)}</a></li>' for name, href in evidence_links)
document = f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Dungeon polish review</title>
<style>
:root{{color-scheme:dark}}*{{box-sizing:border-box}}body{{margin:0;background:#111718;color:#e6e3d9;font:17px/1.6 system-ui,sans-serif}}main{{max-width:1500px;margin:auto;padding:38px 28px 60px}}h1,h2,h3{{color:#dbbe89;line-height:1.2}}h1{{font-size:43px;margin:8px 0 18px}}h2{{font-size:28px;margin:44px 0 15px}}h3{{font-size:22px;margin:0 0 8px}}p{{max-width:1050px}}a{{color:#e0c995}}.kicker,.small{{color:#aabcb6;font-size:14px}}.intro{{font-size:21px;max-width:1050px}}.summary{{display:flex;gap:12px;flex-wrap:wrap;margin:24px 0}}.summary span{{padding:10px 16px;border:1px solid #42544b;border-radius:6px;background:#1b2826}}figure{{margin:0;background:#1d2829;border:1px solid #3b4a45;border-radius:7px;overflow:hidden}}figure img{{display:block;width:100%;height:auto}}figcaption{{padding:10px 14px}}figcaption span{{display:block;font-size:13px;color:#adbbb4}}.comparison-pair,.native-grid{{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px;margin:18px 0}}.pending-card{{min-height:140px;display:flex;align-items:center;justify-content:center;text-align:center;padding:28px;color:#c9bc9c;border:1px dashed #536057;border-radius:7px;background:#1a2424}}.region{{margin:26px 0;padding-top:22px;border-top:1px solid #33433e}}.region p{{margin-top:8px}}.prop-grid{{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(360px,100%),1fr));gap:18px}}.bars{{max-width:960px;padding:14px 18px;background:#1b2625;border-radius:7px}}.bars>div{{display:grid;grid-template-columns:210px 1fr 90px;gap:16px;align-items:center;margin:13px 0}}.track{{background:#34413c;height:22px;border-radius:3px;overflow:hidden}}.track b{{display:block;height:100%;background:#79776a}}.track.new b{{background:#b7a16b}}.table-wrap{{overflow:auto}}table{{width:100%;border-collapse:collapse;font-size:15px}}th,td{{padding:12px 14px;text-align:left;vertical-align:top;border-bottom:1px solid #394940}}th{{color:#d6bd8b}}.pass{{color:#a8d49d}}.fail{{color:#f3a192}}.pending{{color:#deca92}}details{{margin-top:28px;padding:16px 20px;background:#1b2524;border-radius:7px}}summary{{cursor:pointer;color:#dabb81}}details figure{{max-width:1000px;margin-top:18px}}.note{{padding:14px 18px;border-left:3px solid #ab9262;background:#1d2927}}@media(max-width:720px){{main{{padding:25px 15px}}h1{{font-size:32px}}.comparison-pair,.native-grid{{grid-template-columns:1fr}}.bars>div{{grid-template-columns:1fr 60px;gap:8px}}.bars span{{grid-column:1/-1}}table{{min-width:600px}}}}
</style></head><body><main><div class="kicker">{esc(release_status)}</div><h1>More dungeon. More to discover.</h1>
<p class="intro">A closer look at the floor repair, fuller encounters and themed scenery. Native game captures lead this review; Blender source previews are kept separately below.</p>
<div class="summary"><span>Floor before / after</span><span>{f'{new_min}–{new_max} encounters per floor' if enemy_complete else 'Encounter checks pending'}</span><span>{len(props)} new props · nine themes</span></div>
<h2>Floor surfaces</h2><p>Matching camera angles show the earlier floor and the updated surface. Select either image to inspect the original capture.</p>{''.join(floor_comparisons)}
<p class="small">This comparison shows rendered floor geometry. It cannot establish whether a monitor tears during movement.</p>
<h2>Stronger encounter distribution</h2>{enemy_html}
<h2>Nine regional identities</h2><p>Slime and discarded pipes in the sewers, drake remains in the Deep, and fire and lava in Hell. The game reuses its established regional artwork and enemy sprites.</p>{''.join(group_html)}
<h2>Inside the updated dungeon</h2>{journey_html}
<h2>Performance</h2>{performance_html}
<details><summary>Recorded checks and current status</summary><div class="table-wrap"><table><thead><tr><th>Check</th><th>Result</th><th>Evidence</th></tr></thead><tbody>{check_rows}</tbody></table></div><p class="small">These are automated checks. A screenshot is not a hands-on playtest, and a source mesh audit is not proof of its native appearance. Missing final evidence stays pending.</p><ul>{evidence_html}</ul></details>
<details><summary>Blender source previews · 24 editable props</summary><p>Studio renders of the source assets. Their lighting differs from the native screenshots above.</p>{blender_html}</details>
<p class="small">Review generated {datetime.now(timezone.utc).strftime('%Y-%m-%d %H:%M UTC')}. <a href="VALIDATION.md">Validation notes</a>.</p>
</main></body></html>'''
(OUT / 'REVIEW.html').write_text(document, encoding='utf-8')
notes = ['# Dungeon polish review', '', f'Status: {release_status}.', '', f'Native screenshot source: {args.capture_dir}', f'Paired floor comparisons: {len(pairs)}; displayed packaged after-views: {packaged_comparison_count} of {min(4,len(pairs))}.', f'Distinct prop capture types: {len(prop_captures)} of 24.', '']
notes += [f'- {c["label"]}: {c["status"]}. {c["detail"]}' for c in checks]
notes += ['', 'Floor comparisons match the earlier Nanite diagnostic filenames. Final candidate captures are preferred as after-views; Retessellated editor captures are a clearly labeled fallback. Other native screenshots come only from the specified candidate capture directory. Blender renders are a separate section.', '', 'Source/import checks do not establish in-game placement or visual quality. Automated captures are not a hands-on playtest. Static images cannot verify temporal tearing. Performance values are shown only when recorded.', '', 'Generating this page does not install or promote a build.']
if issues:
    notes += ['', 'Incomplete inputs:', *['- '+issue for issue in issues]]
(OUT / 'VALIDATION.md').write_text('\n'.join(notes) + '\n', encoding='utf-8')
(OUT / 'review_manifest.json').write_text(json.dumps(dict(generatedUtc=datetime.now(timezone.utc).isoformat(), promoted=promoted, captureDirectory=str(args.capture_dir), comparisons=len(pairs), displayedPackagedFloorComparisons=packaged_comparison_count, nativePropTypes=len(prop_captures), checks=checks, images=published_images, issues=issues), indent=2))
print(OUT / 'REVIEW.html')
print(f'Floor pairs={len(pairs)}; native prop types={len(prop_captures)}; installed evidence={promoted}; pending checks={sum(c["status"] == "PENDING" for c in checks)}')
