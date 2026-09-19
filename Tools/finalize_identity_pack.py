"""Assemble a factual review gallery and integration handoff from actual reports."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json,shutil,html
G=Path(r'J:\First Person Dungeon Crawler Game');R=Path(r'J:\Lonemoore_Regional_Identity');M=json.loads((R/'manifest.json').read_text());dest=G/'ArtReview/RegionalIdentity';dest.mkdir(parents=True,exist_ok=True)
assert not (G/'Content/RegionalIdentity').exists(), 'Pack already integrated; use finalize_identity_integration.py to preserve current validation status.'
reports={name:json.loads((R/'reports'/name).read_text()) for name in ['export_validation.json','unreal_textures.json','unreal_meshes.json','unreal_lighting.json','preservation.json']}
assert reports['unreal_lighting.json']['status']=='CAPTURES_COMPLETE' and not reports['unreal_lighting.json']['errors']
assert all(reports[n]['status']=='PASS' for n in ['export_validation.json','unreal_textures.json','unreal_meshes.json'])
M['status']='ISOLATED_UNREAL_VALIDATED_AWAITING_RUNTIME_INTEGRATION'
M['reference_images_inspected']=['.art/.concepts/'+s+'.png' for s in ['hell-gate','abyssal-descent','sewer-tunnels-cistern','fungal-caverns','grand-catacombs','desecrated-temple','old-prison-cells']]
M['reference_access']='All listed images were accessible and visually inspected; source images were not altered or pasted into material maps.'
M['image_generation']='Available, not used. All new exports were produced procedurally and with Blender geometry.'
M['runtime_changes']='Prepared for review in Tools/prepare_identity_runtime.py; not yet applied.'
for a in M['materials']:
 a['unreal_uv_material']='/Game/RegionalIdentity/Materials/MI_UV_'+a['id'];a['unreal_world_material']='/Game/RegionalIdentity/Materials/MI_'+a['id']
for p in M['props']:p['unreal_mesh']='/Game/RegionalIdentity/Meshes/'+p['id']
(R/'manifest.json').write_text(json.dumps(M,indent=2))
try:font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',21)
except:font=ImageFont.load_default()
entries=[('Sewers','Old City Sewers - approved benchmark')]+[(r['code'],r['name']) for r in M['regions']]
for lighting in ['Neutral','Torch']:
 board=Image.new('RGB',(1920,1224),(17,18,20));draw=ImageDraw.Draw(board)
 for i,(code,name) in enumerate(entries):
  im=Image.open(R/'previews'/(code+'_'+lighting+'_Unreal.png'));assert im.size==(1600,900);im=im.resize((640,360),Image.Resampling.LANCZOS);xx=(i%3)*640;yy=(i//3)*408;board.paste(im,(xx,yy));draw.text((xx+12,yy+366),name,fill=(235,227,206),font=font)
 path=R/'previews'/('All_Regions_'+lighting+'_Unreal.jpg');board.save(path,quality=94);shutil.copy2(path,dest/path.name)
comparison=Image.new('RGB',(1600,950),(17,18,20));draw=ImageDraw.Draw(comparison)
for i,(code,title) in enumerate([('Sewers','Old City Sewers'),('Hell','Hell - Ashen Threshold')]):
 im=Image.open(R/'previews'/(code+'_Neutral_Unreal.png')).resize((800,450),Image.Resampling.LANCZOS);comparison.paste(im,(i*800,0));draw.text((i*800+16,458),title+' | identical neutral lights',fill='white',font=font)
 im=Image.open(R/'previews'/(code+'_Torch_Unreal.png')).resize((800,450),Image.Resampling.LANCZOS);comparison.paste(im,(i*800,500))
comparison.save(R/'previews/Hell_vs_Sewers_Unreal.jpg',quality=95);shutil.copy2(R/'previews/Hell_vs_Sewers_Unreal.jpg',dest/'Hell_vs_Sewers_Unreal.jpg')
head='''<!doctype html><html><head><meta charset="utf-8"><title>Lonemoore | Regional identity review</title><style>body{background:#111416;color:#e5e1d6;font:17px/1.55 system-ui;margin:35px auto;max-width:1450px;padding:0 24px}h1,h2{color:#dfc399}img{max-width:100%;display:block;border-radius:6px}section{margin:36px 0;border-top:1px solid #45433b;padding-top:24px}.pair{display:grid;grid-template-columns:1fr 1fr;gap:18px}a{color:#c3dbec}small{color:#b6b4ae}summary{cursor:pointer;padding:10px}p{max-width:1000px}</style></head><body>'''
body='<h1>Regional identity correction</h1><p>New walls, floors, ceilings and architectural forms for the eight regions beyond the approved sewer pack. These captures use Unreal 5.8.2 in an isolated test project, at 155 cm eye height and 76° field of view. Each comparison uses the same lights and exposure.</p><p><b>Integration pending.</b> These are isolated Unreal tests, not captures from a newly packaged main game. Existing keys, levers, crates, decals and enemies are retained by the proposed integration.</p><img src="previews/Hell_vs_Sewers_Unreal.jpg" alt="Hell and sewer comparison">'
for code,name in entries:
 body+='<section><h2>'+html.escape(name)+'</h2><div class="pair">'
 for light in ['Neutral','Torch']:body+='<div><img src="previews/'+code+'_'+light+'_Unreal.png"><small>'+light+' lighting · isolated Unreal</small></div>'
 body+='</div>'
 if code!='Sewers':
  reg=next(r for r in M['regions'] if r['code']==code);body+='<p>'+html.escape(reg['direction'])+'</p><details><summary>Blender source preview and tiled material checks</summary><div class="pair"><img src="previews/'+code+'_Neutral_Blender.png"><img src="previews/'+code+'_Torch_Blender.png"></div>'
  for a in M['materials']:
   if a['region']==reg['index']:body+='<p>'+a['id']+' · '+a['role']+' · native 2K / 2 m repeat</p><img loading="lazy" src="previews/'+a['id']+'_3x3.jpg">'
  body+='</details>'
 body+='</section>'
body+='<section><h2>Hell floor progression</h2><div class="pair"><div><img src="previews/Hell_ForgeFloor_Neutral_Unreal.png"><p>The Hellforge: riveted foundry plates.</p></div><div><img src="previews/Hell_HaloFloor_Neutral_Unreal.png"><p>The Broken Halo: fractured dark stone.</p></div></div></section></body></html>'
(R/'REVIEW.html').write_text(head+body,encoding='utf8')
handoff='''# Regional identity correction — integration handoff

Status: new assets passed isolated Blender and Unreal validation; main integration is pending. Automatic approval review rejected running the rendering-patch staging step under the earlier art-only restriction. No main source, config, existing uasset or original artwork was modified.

## Scope

34 native 2048×2048 material sets (107 maps), 50 architectural visual modules, eight editable Blender scenes, 34 physical height sources, and a reusable UE material pair with 68 instances. New region index mapping remains 0 Cathedral, 2 Catacombs, 3 Warrens, 4 Crypts, 5 Fortress, 6 Deep, 7 Infernal, 8 Hell. Index 1 keeps the approved Old City Sewers pack. Hell floors 15, 16 and 17 use CooledSlag, HellforgePlates and BrokenHaloStone respectively. No lore or stable IDs change.

## Evidence and physical conventions

Blender 5.2 produced the meshes and editable sources. Unreal 5.8.2 imported all 107 textures and 50 meshes, with centimetre dimensions verified against the Blender exports. Base color is sRGB; normals are DirectX, linear/BC5 with no green flip in Unreal; roughness, metal and emission masks are linear. Blender flips the normal green channel for its preview. No cast light or perspective is baked into albedo. All textures represent a 2 m repeat (1024 pixels/m), not upscaled source images. No 4K assets are used. Significant wall recesses, braces and ceiling forms are actual geometry. Export checks reject degenerate or badly stretched UVs. Ceiling-to-wall joins were closed and the final outputs were re-rendered.

Reports live in reports/. REVIEW.html separates Blender previews from isolated Unreal captures. The sewer control uses the original approved materials and geometry copied/imported read-only into the test project, with no extra props, to keep the comparison about building fabric.

## Runtime integration prepared for review

Tools/prepare_identity_runtime.py describes the presentation change. It reads the current main rendering files and would stage revised copies in this sibling workspace; it has not been run successfully. The requested change adds DungeonIdentityArt.h/.cpp and alters only DungeonEnvironment.cpp, DungeonRegionalArt.cpp and the save-disabled DungeonRegionalArtReview.cpp fixture. Existing physical modules remain hidden collision components. New regional visual instances are NoCollision. Secret-door cosmetic children follow the original moving door actor. The old flat lower-wall repair strips are removed because the new geometry supplies that facing. Original small props, decals, keys, levers, animation and all gameplay are retained. Tall walls should use the provided world-projected instances to retain physical texture scale.

After explicit approval, validate current source hashes against Saved/RegionalIdentity/Backup before applying the patch; reconcile any other task's edits. Copy only UnrealTest/Content/RegionalIdentity/{Materials,Textures,Meshes} into a new main Content/RegionalIdentity directory. Do not copy benchmark or review maps into the main game. Add /Game/RegionalIdentity to the existing cook directory list. Build, run the existing regression suites, then run the all-18-floor RegionalArtReview and SharedPropReview in the packaged game with saving disabled. Validate boundary sweeps, doors, keys, levers, 2D combat, map/radar, missing assets and all new architectural batches. Restore and hash-check display preferences and existing saves. Actual packaged validation remains outstanding until those steps run.

## Repeatable production

Scripts are mirrored in scripts/. setup_identity_pass.py is an initializer and must not be blindly rerun over a finished revision. bake_identity_surfaces.py generates periodic physical surfaces and optional region subsets. build_identity_architecture.py regenerates FBX, UVs, .blend scenes and Blender previews. import_identity_art.py is guarded to this isolated Unreal project; run texture stage, then -ArtStage=Meshes. stage_identity_benchmark.py and import_identity_benchmark.py prepare the read-only sewer comparison. preview_identity_unreal.py builds the review maps and captures. verify_identity_pack.py audits exports and main preservation hashes; finalize_identity_pack.py builds this handoff and gallery. Image generation was available but not used.
'''
(R/'HANDOFF.md').write_text(handoff,encoding='utf8')
for p in list((G/'Tools').glob('*identity*.py')):shutil.copy2(p,R/'scripts'/p.name)
for name in ['HANDOFF.md','manifest.json']:shutil.copy2(R/name,dest/name)
print('IDENTITY_REVIEW_READY',R/'REVIEW.html',flush=True)
