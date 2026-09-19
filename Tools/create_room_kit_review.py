"""Build a review from real native captures and explicit validation evidence.

Run only after RoomKitReview has produced screenshots. Missing evidence is shown
as pending; this tool never converts an absent check into a pass.
"""
from __future__ import annotations

import argparse
import html
import json
import re
import shutil
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps

ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--capture-dir", type=Path, default=ROOT / "Builds/RoomKitCandidate/Windows/DungeonCrawler/Saved/RoomKit")
parser.add_argument("--evidence-dir", type=Path, default=ROOT / "Saved/RoomKitPass")
parser.add_argument("--output-dir", type=Path, default=ROOT / "ArtReview/RoomKit")
parser.add_argument("--installed", action="store_true", help="Use only after validated candidate has actually been promoted.")
args = parser.parse_args()
out = args.output_dir.resolve()
if not out.is_relative_to(ROOT / "ArtReview"):
    raise SystemExit("Review output must remain inside this project's ArtReview folder.")
captures = sorted(args.capture_dir.glob("Floor_*_*.png"))
if not captures:
    raise SystemExit(f"No native RoomKitReview screenshots found in {args.capture_dir}")
out.mkdir(parents=True, exist_ok=True)
(out / "native").mkdir(exist_ok=True)
(out / "catalog").mkdir(exist_ok=True)


def read_json(path: Path):
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8-sig"))


def esc(value):
    return html.escape(str(value), quote=True)


def thumb(source: Path, destination: Path, size=(1100, 720)):
    with Image.open(source) as image:
        image = image.convert("RGB")
        image.thumbnail(size, Image.Resampling.LANCZOS)
        image.save(destination, quality=90)


manifest = read_json(ROOT / "Content/Game/Data/room_kit.json")
geometry = read_json(ROOT / "ArtSource/RoomKit/validation.json")
tests = read_json(args.evidence_dir / "Automation/index.json")
if (args.evidence_dir / "AutomationFinal/index.json").exists():
    tests = read_json(args.evidence_dir / "AutomationFinal/index.json")
preservation = read_json(args.evidence_dir / "preservation.json")
performance = read_json(args.evidence_dir / "performance_results.json")
materials = read_json(args.evidence_dir / "materials.json")
walkways = read_json(args.evidence_dir / "walkways.json")
campaign = read_json(ROOT / "Content/Game/Data/campaign.json")
floors = campaign.get("floors", []) if campaign else []
runtime_file = args.capture_dir / "runtime_results.txt"
runtime = runtime_file.read_text(encoding="utf-8-sig", errors="replace") if runtime_file.exists() else ""
runtime_summary = re.search(r"Floors=(\d+); physical sweeps=(\d+); captured_room_types=(\d+); failures=(\d+)", runtime)

checks = []
if tests:
    passed = tests.get("succeeded", 0) + tests.get("succeededWithWarnings", 0)
    failed = tests.get("failed", 0)
    pending = tests.get("notRun", 0) + tests.get("inProcess", 0)
    checks.append(("Native automated tests", "PASS" if passed and not failed and not pending else "CHECK", f"{passed} passed, {failed} failed, {pending} unfinished."))
else:
    checks.append(("Native automated tests", "PENDING", "No final automation report is present."))
if runtime_summary:
    count, sweeps, kinds, failures = map(int, runtime_summary.groups())
    checks.append(("Automated native room and gate checks", "PASS" if failures == 0 else "FAIL", f"{count} floors; {sweeps:,} physical boundary sweeps; {failures} failures."))
else:
    checks.append(("Automated native room and gate checks", "PENDING", "Native screenshots are available; a completed runtime report is not yet present."))
if geometry:
    meshes = geometry.get("meshes", [])
    bad_faces = sum(m.get("duplicateFaces", 0) + m.get("degenerateFaces", 0) for m in meshes)
    checks.append(("Blender geometry", "PASS" if geometry.get("roomCount") == 25 and bad_faces == 0 else "CHECK", f"{geometry.get('roomCount', 0)} room designs; {len(meshes)} meshes including the portal cap; {bad_faces} duplicate or degenerate faces."))
else:
    checks.append(("Blender geometry", "PENDING", "No geometry validation report is present."))
if materials:
    checks.append(("Regional materials", str(materials.get("status", "CHECK")), f"{len(materials.get('materials', []))} regional surface materials recorded."))
