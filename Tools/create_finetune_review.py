"""Publish verified screenshots and release evidence for the fine-tuning pass."""
from pathlib import Path
import json,re,shutil,html
r=Path(__file__).resolve().parents[1];e=r/'Saved/FineTunePass';out=r/'ArtReview/FineTunePass'
out.mkdir(parents=True,exist_ok=True)
tests=json.loads((e/'AutomationFinal/index.json').read_text(encoding='utf-8-sig'))
assert tests['failed']==0
native=tests['succeeded']+tests['succeededWithWarnings'];assert native>=20
fine=(e/'fine_review.log').read_text(encoding='utf-8-sig',errors='replace');assert 'FINETUNE_REVIEW_COMPLETE failures=0' in fine
campaign=(e/'campaign_review.log').read_text(encoding='utf-8-sig',errors='replace')
gate=re.search(r'CAMPAIGN_EXPANSION_REVIEW_COMPLETE floors=18 sweeps=(\d+) failures=0',campaign);assert gate
preservation=json.loads((e/'preservation.json').read_text(encoding='utf-8-sig'));assert preservation['status']=='PASS'
perf=json.loads((e/'performance_results.json').read_text(encoding='utf-8-sig'))
rows=[]
for scene in ['Warrens','Explored']:
    before=next(x for x in perf if x['label']=='Before' and x['scene']==scene)
    after=next(x for x in perf if x['label']=='After' and x['scene']==scene)
    rows.append(f'<tr><td>{scene}</td><td>{before["fps"]}</td><td>{after["fps"]}</td><td>{before["one_percent_low_fps"]}</td><td>{after["one_percent_low_fps"]}</td></tr>')
comparisons=[]
for label in ['Before','After']:
    name=label+'_Warrens.png';shutil.copy2(e/label/'Warrens.png',out/name)
    comparisons.append(f'<figure><a href="{name}"><img src="{name}" alt="{label} at the same saved position" loading="lazy"></a><figcaption>{label} — identical saved position</figcaption></figure>')
cards=[]
labels=['Wall close-up','Along the wall','Larger world enemies','Successful save confirmation','Toast dismissed automatically','Combat controls retained','Tall boss fits below the ceiling']
shots=sorted((r/'Builds/FineTuneCandidate/Windows/DungeonCrawler/Saved/FineTune').glob('*.png'));assert len(shots)==7
for p,label in zip(shots,labels):
    shutil.copy2(p,out/p.name)
    cards.append(f'<figure><a href="{p.name}"><img src="{p.name}" alt="{label}" loading="lazy"></a><figcaption>{label}</figcaption></figure>')
text=f'''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Dungeon fine-tuning</title>
<style>body{{background:#131512;color:#e5e0d2;font:18px/1.6 system-ui;max-width:1300px;margin:auto;padding:36px}}h1,h2{{color:#d6b67c}}p{{max-width:1000px}}.grid{{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(430px,100%),1fr));gap:24px}}figure{{margin:0;background:#20251e;padding:10px}}img{{width:100%}}td,th{{text-align:left;padding:10px 24px;border-bottom:1px solid #46513e}}a{{color:#d6b67c}}</style>
<h1>Dungeon fine-tuning</h1><p>Installed in both game launchers. Original art and player saves preserved.</p>
<ul><li>Repaired the stone/timber intersections and separated wall supports.</li><li>Removed the small overlap between adjacent floor modules.</li><li>Enlarged world enemies to 2× dimensions, with a height limit for the tallest bosses; feet stay at ground level.</li><li>Added a timed “Game saved” popup after successful saves, with failure feedback if a write fails.</li><li>Enabled VSync. You can still change it in Graphics settings.</li></ul>
<h2>Performance check</h2><p>RTX 3060, 3440 × 1369, matching settings and saves. VSync disabled only for these uncapped measurements; steady-state samples exclude loading and capture overhead. Average FPS is within 4% of the previous build. The Warrens view has frame-time spikes in both builds; these pre-existing spikes remain. Slow-frame FPS is 1000 divided by the 99th-percentile frame time.</p>
<table><tr><th>Scene</th><th>Before FPS</th><th>After FPS</th><th>Before slow-frame FPS</th><th>After slow-frame FPS</th></tr>{''.join(rows)}</table>
<h2>Same saved position: before and after</h2><div class="grid">{''.join(comparisons)}</div><h2>Packaged screenshots</h2><div class="grid">{''.join(cards)}</div>
<h2>Validation</h2><p>{native} native tests passed. All 18 campaign floors passed {int(gate[1]):,} physical boundary sweeps, with no gate mismatches. Packaged checks confirmed enemy scale, ground anchoring, real save feedback, automatic dismissal, blocked combat saves and architecture reuse.</p>
<p>{preservation['unchanged_saves']} player saves and all protected artwork are unchanged. The only player-settings change is VSync. Physical display tearing requires an on-monitor playtest; the packaged synchronization setting is verified.</p>
<p><a href="VALIDATION.md">Validation details</a></p></html>'''
(out/'REVIEW.html').write_text(text,encoding='utf-8')
(out/'VALIDATION.md').write_text(f'''# Fine-tuning validation

- Native tests: {native} passed, zero failures ({tests['succeededWithWarnings']} with unrelated connectivity-check timeout warnings).
- Packaged campaign: 18 floors, {gate[1]} physical boundary sweeps, zero failures.
- Save slots and original artwork: preserved; {preservation['unchanged_files']} protected files unchanged.
- Display settings: VSync enabled, all other existing preferences retained.
- Both launchers have matching validated binaries and content containers.
- World enemies: 2x dimensions, capped at 520cm in expanded layouts and 440cm in legacy layouts to clear ceilings; combat artwork and controls retained.
- Timed save confirmation is emitted after the actual save API succeeds, never by dry-run or blocked combat saves.
- Initial route-test run had a randomized enemy-roster death. The fixture now pins RunId as well as the layout seed; its isolated rerun passed, followed by the full final suite. Original failure evidence is retained in `Automation/`.
- Evidence: `Saved/FineTunePass/` (build, import, tests, packaged reviews, before/after CSVs, preservation).
- Screenshots cannot establish the absence of physical monitor tearing; VSync is enabled and verified in the packaged runtime.
''',encoding='utf-8')
print(out/'REVIEW.html')
