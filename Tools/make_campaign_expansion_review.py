"""Build a compact review from actual packaged campaign captures."""
from pathlib import Path
import json,html,shutil
r=Path(__file__).resolve().parents[1];out=r/'ArtReview/CampaignExpansion';images=out/'Review';images.mkdir(exist_ok=True)
source=r/'Builds/Windows/DungeonCrawler/Saved/CampaignExpansion'
floors=json.loads((r/'Content/Game/Data/campaign.json').read_text())['floors']
cards=[]
for i,f in enumerate(floors):
    views=[]
    for suffix,label in [('Stairs','Gothic staircase'),('Chamber','Regional chamber'),('Bridge','Bridge and surrounding space'),('StairsSide','Stair detail')]:
        name=f'Floor_{i+1:02d}_{suffix}.png';p=source/name
        if not p.exists():raise RuntimeError(f'Missing packaged capture: {p}')
        shutil.copy2(p,images/name)
        views.append({'file':'Review/'+name,'label':label})
    cards.append({'name':f['name'],'region':f['region'],'views':views})
data=json.dumps(cards).replace('</',r'<\/')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lonemoore — Expanded campaign</title>
<style>body{margin:0;background:#121511;color:#e2decc;font:17px/1.6 system-ui}main{max-width:1200px;margin:auto;padding:38px 24px 65px}h1,h2{font-family:Georgia,serif;color:#e7ce96}h1{font-size:42px;line-height:1.2}a{color:#e7ce96}.note{background:#232b20;padding:18px 24px;border-left:3px solid #b7a070}.stats{display:flex;gap:14px;flex-wrap:wrap}.stats div{padding:12px 22px;background:#232b20}.stats b{font-size:26px;display:block}select,button{font:inherit;padding:9px 15px;background:#293326;color:#e2decc;border:1px solid #7d8567;border-radius:4px}button{cursor:pointer}button[aria-pressed=true]{background:#d9c38e;color:#141710}select{max-width:100%;width:100%}.views{display:flex;flex-wrap:wrap;gap:8px;margin:16px 0}img{width:100%;display:block;border:1px solid #58644b}figure{margin:0}figcaption,small{color:#b9c0af}figcaption{padding:9px 0}.instructions{max-width:1000px}h2{margin-top:34px}li{margin:7px 0}</style>
<main><small>LONEMOORE · CAMPAIGN EXPANSION · SEPTEMBER 17, 2026</small><h1>The expanded descent</h1>
<p class="instructions">The approved dungeon improvements now extend through all eighteen floors. Each region keeps its artwork and enemies, with larger randomized plans, discovered-map memory, warmer player light and more structural detail. Gothic stairs use each region’s stone and metal finishes.</p>
<div class="stats"><div><b>18</b>campaign floors</div><div><b>10×+</b>walkable area</div><div><b>10×</b>regular enemies</div><div><b>18</b>gothic stair variants</div></div>
<p class="note">Launch <strong>Play Lonemoore.cmd</strong> in the project folder. Existing explored floors retain their shape and progress; unvisited floors expand when first entered. New journeys use expanded floors throughout. Existing locations also receive the new stairs and player light.</p>
<h2>Actual packaged game views</h2><label for="floor">Choose a floor</label><select id="floor"></select><div class="views" id="views"></div><figure><a id="full"><img id="shot" alt=""></a><figcaption id="caption"></figcaption></figure>
<h2>What changed</h2><ul class="instructions"><li>The lowest stair tread no longer shares a surface with the dungeon floor. Every staircase has carved stonework, raised treads, balustrades, pointed framing and a regional crest.</li><li>Lower ceilings, supports, regional wall and floor geometry, and room furnishings carry the approved atmosphere into the campaign.</li><li>Room shapes vary by region, including chapels, burial galleries, root caves, fortifications and deeper caverns. Bridges cross water, chasms and fire-lit pits.</li><li>Keys, locked doors, guardian seals, companions, hunts and the final story encounter retain their roles. Clearing a floor’s regular encounters remains permanent.</li></ul>
<h2>Play and preservation</h2><p class="instructions">The normal campaign and prototype keep their separate saves. Continue an existing journey to retain progress, or start a new journey to explore the expanded Cathedral immediately. The first prototype launcher remains available with the revised stair assets.</p>
<p><a href="VALIDATION.md">Validation and preservation details</a> · <a href="HANDOFF.md">Implementation and save behavior</a></p><small>Images use a save-disabled review fixture. Some views intentionally open the seals to inspect stairs. Functional progression and campaign playability are checked separately; these images do not represent a manual campaign playthrough.</small>
</main><script>const floors=DATA;let view=0;const select=document.getElementById('floor');floors.forEach((f,i)=>{const o=document.createElement('option');o.value=i;o.textContent=(i+1)+'. '+f.name;select.append(o)});function render(){const f=floors[+select.value];const tabs=document.getElementById('views');tabs.replaceChildren();f.views.forEach((v,i)=>{const b=document.createElement('button');b.textContent=v.label;b.setAttribute('aria-pressed',i===view);b.onclick=()=>{view=i;render()};tabs.append(b)});const v=f.views[view];const img=document.getElementById('shot');img.src=v.file;img.alt=f.name+' — '+v.label;document.getElementById('full').href=v.file;document.getElementById('caption').textContent=f.name+' · '+v.label;}select.onchange=()=>{view=0;render()};render();</script></html>'''.replace('DATA',data)
(out/'REVIEW.html').write_text(page,encoding='utf-8')
print(out/'REVIEW.html')
