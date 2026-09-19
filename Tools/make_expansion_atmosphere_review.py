"""Publish the packaged atmosphere revision and matched camera comparisons."""
from pathlib import Path
import shutil

root = Path(__file__).resolve().parents[1]
out = root / "ArtReview/ExpansionPrototype/AtmosphereRevision"
photos = out / "Review"
photos.mkdir(parents=True, exist_ok=True)
packaged = root / "Builds/ExpansionPrototype/Windows/DungeonCrawler/Saved/ExpansionPrototype"
for source in packaged.glob("*.png"):
    shutil.copy2(source, photos / source.name)
for name in ["03_reservoir_bridge", "05_ruins"]:
    shutil.copy2(root / f"Saved/ExpansionAtmosphere/Before/{name}.png", photos / f"before_{name}.png")

def figure(name, caption):
    return f'<figure><a href="Review/{name}.png"><img src="Review/{name}.png" alt="{caption}" loading="lazy"></a><figcaption>{caption}</figcaption></figure>'

room_pair = figure("before_05_ruins", "Before: freestanding mini arch and dark, empty chamber") + figure("05_ruins", "Revised: full-height supports, ceiling ribs, shelving and pipework")
bridge_pair = figure("before_03_reservoir_bridge", "Before: tall, dark reservoir") + figure("03_reservoir_bridge", "Revised: lower roof, structural beams and clearer bridge lighting")
torch_pair = figure("10_torch_off_comparison", "Comparison only: player light disabled") + figure("11_torch_on_comparison", "Player light enabled: warm light on nearby surfaces")
resume = ""
if (photos / "12_existing_save_revised.png").exists():
    resume = '<h2>Your continued save</h2><p>The packaged review loaded the existing prototype autosave with saving disabled. Its stored floor plan was retained exactly.</p>' + figure("12_existing_save_revised", "Existing prototype save, with the revised atmosphere")

page = '''<!doctype html>
<html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lonemoore — Dungeon atmosphere revision</title>
<style>
body{margin:0;background:#141510;color:#ded8c7;font:17px/1.6 system-ui}main{max-width:1320px;margin:auto;padding:36px 24px 60px}h1,h2{font-family:Georgia,serif;color:#e6cfa0}h1{font-size:42px;line-height:1.15}h2{margin-top:40px;font-size:27px}p{max-width:950px}a{color:#e6cfa0}small,figcaption{color:#b6b8a7}.grid{display:grid;grid-template-columns:1fr 1fr;gap:20px}figure{margin:0}img{display:block;width:100%;border:1px solid #545846}figcaption{padding:8px 0 18px}.play{background:#242a20;border-left:3px solid #b8a270;padding:18px 24px}.features{display:flex;gap:18px;flex-wrap:wrap}.features span{background:#242a20;padding:10px 18px}@media(max-width:850px){.grid{grid-template-columns:1fr}h1{font-size:34px}}
</style><main>
<small>LONEMOORE · CISTERN PROTOTYPE · SEPTEMBER 17, 2026</small>
<h1>A closer, warmer dungeon</h1>
<p>The random miniature arches have been removed. Lower ceilings, full-height supports, masonry ribs and more storage and machinery give the chambers a stronger sense of structure. A warm, gently flickering player light makes nearby surfaces visible.</p>
<div class="features"><span>Lower ceilings</span><span>Player torch glow</span><span>More room furnishings</span><span>Original artwork retained</span></div>
<p class="play">Launch <strong>Play Expanded Dungeon Prototype.cmd</strong> in the project folder and choose <strong>1 — Continue</strong>. The changes apply to your existing prototype save. You do not need to start again.</p>
<h2>The same chamber, before and after</h2><div class="grid">''' + room_pair + '''</div>
<p>Original sewer surfaces and enemy artwork are retained. Enemy sprites in the dungeon dim with distance from the player light; combat portraits keep their existing presentation. Furnishings use the existing wood, stone and iron materials.</p>
<h2>The reservoir</h2><div class="grid">''' + bridge_pair + '''</div>
<p>Passages now have 5.6m ceilings, ordinary chambers 6–9.5m, and the large reservoir 14m. Room outlines, bridges, fog of war, enemy counts, locks and boss progression are unchanged.</p>
<h2>The player light</h2><div class="grid">''' + torch_pair + '''</div>
<p>The warm light and softer ambient fill follow your position and viewing direction. Distant spaces remain dark. These two views isolate the player light; wall and room lighting remain enabled in both.</p>
''' + resume + '''
<h2>Validation</h2><p>The revised game passed all 15 regression suites and 4,826 physical boundary checks. The packaged review checked the key gate, boss seal, player light and continued save. Source art, existing saves and display settings were checked for preservation.</p>
<p><a href="../VALIDATION.md">Validation details</a> · <a href="../README.md">Play instructions</a> · <a href="../REVIEW.html">Floor plans and original expansion overview</a></p>
<small>Before/after views use the same seed and camera positions. Screenshots come from the packaged game with save writes disabled. These checks establish functionality; the atmosphere and exploration pace remain for your playtest.</small>
</main></html>'''
(out / "REVIEW.html").write_text(page, encoding="utf-8")
print(out / "REVIEW.html")
