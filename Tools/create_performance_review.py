"""Publish measured results and unmodified screenshots from the packaged A/B runs."""
import html
import json
import re
import shutil
from pathlib import Path

root = Path(__file__).resolve().parents[1]
evidence = root / 'Saved/PerformancePass'
out = root / 'ArtReview/PerformancePass'
out.mkdir(parents=True, exist_ok=True)
results = json.loads((evidence / 'performance_results.json').read_text())
cases = {'ExistingSave':'Your existing save', 'Explored':'Fully explored sewer floor',
         'Floor_00':'Cathedral chamber', 'Floor_02':'Sewer bridge', 'Floor_16':'Hellforge chamber'}
rows = []
for key, name in cases.items():
    pair = {x['label']:x for x in results if x['scene']==key}
    before, after = pair['Before'], pair['After']
    rows.append({'key':key, 'name':name, 'before':before, 'after':after,
                 'gain':round((after['fps']/before['fps']-1)*100)})
    for label in ('Before','After'):
        shutil.copy2(evidence / label / f'{key}.png', out / f'{label}_{key}.png')
preserved = json.loads((evidence / 'preservation.json').read_text())
assert not preserved['changed_protected_files']
assert all(x['campaign_matches'] and x['prototype_matches'] for x in preserved['runtime'])
tests = json.loads((evidence / 'Automation/index.json').read_text(encoding='utf-8-sig'))
assert tests['failed']==0 and tests['notRun']==0
campaign = (evidence / 'campaign_review.log').read_text(encoding='utf-8-sig')
match = re.search(r'CAMPAIGN_EXPANSION_REVIEW_COMPLETE floors=(\d+) sweeps=(\d+) failures=(\d+)', campaign)
assert match and match[1]=='18' and match[3]=='0'
prototype = (evidence / 'prototype_review.log').read_text(encoding='utf-8-sig')
assert 'EXPANSION_REVIEW_COMPLETE failures=0' in prototype
refresh = [float(x) for x in re.findall(r'DUNGEON_WORLD_REFRESH architecture=reused build_count=1 cpu_ms=([0-9.]+)', prototype)]
assert refresh
table = ''.join(f'<tr><td>{r["name"]}</td><td>{r["before"]["fps"]}</td><td class="good">{r["after"]["fps"]}</td><td>+{r["gain"]}%</td><td>{r["before"]["one_percent_low_fps"]} → {r["after"]["one_percent_low_fps"]}</td></tr>' for r in rows)
options = ''.join(f'<option value="{r["key"]}">{r["name"]}</option>' for r in rows)
page = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lonemoore · Performance pass</title><style>
:root{color-scheme:dark;font:17px/1.6 system-ui;background:#101315;color:#e9e4d7}body{max-width:1400px;margin:auto;padding:36px}h1{font:normal 48px Georgia;margin:0}h2{font:normal 29px Georgia;color:#ddc491;margin-top:40px}.eyebrow{letter-spacing:.2em;color:#a6bda9;font-size:13px}p{max-width:1000px;color:#bbc3c3}strong{color:#e9e4d7}a{color:#d8c69e}table{border-collapse:collapse;width:100%;background:#181d20}th,td{text-align:left;padding:14px;border-bottom:1px solid #313839}th{color:#a7b1b3;font-size:14px}.good{color:#a6dfb3;font-weight:700}.controls{display:flex;gap:16px;flex-wrap:wrap;align-items:center;margin:20px 0}button,select{background:#202a2b;color:#eee;border:1px solid #657674;border-radius:4px;padding:12px 18px;font:inherit;cursor:pointer}button.active{background:#425c4d;border-color:#a6dfb3}img{width:100%;height:auto;display:block;border:1px solid #394244}.caption{font-size:14px;color:#9ba9a9}.checks{display:flex;gap:18px;flex-wrap:wrap}.checks span{padding:14px 22px;background:#1c2b23;border:1px solid #354f3e}details{margin-top:30px;padding:20px;background:#181d20}summary{cursor:pointer}.tablewrap{overflow:auto}@media(max-width:650px){body{padding:18px}h1{font-size:34px}td,th{padding:9px}}</style>
<div class="eyebrow">LONEMOORE · 17 SEPTEMBER 2026</div><h1>More dungeon. Less overhead.</h1>
<p>The optimized build is installed in both the normal game and the expanded prototype. These are actual packaged captures on this computer’s <strong>RTX 3060 12 GB</strong> at <strong>3440 × 1369</strong>.</p>
<h2>Same saved scenes, measured again</h2><div class="tablewrap"><table><thead><tr><th>Scene</th><th>Before FPS</th><th>After FPS</th><th>Gain</th><th>1% low FPS · before → after</th></tr></thead><tbody>TABLE</tbody></table></div>
<p class="caption">Five fixed-camera scenes, identical saves and settings, uncapped frame rate, native output resolution, Epic lighting. Each result uses 1,300 steady frames after loading; the startup and screenshot frames are excluded. “1% low” is the reciprocal of the 99th-percentile frame time. These are scene benchmarks, not a guarantee for every combat or movement sequence.</p>
<h2>Check the artwork</h2><div class="controls"><select id="scene" aria-label="Scene">OPTIONS</select><button id="before">Before</button><button id="after" class="active">Optimized</button><a id="full" target="_blank">Open full size</a></div><img id="view" alt="Actual packaged dungeon screenshot"><p class="caption" id="caption"></p>
<h2>What changed</h2><p>Floors stay in memory when you pick up items or open gates. The radar only draws the part inside its frame. Static scenery is batched for culling, detailed meshes use Nanite, distant lights fade out, and unchanged enemy sprites and doors stop doing redundant work. Regional assets load asynchronously, with construction and decoration spread across separate loading stages.</p>
<p>The original artwork, floor layouts, enemy counts, player torch and boss/key gates are retained. Epic anti-aliasing now uses a smaller history buffer at the same output resolution; this saves GPU work but can make very fine detail softer during movement. Cinematic retains the original history setting.</p>
<div class="checks"><span>19 automated suites passed</span><span>18 floors checked in game</span><span>SWEEPS physical boundary checks</span><span>124 protected files unchanged</span></div>
<details><summary>Evidence and technical notes</summary><p><a href="VALIDATION.md">Full validation record</a> · <a href="../../Saved/PerformancePass/performance_results.json">Raw results</a> · <a href="../../Saved/PerformancePass/Automation/index.html">Automated tests</a></p><p>Checks cover fog, themed stairs, regional materials, player lighting, live key/guardian gates, save continuation and architecture reuse. The previous package remains in Builds/PerformanceBaseline/Windows.</p><p>Technique references: <a href="https://dev.epicgames.com/documentation/unreal-engine/instanced-static-mesh-component-in-unreal-engine">Epic’s instancing guidance</a> · <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/temporal-super-resolution-in-unreal-engine">TSR quality and cost</a>.</p></details>
<script>let mode='After';const scene=document.querySelector('#scene'),view=document.querySelector('#view');function update(){let src=mode+'_'+scene.value+'.png';view.src=src;document.querySelector('#full').href=src;document.querySelector('#caption').textContent=scene.selectedOptions[0].text+' · '+(mode==='After'?'Optimized build':'Previous build')+' · 3440 × 1369';document.querySelector('#before').classList.toggle('active',mode==='Before');document.querySelector('#after').classList.toggle('active',mode==='After')}scene.onchange=update;document.querySelector('#before').onclick=()=>{mode='Before';update()};document.querySelector('#after').onclick=()=>{mode='After';update()};update();</script></html>'''
(out/'REVIEW.html').write_text(page.replace('TABLE',table).replace('OPTIONS',options).replace('SWEEPS',f'{int(match[2]):,}'),encoding='utf-8')
md = '''# Performance pass — 17 September 2026

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
'''
for r in rows:
    md += f'| {r["name"]} | {r["before"]["fps"]} | {r["after"]["fps"]} | +{r["gain"]}% | {r["before"]["one_percent_low_fps"]} / {r["after"]["one_percent_low_fps"]} |\n'
md += f'''
FPS is 1,000 / mean frame milliseconds. The reported 1% low is 1,000 / the
99th-percentile frame milliseconds, not the mean of the slowest 1% of frames.

## Implemented

- Immutable architecture survives pickups, combat refreshes and gate changes.
  In the final packaged interaction review, same-floor refreshes took {min(refresh):.1f}–{max(refresh):.1f} ms;
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

- {tests['succeeded'] + tests['succeededWithWarnings']} native suites completed; zero failed or skipped.
  Eight suites recorded an unrelated connectivity-probe timeout warning.
- Packaged 18-floor review: {int(match[2]):,} physical boundary sweeps, zero failures.
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
'''
(out/'VALIDATION.md').write_text(md,encoding='utf-8')
print('Published',out/'REVIEW.html')