else:
    checks.append(("Regional materials", "PENDING", "No material import report is present."))
if walkways:
    checks.append(("Authored walking paths", "PASS" if walkways.get("passed") else "FAIL", f"{walkways.get('roomCount', 0)} rooms; {walkways.get('rayCount', 0):,} sampled lane rays; {walkways.get('failureCount', 0)} intersections. Supplements the physical boundary sweeps."))
else:
    checks.append(("Authored walking paths", "PENDING", "No source-geometry lane report is present."))
if preservation:
    detail = preservation.get("summary") or f"{preservation.get('unchanged', 0)} of {preservation.get('protected', 0)} protected files unchanged."
    if "unchanged_files" in preservation:
        detail = f"{preservation['unchanged_files']} protected files unchanged."
    checks.append(("Player saves and original art", str(preservation.get("status", "CHECK")), str(detail)))
else:
    checks.append(("Player saves and original art", "PENDING", "Final preservation comparison has not been recorded."))

labels = {
    "Entrance": "Dungeon entrance",
    "Circular": "Circular ossuary",
    "Bridge": "Bridge over the chasm",
    "RoundedTurn": "Rounded turn",
    "BossApproach": "Ominous guardian approach",
    "LockedArena": "Guardian arena",
    "GuardianSeal": "Sealed descent gate",
    "SealedStairs": "Themed staircase",
}


def capture_info(path):
    match = re.fullmatch(r"Floor_(\d+)_(.+)", path.stem)
    number, scene = int(match[1]), match[2]
    floor_name = floors[number - 1].get("name", f"Floor {number}") if 0 < number <= len(floors) else f"Floor {number}"
    return number, scene, floor_name


# Native game views lead the review; the catalog below is explicitly Blender art.
# Prefer a crypt-themed floor for the concept comparison, then show each region's entrance.
preferred = next((i + 1 for i, floor in enumerate(floors) if "crypt" in str(floor.get("region", "")).lower() and "catacomb" not in str(floor.get("region", "")).lower() and floor.get("boss")), 1)
available_numbers = {capture_info(path)[0] for path in captures}
if preferred not in available_numbers:
    preferred = min(available_numbers)
selected = [path for path in captures if capture_info(path)[0] == preferred]
selected += [path for path in captures if capture_info(path)[1] == "Entrance" and capture_info(path)[0] != preferred]
native_cards = []
for path in selected:
    number, scene, floor_name = capture_info(path)
    shutil.copy2(path, out / "native" / path.name)
    preview_name = path.stem + ".jpg"
    thumb(path, out / "native" / preview_name)
    caption = f"{floor_name} · {labels.get(scene, scene)}"
    native_cards.append(f'<figure><a href="native/{esc(path.name)}"><img src="native/{esc(preview_name)}" alt="{esc(caption)}" loading="lazy"></a><figcaption>{esc(caption)}<span>Native Unreal capture from the automated review</span></figcaption></figure>')

