from pathlib import Path
import html, shutil
r=Path(__file__).resolve().parents[1];out=r/'ArtReview/ExpansionPrototype';photos=out/'Review';photos.mkdir(exist_ok=True)
pack=r/'Builds/ExpansionPrototype/Windows/DungeonCrawler/Saved/ExpansionPrototype'
for p in pack.glob('*.png'):shutil.copy2(p,photos/p.name)
figures=[]
for p in sorted((r/'Saved/ExpansionPrototype').glob('layout_*.txt')):
    rows=p.read_text(encoding='utf-8-sig').splitlines();w=len(rows[0]);h=len(rows);cells=[]
    colors={'~':'#285d66','E':'#be715a','B':'#e05252','K':'#eacb68','S':'#cde0d8','>':'#dfc585','?':'#8a76ba','G':'#839aae','L':'#839aae'}
    for y,row in enumerate(rows):
        for x,t in enumerate(row):
            if t!='#':cells.append(f'<rect x="{x}" y="{y}" width="1" height="1" fill="{colors.get(t,"#7f897b")}"/>')
    svg=f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" role="img" aria-label="Actual generated floor {p.stem}"><rect width="100%" height="100%" fill="#111815"/>'+''.join(cells)+'</svg>'
    (out/(p.stem+'.svg')).write_text(svg)
    figures.append(f'<figure><img src="{p.stem}.svg" alt="Generated floor {p.stem}"><figcaption>Seed {p.stem.split("_")[1]}</figcaption></figure>')
gallery=[]
for name,caption in [('03_reservoir_bridge','Reservoir bridges and existing sewer materials'),('05_ruins','Ruined architecture and varied chamber sizes'),('02_fog_at_entrance','Fog of war at the entrance'),('06_rat_king','The original Rat King guards the descent')]:
    gallery.append(f'<figure><a href="Review/{name}.png"><img src="Review/{name}.png" alt="{caption}"></a><figcaption>{caption}</figcaption></figure>')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lonemoore — Expanded Cistern review</title>
<style>body{margin:0;background:#111612;color:#ddd8c6;font:17px/1.55 system-ui}main{max-width:1120px;margin:auto;padding:44px 28px}h1{font:44px Georgia;color:#dfc78f;margin:.2em 0}h2{font:28px Georgia;color:#dfc78f}p{max-width:850px}a{color:#dfc78f}small{color:#adb4a8}.stats{display:flex;gap:18px;flex-wrap:wrap;margin:28px 0}.stats div{padding:18px 25px;background:#202820;border:1px solid #59614b}.stats b{display:block;font-size:28px;color:#e8d5a4}.grid{display:grid;grid-template-columns:1fr 1fr;gap:20px}figure{margin:0}img{width:100%;display:block;border:1px solid #59614b}figcaption{padding:10px 0;color:#bbc3b4}details{border-top:1px solid #59614b;margin:30px 0;padding-top:18px}summary{cursor:pointer;color:#dfc78f}.maps{display:grid;grid-template-columns:repeat(3,1fr);gap:18px}.note{padding:20px;border-left:3px solid #ae9460;background:#202820}@media(max-width:700px){.grid,.maps{grid-template-columns:1fr}h1{font-size:34px}}</style>
<main><small>LONEMOORE · FIRST PLAYABLE EXPANSION · SEPTEMBER 17, 2026</small><h1>The expanded Cistern</h1>
<p>Explore a much larger, randomized Rat King's Cistern with the existing artwork and enemies. This is the single-floor review before expanding the other regions.</p>
<p class="note"><strong>Atmosphere revision:</strong> the miniature arches are removed, ceilings are lower, chambers have more furnishings, and a warm player light follows you. <a href="AtmosphereRevision/REVIEW.html">See the before/after comparison</a>. Choose Continue in the launcher to use your existing save.</p>
<div class="stats"><div><b>13.4×</b>walkable area in reviewed seed</div><div><b>10×</b>regular enemies</div><div><b>100</b>distinct tested layouts</div><div><b>2 seals</b>key gate, then boss gate</div></div>
<p class="note">Start <strong>Play Expanded Dungeon Prototype.cmd</strong> in the project folder. Choose a random layout, continue your prototype save, or choose the reviewed seed. The prototype has its own Windows package and save slots.</p>
<h2>Packaged game views</h2><div class="grid">'''+''.join(gallery)+'''</div>
<h2>What to try</h2><p>Find the rusted key and the optional lever vault. Cross the reservoir, investigate the secret treasury, defeat the Rat King, and open the final stair seal. Visit town for supplies and return through an activated waypoint.</p>
<p>W/S move · A/D turn · E interact · M map · F5/F9 save/load · T town. The map has zoom, pan, markers, and <strong>Fit floor</strong>. Bright terrain is visible now, dim terrain is remembered, and black space is unexplored.</p>
<p>The review party is a level-three warrior, mage and cleric with ordinary stats. The full-clear automated route walked 1,025 steps, won all 21 fights, visited both optional vaults, and descended using the key and boss seals.</p>
<details><summary>Compare three actual generated floor plans — contains spoilers</summary><p>Different room shapes and connections. Blue: reservoir gaps. Red: enemies. Gold: key and stairs. White: start. These diagrams expose the whole layout for review; the playable map starts concealed.</p><div class="maps">'''+''.join(figures)+'''</div></details>
<h2>Review boundary</h2><p>Please assess room variety, scale, enemy spacing, bridge readability and the pace of exploration. This first prototype retains grid movement while allowing larger architecture, variable wall spans and ceiling heights. Fire pits, more cavern treatments and the campaign-wide rollout follow this review.</p>
<p><a href="README.md">Play instructions and implementation notes</a> · <a href="VALIDATION.md">Validation and preservation evidence</a></p>
<small>Game captures use a save-disabled camera fixture. The fully explored comparison is intentionally a review view. Automated playability checks are evidence of functional routes, not a substitute for your playtest.</small></main></html>'''
(out/'REVIEW.html').write_text(page,encoding='utf-8')
print(out/'REVIEW.html')
