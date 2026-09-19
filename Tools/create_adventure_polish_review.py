"""Build a local review from completed native captures and recorded evidence.

Run with Pillow's Python after the native candidate review. This tool never
launches Unreal, edits source PNGs, changes game assets, or promotes a build.
Missing, incomplete, or unfamiliar reports remain explicitly pending.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import html
from io import BytesIO
import json
from pathlib import Path
import re

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SHOTS = [
    ('01_all_classes', 'Choose any of the seven classes', 'New journeys begin with the complete existing class roster.'),
    ('02_name_character', 'Name your character', 'Keyboard name entry, validation, and a named journey.'),
    ('03_multiple_saves', 'Keep several saved moments', 'New manual saves sit alongside that character’s autosave and quicksave.'),
    ('04_character_browser', 'Choose a character to load', 'Separate named characters appear in the load browser.'),
    ('05_character_slots', 'Choose the exact save', 'Each character has their own list of saved moments.'),
    ('06_load_confirmation', 'Confirm the chosen journey', 'The confirmation identifies the save about to be restored.'),
    ('07_darker_arrival_and_shrine', 'A darker arrival', 'Dimmer player lighting keeps the dungeon mood while retaining local visibility.'),
    ('08_enemy_ground_contact', 'Enemies meet the floor', 'Sprite placement accounts for transparent space below the artwork.'),
    ('09_post_boss_save_shrine', 'A checkpoint beyond the boss', 'The descent chamber includes a shrine behind the required key and guardian gates.'),
    ('10_checkpoint_saved', 'Save at the descent shrine', 'Activating the cleared checkpoint writes an autosave and a return point.'),
    ('11_waypoint_selection', 'Return from town', 'The cleared checkpoint becomes an available destination.'),
    ('12_smaller_floor_map', 'A more focused floor', 'New floors use 68 sections instead of 104, about 35% fewer, with the boss branch retained.'),
]


def esc(value):
    return html.escape(str(value), quote=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    candidate = ROOT / 'Builds/AdventurePolishCandidate/Windows/DungeonCrawler/Saved'
    parser.add_argument('--capture-dir', type=Path, default=candidate / 'AdventurePolish')
    parser.add_argument('--room-capture-dir', type=Path, default=candidate / 'RoomKit')
    parser.add_argument('--evidence-dir', type=Path, default=ROOT / 'Saved/AdventurePolish')
    parser.add_argument('--performance-json', type=Path)
    parser.add_argument('--output-dir', type=Path, default=ROOT / 'ArtReview/AdventurePolish')
    parser.add_argument('--without-before', action='store_true')
    args = parser.parse_args()
    out = args.output_dir.resolve()
    if not out.is_relative_to((ROOT / 'ArtReview').resolve()):
        raise SystemExit('Review output must remain inside this project’s ArtReview folder.')
    for name in ['', 'native', 'floors', 'comparison', 'evidence']:
        (out / name).mkdir(parents=True, exist_ok=True)
    issues, checks, images, evidence_files = [], [], [], []

    def read_json(path):
        try:
            return json.loads(path.read_text(encoding='utf-8-sig'))
        except FileNotFoundError:
            return None
        except (OSError, ValueError) as error:
            issues.append(f'{path.name}: unreadable or incomplete ({type(error).__name__}).')
            return None

    def read_text(path):
        try:
            raw = path.read_bytes()
            encoding = 'utf-16' if raw.startswith((b'\xff\xfe', b'\xfe\xff')) else 'utf-8-sig'
            return raw.decode(encoding, errors='replace')
        except FileNotFoundError:
            return ''
        except OSError:
            issues.append(f'{path.name}: could not read evidence.')
            return ''

    def evidence(path, name=None):
        if not path.is_file():
            return ''
        try:
            raw = path.read_bytes()
            target = out / 'evidence' / (name or path.name)
            target.write_bytes(raw)
            href = target.relative_to(out).as_posix()
            if not any(item['href'] == href for item in evidence_files):
                evidence_files.append(dict(href=href, source=str(path.resolve()), sha256=hashlib.sha256(raw).hexdigest()))
            return href
        except OSError:
            issues.append(f'{path.name}: evidence copy could not be completed.')
            return ''

    def check(label, status, detail, source=None, copy_name=None):
        link = evidence(source, copy_name) if source else ''
        checks.append(dict(label=label, status=status, detail=detail, evidence=link))

    def pending(title, detail='Capture pending.'):
        return f'<article class="missing"><strong>{esc(title)}</strong><p>{esc(detail)}</p><span class="badge pending">Pending</span></article>'

    def picture(path, folder, title, caption, tag='Current candidate · native Unreal capture', stem=None):
        if not path.is_file():
            return pending(title)
        stem = stem or path.stem
        try:
            raw = path.read_bytes()
            with Image.open(BytesIO(raw)) as source:
                source.load()
                width, height = source.size
                preview = source.convert('RGB')
                preview.thumbnail((1440, 900), Image.Resampling.LANCZOS)
                jpg = out / folder / (stem + '.jpg')
                preview.save(jpg, quality=91, optimize=True)
            full = out / folder / (stem + path.suffix.lower())
            full.write_bytes(raw)
            original_href, preview_href = full.relative_to(out).as_posix(), jpg.relative_to(out).as_posix()
            images.append(dict(source=str(path.resolve()), original=original_href, preview=preview_href,
                               sha256=hashlib.sha256(raw).hexdigest(), width=width, height=height, caption=caption, tag=tag))
            return (f'<figure><a href="{esc(original_href)}" target="_blank" rel="noopener">'
                    f'<img src="{esc(preview_href)}" alt="{esc(title)}" loading="lazy" width="{width}" height="{height}"></a>'
                    f'<figcaption><strong>{esc(title)}</strong><p>{esc(caption)}</p><small>{esc(tag)} · Click for original PNG</small></figcaption></figure>')
        except (OSError, ValueError) as error:
            issues.append(f'{path.name}: image not ready ({type(error).__name__}).')
            return pending(title, 'The image is missing or still being written.')

    tests_path = args.evidence_dir / 'AutomationFinal/index.json'
    tests = read_json(tests_path)
    tests_table = ''
    if isinstance(tests, dict):
        try:
            passed = int(tests.get('succeeded', 0))
            warned = int(tests.get('succeededWithWarnings', 0))
            failed = int(tests.get('failed', 0))
            unfinished = int(tests.get('notRun', 0)) + int(tests.get('inProcess', 0))
            test_rows = tests.get('tests', [])
            status = 'FAIL' if failed else 'PASS' if passed + warned > 0 and not unfinished else 'PENDING'
            check('Final automated tests', status,
                  f'{passed + warned} passed ({warned} with warnings), {failed} failed, {unfinished} unfinished. '
                  f'Report: {tests.get("reportCreatedOn", "date unavailable")}.', tests_path, 'automation_final.json')
            if isinstance(test_rows, list):
                rows = ''.join(f'<tr><td>{esc(row.get("fullTestPath", row.get("testDisplayName", "Unnamed")))}</td>'
                               f'<td>{esc(row.get("state", "Unknown"))}</td><td>{esc(row.get("warnings", 0))}</td></tr>'
                               for row in test_rows if isinstance(row, dict))
                tests_table = f'<details><summary>Individual automated tests</summary><div class="table-scroll"><table><thead><tr><th>Test</th><th>Recorded result</th><th>Warnings</th></tr></thead><tbody>{rows}</tbody></table></div></details>'
        except (ValueError, TypeError):
            check('Final automated tests', 'PENDING', 'The final report is incomplete or has an unfamiliar schema.', tests_path, 'automation_final.json')
    else:
        check('Final automated tests', 'PENDING', 'No readable final automation report is present.', tests_path, 'automation_final.json')

    native_path = args.capture_dir / 'native_results.txt'
    native_text = read_text(native_path)
    native_summary = re.search(r'AdventurePolish native review failures=(\d+)', native_text)
    native_passes = len(re.findall(r'^PASS ', native_text, re.M))
    native_fails = len(re.findall(r'^FAIL ', native_text, re.M))
    if native_summary:
        failures = max(int(native_summary[1]), native_fails)
        check('Packaged journey and save review', 'FAIL' if failures else 'PASS',
              f'{native_passes} recorded checks passed; {failures} failures. Automated native input and disk-save checks, not a hands-on playtest.', native_path)
    else:
        check('Packaged journey and save review', 'FAIL' if native_fails else 'PENDING',
              'The candidate’s completed native review is not yet recorded.', native_path)

    runtime_path = args.room_capture_dir / 'runtime_results.txt'
    runtime_text = read_text(runtime_path)
    runtime_summary = re.search(r'Floors=(\d+); physical sweeps=(\d+); captured_room_types=(\d+); failures=(\d+)', runtime_text)
    if runtime_summary:
        floor_count, sweeps, room_types, failures = map(int, runtime_summary.groups())
        check('Current 18-floor geometry and gates', 'FAIL' if failures else 'PASS' if floor_count == 18 else 'PENDING',
              f'{floor_count}/18 floors; {sweeps:,} physical sweeps; {room_types} room types captured; {failures} failures.', runtime_path, 'room_runtime_results.txt')
    else:
        check('Current 18-floor geometry and gates', 'PENDING', 'The current candidate’s complete room review is not present.', runtime_path, 'room_runtime_results.txt')

    audio_path = args.evidence_dir / 'combat_audio_validation.json'
    audio = read_json(audio_path)
    if isinstance(audio, dict):
        audio_ok = audio.get('status') == 'PASS' and audio.get('cueCount') == 27 and audio.get('distinctSkillCues') == 21 and audio.get('failures') == []
        check('Combat sound source checks', 'PASS' if audio_ok else 'FAIL',
              f'{audio.get("cueCount", "?")} cues; {audio.get("distinctSkillCues", "?")} distinct skill cues; '
              f'peak {audio.get("maxPeakDbFS", "?")} dBFS. Waveform and routing checks do not establish subjective listening quality.', audio_path)
    else:
        check('Combat sound source checks', 'PENDING', 'The sound waveform/routing report is not present.', audio_path)
    evidence(args.evidence_dir / 'combat_audio_import.json')
    evidence(args.evidence_dir / 'generation_and_loot_design.json')
    evidence(args.evidence_dir / 'ProfileGuide.md')
    for report in ['chest_catalog_quality.txt', 'chest_retry.txt', 'descent_checkpoints.txt',
                   'recruit_interactions.txt', 'campaign_routes.txt', 'enemy_distribution_results.txt']:
        evidence(args.evidence_dir / report)
    promotion_path = args.evidence_dir / 'promotion.json'
    promotion = read_json(promotion_path)
    if isinstance(promotion, dict) and promotion.get('status') == 'PASS':
        targets = promotion.get('targets', [])
        installed = len(targets) == 3 and all(row.get('status') == 'PASS' for row in targets)
        check('Playable build installation', 'PASS' if installed else 'PENDING',
              f'{len(targets)} launch locations recorded; runtime file hashes checked after copying. '
              'Start Play Lonemoore.cmd and choose New Game for a named journey.', promotion_path)
    else:
        check('Playable build installation', 'PENDING', 'The candidate has not yet been recorded as installed.', promotion_path)

    performance_path = args.performance_json or args.evidence_dir / 'performance_results.json'
    performance = read_json(performance_path)
    performance_rows = performance if isinstance(performance, list) else []
    if isinstance(performance, dict):
        for key in ('results', 'runs', 'measurements'):
            if isinstance(performance.get(key), list):
                performance_rows = performance[key]
                break
    measurements = [row for row in performance_rows if isinstance(row, dict) and isinstance(row.get('fps'), (int, float))
                    and row['fps'] > 0 and isinstance(row.get('samples'), (int, float)) and row['samples'] > 0]
    performance_html = pending('Performance measurements', 'The current candidate’s timing report is not available yet. Previous-build FPS is not substituted.')
    if measurements:
        check('Current performance measurements', 'MEASURED', f'{len(measurements)} recorded scene runs. Timings are measurements, not a universal performance guarantee.', performance_path)
        rows = ''.join(f'<tr><td>{esc(row.get("label", "Recorded build"))}</td><td>{esc(row.get("scene", "Unnamed scene"))}</td>'
                       f'<td>{row["fps"]:.1f}</td><td>{esc(row.get("p99_ms", "—"))}</td><td>{esc(row["samples"])}</td></tr>' for row in measurements)
        performance_html = f'<div class="table-scroll"><table><thead><tr><th>Build label</th><th>Scene</th><th>Mean FPS</th><th>99th-percentile frame (ms)</th><th>Samples</th></tr></thead><tbody>{rows}</tbody></table></div><p class="muted">Hardware, resolution, and capture conditions are those recorded in the linked evidence. Different dungeon layouts are not a matched-scene benchmark.</p>'
    else:
        check('Current performance measurements', 'PENDING', 'A readable current candidate timing report is not present.', performance_path)
    evidence(args.evidence_dir / 'performance_comparison.json')
    evidence(args.evidence_dir / 'performance_release_evidence.json')

    campaign = read_json(ROOT / 'Content/Game/Data/campaign.json') or {}
    floors = campaign.get('floors', []) if isinstance(campaign, dict) else []
    ui_cards = [picture(args.capture_dir / (stem + '.png'), 'native', title, caption) for stem, title, caption in SHOTS]
    ui_published = len([image for image in images if image['original'].startswith('native/')])
    check('Journey screenshots', 'CAPTURED' if ui_published == len(SHOTS) else 'PENDING',
          f'{ui_published}/{len(SHOTS)} real candidate screenshots available. Capture presence is not a visual approval.')

    room_cards = []
    shot_types = ['Entrance', 'Circular', 'BossApproach', 'RoundedTurn', 'Bridge', 'SealedStairs']
    for index in range(18):
        prefix = f'Floor_{index + 1:02d}_'
        preferred = shot_types[index % len(shot_types)]
        candidates = [args.room_capture_dir / (prefix + kind + '.png') for kind in [preferred] + [kind for kind in shot_types if kind != preferred]]
        path = next((candidate for candidate in candidates if candidate.is_file()), candidates[0])
        floor = floors[index] if index < len(floors) and isinstance(floors[index], dict) else {}
        name = floor.get('name', floor.get('region', 'Dungeon floor'))
        room_cards.append(picture(path, 'floors', f'Floor {index + 1:02d} · {name}',
                                  f'{path.stem.removeprefix(prefix)} view from the current 68-section generation.'))
    floor_published = len([image for image in images if image['original'].startswith('floors/')])
    check('Representative floor screenshots', 'CAPTURED' if floor_published == 18 else 'PENDING',
          f'{floor_published}/18 current candidate floors pictured. Geometry and gate results are reported separately.')

    comparison_html = ''
    before = ROOT / 'Builds/RoomKitPolishCandidate/Windows/DungeonCrawler/Saved/RoomKit/Floor_01_Entrance.png'
    if not args.without_before and before.is_file():
        comparison_html = '<section id="comparison"><h2>Previous and current atmosphere</h2><p class="muted">Different dungeon layouts and camera positions: the previous build used 104 sections; new floors use 68. These images provide context, not a matched-camera lighting test.</p><div class="grid">'
        comparison_html += picture(before, 'comparison', 'Previous build · 104 sections', 'Earlier RoomKitPolish entrance, retained only as a labeled reference.', 'Previous candidate · different layout and camera', 'previous_entrance')
        comparison_html += picture(args.capture_dir / '07_darker_arrival_and_shrine.png', 'comparison', 'Current build · 68 sections', 'New arrival and shrine capture; dimmer player lighting.', stem='current_arrival')
        comparison_html += '</div></section>'

    audio_link = '<a class="button" href="Audio/REVIEW.html">Listen to the 27 sound effects</a>' if (out / 'Audio/REVIEW.html').is_file() else pending('Audio listening library', 'Audio/REVIEW.html has not been generated in this review folder.')
    cards = ''.join(f'<article class="check"><span class="badge {esc(item["status"].lower())}">{esc(item["status"])}</span>'
                    f'<strong>{esc(item["label"])}</strong><p>{esc(item["detail"])}</p>'
                    + (f'<a href="{esc(item["evidence"])}">Recorded evidence</a>' if item['evidence'] else '') + '</article>' for item in checks)
    evidence_html = ''.join(f'<li><a href="{esc(item["href"])}">{esc(Path(item["href"]).name)}</a></li>' for item in evidence_files)
    warnings_html = '<ul>' + ''.join(f'<li>{esc(issue)}</li>' for issue in issues) + '</ul>' if issues else '<p>No evidence files were unreadable at generation time.</p>'
    generated = datetime.now(timezone.utc).isoformat(timespec='seconds')
    style = '''
:root{color-scheme:dark;--bg:#101214;--panel:#191d20;--line:#343b3d;--muted:#b4babe;--gold:#ddbd80}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:#f2eee5;font:16px/1.55 system-ui,Segoe UI,sans-serif}main{max-width:1320px;margin:auto;padding:30px 24px 70px}header{padding:26px 0 32px;border-bottom:1px solid var(--line)}h1{font-size:clamp(30px,5vw,52px);line-height:1.1;margin:10px 0 16px}h2{font-size:26px;margin:0 0 12px}h3{font-size:20px}p{margin:10px 0}a{color:var(--gold);text-underline-offset:3px}.eyebrow{color:var(--gold);font-size:13px;text-transform:uppercase;letter-spacing:.15em}.lead{max-width:900px;font-size:19px;color:#d5d8d9}.muted,small{color:var(--muted)}nav{display:flex;flex-wrap:wrap;gap:16px;margin-top:22px}section{margin:40px 0}.features,.grid,.checks{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:18px}.features article,.check,.missing{background:var(--panel);border:1px solid var(--line);border-radius:10px;padding:20px}.features h3{margin:0}.check strong{display:block;margin-top:12px}.check p{font-size:14px;color:var(--muted)}.badge{display:inline-block;border:1px solid currentColor;border-radius:30px;padding:3px 9px;font-size:11px;font-weight:700;letter-spacing:.05em}.pass{color:#a2dab0}.fail{color:#ffaaa4}.pending{color:#e2c17f}.measured,.captured{color:#a6d1ea}.missing{min-height:180px;display:flex;flex-direction:column;align-items:flex-start;justify-content:center;color:var(--muted)}figure{margin:0;border:1px solid var(--line);border-radius:10px;overflow:hidden;background:var(--panel)}figure>a{display:block;background:#090a0a}img{display:block;width:100%;height:auto}figcaption{padding:16px}figcaption strong{font-size:17px}figcaption p{font-size:14px;color:#c5cbd0}figcaption small{font-size:12px}.button{display:inline-block;border:1px solid var(--gold);padding:10px 16px;border-radius:6px;text-decoration:none;margin:8px 0}.table-scroll{overflow:auto}table{width:100%;border-collapse:collapse;font-size:14px}th,td{border-bottom:1px solid var(--line);padding:10px;text-align:left}th{color:var(--gold)}details{border:1px solid var(--line);border-radius:8px;padding:16px;margin-top:18px}summary{cursor:pointer;color:var(--gold)}footer{border-top:1px solid var(--line);padding-top:22px;color:var(--muted);font-size:13px}li{margin:6px 0}@media(max-width:720px){main{padding:16px 14px 45px}.features,.grid,.checks{grid-template-columns:1fr}nav{gap:12px}section{margin:30px 0}}
'''
    page = f'''<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lonemoore · Adventure polish review</title><style>{style}</style></head><body><main>
<header><div class="eyebrow">Lonemoore · Candidate review</div><h1>A more focused journey</h1><p class="lead">Named characters and separate saves, tighter dungeon floors, a checkpoint after each boss, better grounded enemies, and distinct combat sounds.</p><nav><a href="#changes">Changes</a><a href="#journeys">Game captures</a><a href="#audio">Listen</a><a href="#floors">All 18 floors</a><a href="#evidence">Checks</a></nav></header>
<section id="changes"><h2>What changed</h2><div class="features">
<article><h3>Your character, your saves</h3><p>Start as any of the seven classes, name your character, and keep multiple manual saves. Autosaves and quicksaves belong to the selected character.</p></article>
<article><h3>Less empty travel</h3><p>New dungeon floors use 68 sections instead of 104. The four-section boss approach, arena, and descent branch remains part of the route.</p></article>
<article><h3>A checkpoint after the boss</h3><p>The descent shrine saves your journey and provides a return destination from town after the required boss and key gates are cleared.</p></article>
<article><h3>More useful treasure</h3><p>Ordinary chests draw from the item catalog without filtering by the current party. Deeper floors favor better quality, and full bags leave the chest unclaimed.</p></article>
<article><h3>Stronger dungeon atmosphere</h3><p>Dimmer player lighting and enemy feet aligned to the floor refine the existing room art and themes.</p></article>
<article><h3>Recognizable combat feedback</h3><p>Metallic attacks, a punchy retro fireball, glassy ice, electric strikes, healing chimes, and darker magic replace shared generic spell feedback.</p></article>
</div><p class="muted">Start <strong>Play Lonemoore.cmd</strong> and choose <strong>New Game</strong> to begin a named journey. Installation and validation results appear below. <a href="HANDOFF.md">Release notes</a> · <a href="VALIDATION.md">Validation details</a></p></section>
<section id="journeys"><h2>See the changes in the game</h2><p class="muted">Actual packaged candidate captures. Open any image for the untouched full-resolution PNG.</p><div class="grid">{''.join(ui_cards)}</div></section>
<section id="audio"><h2>Hear each spell family</h2><p>27 original short effects include a unique cue for each of the 21 active skills. The listening library includes a short comparison reel and individual controls.</p>{audio_link}<p class="muted">Waveform checks cover clipping and routing. They are separate from subjective listening and the final in-game mix.</p></section>
{comparison_html}
<section id="floors"><h2>The current dungeon across all 18 floors</h2><p class="muted">One representative native view per floor from this candidate. Missing floor captures remain visible as pending.</p><div class="grid">{''.join(room_cards)}</div></section>
<section id="evidence"><h2>Recorded checks</h2><div class="checks">{cards}</div>{tests_table}</section>
<section id="performance"><h2>Performance</h2>{performance_html}</section>
<details><summary>Evidence files and provenance</summary><p>Reports are copied from the recorded sources without rewriting their results. Image hashes and original source locations are listed in <a href="review_manifest.json">the review manifest</a>.</p><ul>{evidence_html}</ul>{warnings_html}</details>
<footer>Generated {esc(generated)} from recorded game captures and validation. Source PNGs are unchanged; JPEGs are display previews. The installation record identifies the delivered build.</footer>
</main></body></html>'''
    (out / 'REVIEW.html').write_text(page, encoding='utf-8')
    manifest = dict(generatedUtc=generated, candidateCaptureDirectory=str(args.capture_dir.resolve()),
                    roomCaptureDirectory=str(args.room_capture_dir.resolve()), evidenceDirectory=str(args.evidence_dir.resolve()),
                    checks=checks, images=images, evidenceFiles=evidence_files, issues=issues,
                    limitations=['Automated results are not a hands-on playtest or visual approval.',
                                 'Audio waveform validation is not a subjective listening review.',
                                 'Previous/current images have different layouts and camera positions.'])
    (out / 'review_manifest.json').write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(f'ADVENTURE_REVIEW_READY native={ui_published}/12 floors={floor_published}/18 issues={len(issues)} output={out / "REVIEW.html"}')


if __name__ == '__main__':
    main()