font_path = Path("C:/Windows/Fonts/segoeui.ttf")
font = ImageFont.truetype(str(font_path), 18) if font_path.exists() else ImageFont.load_default()
sheet_columns, cell_w, cell_h = 3, 480, 310
sheet = Image.new("RGB", (sheet_columns * cell_w, ((len(selected) + sheet_columns - 1) // sheet_columns) * cell_h), (19, 22, 23))
draw = ImageDraw.Draw(sheet)
for index, path in enumerate(selected):
    _, scene, floor_name = capture_info(path)
    x, y = index % sheet_columns * cell_w, index // sheet_columns * cell_h
    with Image.open(path) as original:
        preview = ImageOps.contain(original.convert("RGB"), (cell_w - 16, 258), Image.Resampling.LANCZOS)
    sheet.paste(preview, (x + (cell_w - preview.width) // 2, y + 8))
    label = f"{floor_name} / {labels.get(scene, scene)}"
    while draw.textlength(label, font=font) > cell_w - 20 and len(label) > 10:
        label = label[:-2]
    draw.text((x + 10, y + 274), label, fill=(226, 215, 193), font=font)
sheet.save(out / "CONTACT_SHEET.jpg", quality=91)

stats = {mesh["id"]: mesh for mesh in geometry.get("meshes", [])} if geometry else {}
catalog_cards = []
for room in manifest["templates"]:
    room_id = room["id"]
    render = ROOT / "ArtSource/RoomKit/Renders" / (room_id + ".png")
    if render.exists():
        thumb(render, out / "catalog" / (room_id + ".jpg"), (720, 560))
        picture = f'<img src="catalog/{esc(room_id)}.jpg" alt="Blender preview of {esc(room["name"])}" loading="lazy">'
    else:
        picture = '<div class="pending-art">Blender preview pending</div>'
    stats_text = f'{stats[room_id]["triangles"]:,} triangles' if room_id in stats else "Geometry report pending"
    source = ROOT / "ArtSource/RoomKit/Scenes" / ("SM_RK_" + room_id + ".blend")
    source_link = f'<a href="../../ArtSource/RoomKit/Scenes/SM_RK_{esc(room_id)}.blend">Editable Blender source</a>' if source.exists() else "Blender source pending"
    catalog_cards.append(f'<figure>{picture}<figcaption><strong>{esc(room["name"])}</strong><span>Blender asset preview · {stats_text}</span><span>{source_link}</span></figcaption></figure>')

performance_rows = []
if isinstance(performance, list):
    for scene in performance:
        if not all(key in scene for key in ("scene", "fps")):
            continue
        low = scene.get("p99_fps", "—")
        performance_rows.append(f'<tr><td>{esc(scene.get("label", "Candidate"))}</td><td>{esc(scene["scene"])}</td><td>{esc(scene["fps"])}</td><td>{esc(low)}</td></tr>')
performance_html = '<p>Performance measurements are pending. Screenshots and geometry counts do not establish an FPS result.</p>'
if performance_rows:
    performance_html = '<p>RTX 3060, 3440 × 1369 output, Epic quality with the existing automatic resolution setting and TSR history at 100%. Uncapped offscreen native captures after scene warm-up; 1,300 measured frames per view. “Slow-frame FPS” is 1000 divided by the 99th-percentile frame time; it is not a minimum FPS guarantee. These are stationary scene measurements, not a full gameplay route.</p><table><thead><tr><th>Build</th><th>Scene</th><th>Average FPS</th><th>Slow-frame FPS</th></tr></thead><tbody>' + "".join(performance_rows) + '</tbody></table>'

concept = Path("C:/Users/nicho/AppData/Local/Temp/codex-clipboard-fcc2ee2c-6026-42c4-9dbb-4716a69aad1e.png")
concept_html = ""
if concept.exists():
    shutil.copy2(concept, out / "CONCEPT_REFERENCE.png")
    concept_html = '<details><summary>Your concept reference</summary><figure class="reference"><img src="CONCEPT_REFERENCE.png" alt="User supplied Gothic crypt reference"><figcaption>Direction reference — concept artwork, not a game screenshot.</figcaption></figure></details>'

check_html = "".join(f'<tr><td>{esc(name)}</td><td class="{esc(status.lower())}">{esc(status)}</td><td>{esc(detail)}</td></tr>' for name, status, detail in checks)
status = "Installed game update" if args.installed else "Candidate build for review"
document = f'''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>Authored dungeon room kit</title>
<style>
:root{{color-scheme:dark}}*{{box-sizing:border-box}}body{{margin:0;background:#111516;color:#e8e2d5;font:17px/1.6 system-ui,sans-serif}}main{{max-width:1440px;margin:auto;padding:38px 28px}}h1,h2{{line-height:1.2;color:#dabb81}}h1{{font-size:42px;margin:8px 0 18px}}h2{{font-size:29px;margin-top:48px}}p{{max-width:980px}}a{{color:#dfc18a}}.kicker{{color:#acb9af;font-size:14px;letter-spacing:.08em;text-transform:uppercase}}.intro{{max-width:980px;font-size:21px}}.grid{{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(430px,100%),1fr));gap:22px}}.catalog{{grid-template-columns:repeat(auto-fit,minmax(min(310px,100%),1fr))}}figure{{margin:0;background:#1b2223;border:1px solid #394441;border-radius:8px;overflow:hidden}}figure img{{width:100%;display:block}}figcaption{{padding:12px 15px}}figcaption span{{display:block;font-size:13px;color:#b4bdb4}}.pending-art{{height:210px;display:grid;place-items:center;color:#a8b1aa;background:#222a2b}}table{{border-collapse:collapse;width:100%;font-size:15px}}th,td{{text-align:left;padding:13px 14px;border-bottom:1px solid #3a4442;vertical-align:top}}th{{color:#dabb81}}.pass{{color:#a7d0a0}}.fail{{color:#ef9a8c}}.pending,.check{{color:#e3c98f}}details{{margin-top:28px}}summary{{cursor:pointer;color:#dabb81}}.reference{{max-width:960px;margin-top:16px}}.note{{padding:16px 20px;background:#202727;border-left:3px solid #b79a67}}li{{margin:7px 0}}@media(max-width:640px){{main{{padding:24px 15px}}h1{{font-size:32px}}table{{font-size:13px}}td,th{{padding:9px 7px}}}}
</style><main>
<div class="kicker">{status}</div><h1>25 rooms. Randomized dungeon floors.</h1>
<p class="intro">Authored Blender rooms give the dungeon deliberate walls, vaults, turns and bridges. The existing game still connects the rooms into a different floor plan for each seed.</p>
<p>Try it with <strong>Play Authored Room Dungeon.cmd</strong> in the project folder. Choose the reviewed crypt floor or a new random layout. This playtest has separate saves. <a href="PLAYTEST.md">Playtest guide</a>.</p>
<ul><li>Matching doorways and consistent texture scale keep joins aligned.</li><li>Regional materials, enemy artwork, fog of war and the player torch remain part of the game.</li><li>Two connected Gothic approach halls lead to the guardian arena. A separate sealed chamber protects the stairs.</li><li>New floors use the room kit; explored floors and recovery locations keep their existing layout.</li></ul>
<p class="note">The images below are actual Unreal captures. The separate 25-room catalog shows Blender asset previews. Physical monitor tearing still requires an on-monitor playtest; static room geometry addresses wall seams and overlapping surfaces.</p>
<h2>Inside the dungeon</h2><p><a href="CONTACT_SHEET.jpg">Open the native screenshot contact sheet</a>. Select any image for the original full-resolution capture.</p><div class="grid">{''.join(native_cards)}</div>
{concept_html}
<h2>Validation</h2><table><thead><tr><th>Check</th><th>Result</th><th>Evidence</th></tr></thead><tbody>{check_html}</tbody></table>
<p><a href="VALIDATION.md">Full validation notes</a>. Separate collision sweeps test movement barriers and gates; they do not by themselves prove that every decorative surface is visually clear.</p>
<h2>Performance</h2>{performance_html}
<h2>The 25-room kit</h2><p>Editable Blender source with broad textured surfaces, architectural trim and defined connection points. These are asset previews, separate from the game lighting above.</p><div class="grid catalog">{''.join(catalog_cards)}</div>
</main></html>'''
(out / "REVIEW.html").write_text(document, encoding="utf-8")
notes = ["# Authored dungeon room kit validation", "", f"Release status: {status}.", "", f"Native screenshot source: `{args.capture_dir}`.", f"Selected native captures: {len(selected)} of {len(captures)} available.", ""]
for name, check_status, detail in checks:
    notes.append(f"- {name}: {check_status}. {detail}")
notes += ["", "## Scope and limits", "", "- The automated native review checks actual room transforms, material slots, torch, fog, stairs, physical boundary sweeps, gate state changes and reuse of static architecture. This is not a hands-on playtest.", "- Automated generation coverage includes deterministic layouts, exact tenfold encounter rosters, gate bypass rejection, malformed room rejection, real isolated save/load and legacy floor preservation.", "- Boundary sweeps use the same collision barriers as gameplay. Decorative mesh clearance also needs visual inspection.", "- Monitor tearing cannot be established from screenshots. Display synchronization and on-monitor playtesting are separate from wall geometry validation.", "- Blender previews are labeled separately from native Unreal captures.", "- Pending evidence remains pending; generating this review does not promote a build.", ""]
if runtime:
    shutil.copy2(runtime_file, out / "runtime_results.txt")
    notes += ["## Native runtime report", "", "See `runtime_results.txt` for individual checks and any failures.", ""]
for file in ("preservation.json", "performance_results.json", "materials.json"):
    if (args.evidence_dir / file).exists():
        shutil.copy2(args.evidence_dir / file, out / file)
(out / "VALIDATION.md").write_text("\n".join(notes), encoding="utf-8")
print(out / "REVIEW.html")
