"""Publish the handoff only after isolated imports and packaged all-floor QA pass."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,shutil,html,hashlib
G=Path(__file__).resolve().parents[1];R=Path(r'J:\Lonemoore_Regional_Art');D=G/'ArtReview/RegionalArt';D.mkdir(parents=True,exist_ok=True)
for f in ['unreal_textures.json','unreal_meshes.json','refined_decals.json','export_validation.json']:
 assert json.loads((R/'reports'/f).read_text())['status']=='PASS',f
lighting=json.loads((R/'reports/unreal_lighting.json').read_text());assert lighting['status']=='CAPTURES_COMPLETE' and not lighting['errors']
preserved=json.loads((G/'Saved/RegionalArt/preservation.json').read_text());assert not preserved['mismatches'] and preserved['preferences_restored']
src=G/'Builds/Windows/DungeonCrawler/Saved/RegionalArt';results=(src/'runtime_results.txt').read_text(encoding='utf-8-sig');assert 'failures: 0' in results and 'Floors: 18;' in results
shared=(G/'Saved/RegionalArt/shared_packaged_review.log').read_text(errors='replace');assert 'SHARED_PROP_REVIEW_COMPLETE failures=0' in shared
assert hashlib.sha256((G/'Binaries/Win64/DungeonCrawler.exe').read_bytes()).digest()==hashlib.sha256((G/'Builds/Windows/DungeonCrawler/Binaries/Win64/DungeonCrawler.exe').read_bytes()).digest()
for log in ['fixture_editor_build.log','fixture_game_build.log']:assert 'Result: Succeeded' in (G/'Saved/RegionalArt'/log).read_text(errors='replace')
captures=D/'Packaged';captures.mkdir(exist_ok=True)
for p in src.glob('*'):
 if p.is_file():shutil.copy2(p,captures/p.name)
M=json.loads((R/'manifest.json').read_text());M['status']='INTEGRATED_PACKAGED_VALIDATED';M['integration']={'runtime_root':'/Game/RegionalArt','floors_covered':18,'new_regional_surface_packs':8,'approved_sewer_pack':'/Game/OldCitySewers preserved','decoration_collision':'None; existing logical and physical boundaries unchanged','world_keys':'6 identities, camera facing, 20cm hover over 2.4 seconds','world_key_scale':1.4,'approved_crate':'/Game/SharedInteractables preserved','parents':['M_RegionalWorld','M_RegionalUV','M_RegionalDecal'],'review_camera':'400cm cells, 155cm eye height, 76 degree horizontal FOV','production_tools':['Blender 5.2','Python with NumPy and Pillow','Unreal Engine 5.8.2'],'generation':'No image generation used; procedural maps and modeled geometry'}
refs={'Cathedral':['desecrated-temple','abandon-gate-house'],'Catacombs':['grand-catacombs','desecrated-temple'],'Warrens':['fungal-caverns','forgotten-mines'],'Crypts':['grand-catacombs','desecrated-temple'],'Fortress':['old-prison-cells','abandon-gate-house'],'Deep':['abyssal-descent','fungal-caverns','lost-underground-city'],'Infernal':['desecrated-temple','hell-gate'],'Hell':['hell-gate','abyssal-descent']}
for r in M['regions']:
 r['reference_images']=[str(G/'.art/.concepts'/(name+'.png')) for name in refs[r['code']]];r['reference_status']='Inspected originals; visual associations with existing lore, not new campaign canon';r['authority']=str(G/'Content/Game/Data/campaign.json')
for a in M['materials']:
 a['placement']={'wall':'Existing structural walls','secondary':'Localized lower wall repair panels','floor':'Existing walkable floor surfaces','vault':'Vaults and ceiling caps','trim':'Existing arches, trim, stairs and pillars','metal':'Existing doors, grates, fixtures and themed hardware','accent':'Regional prop detail and camp dressing','detail':'Niches, tombs and selected later-floor wall panels'}[a['role']];a['unreal_world_material']='/Game/RegionalArt/Materials/MI_'+a['id'];a['unreal_uv_material']='/Game/RegionalArt/Materials/MI_UV_'+a['id']
for a in M['props']:
 a['source']='sources/'+next(r['code'] for r in M['regions'] if r['index']==a['region'])+'.blend'
 a['placement']={'decoration':'Offset 65cm inward and 88cm sideways from selected existing wall boundaries; no collision','lever':'Existing lever tile; original switch interaction','world_key':'Existing uncollected key tile; original pickup and lock identity','showcase':'Isolated test corridor only; excluded from main runtime content'}.get(a['role'],a['role'])
for a in M['decals']:a['placement']='Selected wall surfaces only; localized transparent projection, no collision';a['unreal_material']='/Game/RegionalArt/Materials/MI_'+a['id']
(R/'manifest.json').write_text(json.dumps(M,indent=2));shutil.copy2(R/'manifest.json',D/'manifest.json');shutil.copy2(G/'DUNGEON_REGIONAL_ART_PLAN.md',R/'APPROVED_PLAN.md')
for name in ['setup_regional_art','bake_regional_textures','polish_regional_surfaces','polish_regional_decals','build_regional_models','import_regional_art','refresh_regional_story_decals','preview_regional_unreal','review_regional_exports','finalize_regional_art']:
 shutil.copy2(G/'Tools'/(name+'.py'),R/'scripts'/(name+'.py'))
def board(paths,out,cols=3,w=640):
 h=int(w*9/16)+40;im=Image.new('RGB',(cols*w,((len(paths)+cols-1)//cols)*h),(21,23,27));draw=ImageDraw.Draw(im);font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)
 for i,(label,path) in enumerate(paths):
  assert path.exists(),path;x=i%cols*w;y=i//cols*h;pic=Image.open(path).convert('RGB');pic.thumbnail((w,h-40));im.paste(pic,(x,y));draw.text((x+10,y+h-29),label,fill=(230,224,211),font=font)
 im.save(out,quality=95)
camp=json.loads((G/'Content/Game/Data/campaign.json').read_text());floors=camp['floors'];board([(f['name'],captures/f'Floor_{i+1:02d}_Passage.png') for i,f in enumerate(floors)],D/'All_18_Floors_Packaged.jpg')
board([(r['name'],R/'previews'/(r['code']+'_Corridor_Unreal.png')) for r in M['regions']],R/'previews/All_Regions_Unreal.jpg',2,800)
board([(r['name'],R/'previews'/(r['code']+'_Props_Blender.png')) for r in M['regions']],R/'previews/All_Themed_Models_Blender.jpg',2,800)
css='body{background:#141619;color:#e8e1d5;font:17px system-ui;max-width:1450px;margin:40px auto;padding:0 24px}h1{font-size:36px}h2{margin-top:50px}p{max-width:950px;line-height:1.6}a{color:#d7b979}img{width:100%;border-radius:8px}section{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:20px}figure{margin:0}figcaption{padding:12px 0}small{color:#b0b5bc}nav a{margin-right:20px}'
page=['<!doctype html><html><meta charset="utf-8"><title>Lonemoore regional art review</title><style>'+css+'</style><h1>Lonemoore — regional art review</h1><p>Eight new regional packs across sixteen floors, alongside the approved Old City Sewers. Includes 64 materials, 32 decals, 24 decorative props, eight themed levers and six floating world-key designs. Enemies remain 2D.</p><nav><a href="#game">Packaged game</a><a href="#isolated">Isolated Unreal</a><a href="#models">Blender models</a><a href="HANDOFF.md">Handoff</a></nav><h2 id="game">Actual packaged game — all 18 floors</h2><p>These captures use the existing game camera, lighting, interface and dungeon layouts. Click an image for full size.</p><section>']
for i,f in enumerate(floors):
 path=(captures/f'Floor_{i+1:02d}_Passage.png').as_uri();links=' · '.join(f'<a href="{(captures/f"Floor_{i+1:02d}_{kind}.png").as_uri()}">{kind}</a>' for kind in ['Entrance','Key','Lever','Map','Combat']);page.append(f'<figure><a href="{path}"><img loading="lazy" src="{path}"></a><figcaption>{html.escape(f["name"])}<br><small>{links}</small></figcaption></figure>')
page.append('</section><h2 id="isolated">Isolated Unreal material and lighting review</h2><p>Dedicated test corridors at 155 cm eye height and 76° FOV. These are material showcases, separate from the game layouts above.</p><section>')
for r in M['regions']:
 for kind in ['Corridor','Neutral']:
  path='previews/'+r['code']+'_'+kind+'_Unreal.png';page.append(f'<figure><a href="{path}"><img loading="lazy" src="{path}"></a><figcaption>{r["name"]} — {kind.lower()} lighting, Unreal</figcaption></figure>')
page.append('</section><h2 id="models">Editable Blender models and material repeats</h2><section>')
for r in M['regions']:
 for kind in ['Props_Blender','Materials_Blender3x3']:
  path='previews/'+r['code']+'_'+kind+'.png';page.append(f'<figure><a href="{path}"><img loading="lazy" src="{path}"></a><figcaption>{r["name"]} — {kind.replace("_"," ")}</figcaption></figure>')
page.append('</section></html>');(R/'REVIEW.html').write_text(''.join(page),encoding='utf-8')
readme=f'''# Lonemoore regional art handoff

Approved and integrated on 2026-09-16. Open REVIEW.html for labeled Blender, isolated Unreal and actual packaged-game comparisons. Run the game's `Play Regional Art Test.cmd` to select any of the nine existing regions with campaign saving disabled. The normal launcher uses the updated packaged build.

## Delivered

- Eight new surface packs: Cathedral, Catacombs, Warrens, Crypts, Fortress, Deep, Infernal and Hell. Approved Old City Sewers remains unchanged.
- 64 distinct native 2048×2048 material sets, 32 RGBA decals, 24 decorative meshes, eight regional lever variants, six world-key meshes and eight showcase corridor meshes. 268 texture-map PNGs. No 4K assets, upscaling or image generation.
- Eight editable .blend sources in `sources`, 46 FBX exports in `props`, maps in `textures`, and repeatable production/import scripts in `scripts`.
- Three reusable Unreal parents, 128 world/UV surface instances and 32 decal instances. Main content excludes showcase meshes/maps.
- The approved shared crate remains active. Rusted hardware serves the cathedral/sewers; lower regions have burial, military, mineral, ritual and horned forge details. World-key style follows the existing key identity. Astra and the narrative Hell Key are untouched.

## Scale and maps

Blender metres, Unreal centimetres. World surfaces repeat over 200 cm (1024 texels/metre), with projection preventing stretching on existing architecture. UV props use the same nominal repeat; curved bevels use local projection and real geometry for their silhouette. The existing game grid is 400 cm, eye height 155 cm and horizontal FOV 76°. Mesh dimensions and triangle counts are listed individually in manifest.json. Keys use 1.4× world presentation scale and a 20 cm, 2.4 second vertical hover.

Base colour is sRGB pigment without lighting or cast shadows. DirectX normals, roughness, metallic and emission are linear. Unreal normal compression is TC_NORMALMAP with green flip disabled; Blender inverts green before its Normal Map node. Metallic and emission maps exist only where used. Decals use straight RGBA with fully transparent borders. World and UV material parents expose tint, wetness, roughness, normal detail and emission; world tiling uses TextureSizeCm, UV tiling uses Tiling. Wetness defaults to zero; roughness is clamped at 0.30 to avoid mirrorlike surfaces.

## Reproduction

Workspace root is currently J:/Lonemoore_Regional_Art; scripts declare it explicitly. Change that root when relocating the pack. Existing Blender texture paths point into this workspace.

1. Run setup_regional_art.py only to initialize a new isolated workspace and save baseline evidence. It reads the authoritative campaign and reference files.
2. Run bake_regional_textures.py, then polish_regional_surfaces.py, then polish_regional_decals.py using Python with NumPy/Pillow. The polish steps are required to reproduce final surfaces and story marks.
3. Run Blender 5.2 in background with build_regional_models.py. Optional region-code arguments after `--` select a subset. Sources and exports remain editable and deterministic.
4. Run import_regional_art.py in UnrealTest/RegionalArtTest.uproject using UE 5.8.2 PythonScript commandlet: default stage imports textures/materials; `-ArtStage=Meshes` imports meshes and checks physical bounds/material slots. The importer rejects a different project. Then run preview_regional_unreal.py in the isolated editor for lighting captures.
5. Run review_regional_exports.py after any map polish, inspect the 3×3 sheets and neutral/torchlit views, then rebuild the main game only when integration is authorized. The initial activation script refuses an existing main content root.

## Integration

New art lives under /Game/RegionalArt. DungeonRegionalArt.cpp maps existing architecture roles to the region's material set and places non-colliding dressing using existing wall boundaries. It skips sewer decoration so the approved sewer pack stays authoritative. Later floors rotate prop/decal choices and use selected alternate wall panels. DungeonGame.cpp resolves regional lever meshes and existing key identities; animation and interaction logic stay separate. Config/DefaultGame.ini includes the new content in packaged cooking.

Generation, progression, collision, campaign lore, map/radar and 2D enemies were not redesigned. Original materials and assets were preserved. Source/config and the previous packaged binaries/content are backed up under Saved/RegionalArt/Backup. To roll back presentation, restore the backed-up original source/config, remove the three newly introduced DungeonRegionalArt source files from compilation, and rebuild; the original assets remain present. Never replace the user's save files with review state.

## Validation evidence

- Export validation: native 2K dimensions, normalized forward-facing normals, transparent decal borders and coverage; PASS.
- All 46 mesh imports verified against physical bounds and material-slot bindings; all texture/instance parameter bindings validated in Unreal 5.8.2.
- 16 isolated Unreal captures plus Blender neutral model sheets, material 3×3 sheets and torchlit corridors. Visible export scale, curved shading, corrosion regularity and decal-shape issues were corrected during inspection.
- Seam checks cover 192 base-colour/normal/roughness maps and 12 optional metallic/emission maps. Decal sheets show actual straight-alpha transparency rather than a baked checkerboard.
- Packaged all-floor review: {results.strip().splitlines()[-1]}
- Runtime review checks boundary capsule sweeps, no-collision dressing, key loading/hover/pickup/removal, lever state/gates, map input blocking and existing 2D combat. Screenshots include entrance, passage, key, lever, map and combat for every floor.
- The save-disabled review fixture gives its hero first initiative for static combat captures, preventing accumulated enemy damage from interrupting the 18-floor visual inspection. This affects only the automated RegionalArtReview flag. Normal character stats and combat balance are unchanged. The corrected fixture was rebuilt into editor and packaged executables; existing cooked content remains valid because no assets or reflected types changed.
- The separate packaged shared-interactable review also passed, including crate loot collection, anchored key motion, lever gates and clearing/rebuilding the hover registry.
- Original-file hash verification: {json.dumps(preserved['counts'])}; zero mismatches. Review display preferences restored.

Reports and captures prove the checks described; they do not replace an owner visual review or a full manual campaign playthrough. The first-pass art remains stylized and uses the game's existing architectural meshes. The isolated showcase corridors are not new dungeon layouts.
'''
(R/'HANDOFF.md').write_text(readme,encoding='utf-8');shutil.copy2(R/'HANDOFF.md',D/'HANDOFF.md');shutil.copy2(R/'reports/unreal_lighting.json',D/'unreal_lighting.json');shutil.copy2(G/'Saved/RegionalArt/preservation.json',D/'preservation.json')
build=(G/'Saved/RegionalArt/build_verified.log').read_text(errors='replace');tests=(G/'Saved/Validation/tests.log').read_text(errors='replace');assert 'BUILD SUCCESSFUL' in build and '12 tests performed' in tests and 'Result={Fail' not in tests
p=G/'BUILD_STATUS.md';old=p.read_text();old=old.replace('Regional art expansion remains awaiting approval in `DUNGEON_REGIONAL_ART_PLAN.md`.','The subsequent regional expansion was approved and integrated; see the regional art handoff above.')
headline='Regional art expansion, **16 September 2026**: eight additional regional surface packs, 24 decorative props, eight themed levers and six floating world-key designs are integrated in the normal Windows build. All 12 regression suites and the packaged 18-floor art/interaction/collision review passed. Original assets, artwork, campaign and saves were preserved. Use `Play Regional Art Test.cmd` to choose a region with campaign saving disabled. Sources and review: `J:\\Lonemoore_Regional_Art`; handoff: `ArtReview/RegionalArt/HANDOFF.md`.\n\n'
if not old.startswith('Regional art expansion,'):p.write_text(headline+old)
p=G/'SHARED_INTERACTABLES_UPDATE.md';text=p.read_text();text=text.replace('The new lever and loot-crate models replace the old shared placeholders. World keys retain the original artwork and now float smoothly. Regional dungeon expansion remains awaiting approval in `DUNGEON_REGIONAL_ART_PLAN.md`; no new region pack was produced or integrated during this work.','Historical record of the initial shared-interactable pass: the lever and crate replaced their placeholders, and the original world-key illustrations gained a smooth hover. The subsequent regional expansion is now approved and integrated. It retains this crate and hover behavior while adding regional lever and 3D world-key models. Inventory key artwork remains original. See `ArtReview/RegionalArt/HANDOFF.md` for the current build.');p.write_text(text)
shutil.copy2(G/'Saved/Validation/tests.log',D/'regression_tests.log');print('REGIONAL_HANDOFF_READY',R/'REVIEW.html')
