"""Build a compact listening/animation page and native screenshot contact sheet."""
from pathlib import Path
import html
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'ArtReview/CombatFX'
CORE = [('hit', 'Metal slash'), ('critical', 'Critical strike'), ('fireball', 'Fireball'),
        ('ice_lance', 'Ice lance'), ('shadow_bolt', 'Shadow bolt'), ('defend', 'Defend'),
        ('dodge', 'Dodge')]
cards = []
for cue, label in CORE:
    audio = f'../../ArtSource/CombatAudio/Wav/{cue}.wav'
    cards.append(f'<article><h2>{html.escape(label)}</h2><img src="{cue}.gif" alt="{html.escape(label)} animation"><audio controls preload="none" src="{audio}"></audio></article>')
page = '''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Lonemoore — Combat Effects</title>
<style>body{background:#151720;color:#eee;font:16px system-ui;margin:32px auto;max-width:1150px;padding:0 20px}h1{color:#ffdf73}p{line-height:1.6;color:#ccd0de}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:20px}article{border:1px solid #454756;border-radius:12px;padding:16px;background:#1d202b}h2{font-size:20px}article img{display:block;width:100%;image-rendering:pixelated}audio{width:100%}a{color:#a5dcff}.native{width:100%;height:auto}</style>
<h1>Lonemoore — Combat Effects</h1><p>Original console-style sounds and animated pixel effects. Use each player's play button to hear its sound. Open <strong>Review Combat Effects.cmd</strong> in the game folder for the in-game showcase, or <strong>Play Lonemoore.cmd</strong> to play normally.</p>
<p><a href="../AdventurePolish/Audio/REVIEW.html">Listen to all 33 sounds</a> · <a href="contact-sheet.png">All sprite animation frames</a></p><main>'''
page += '\n'.join(cards) + '</main>'

candidates = [ROOT/'Saved/CombatFXPass/Runtime', ROOT/'Builds/Windows/DungeonCrawler/Saved/CombatFXPass/Runtime']
candidates += list((ROOT/'Saved/CombatFXPass/ReviewUser').glob('**/CombatFXPass/Runtime'))
runtime = next((p for p in candidates if p.exists() and list(p.glob('*.png'))), None)
if runtime:
    selected = []
    for cue, label in CORE:
        files = list(runtime.glob(f'*_{cue}.png'))
        if files:
            selected.append((files[0], label))
    w, h = 800, 480
    board = Image.new('RGB', (w*2, h*((len(selected)+1)//2)), '#151720')
    draw = ImageDraw.Draw(board)
    for i, (path, label) in enumerate(selected):
        x, y = (i%2)*w, (i//2)*h
        image = Image.open(path).convert('RGB')
        image.thumbnail((800,450))
        board.paste(image,(x,y+30))
        draw.text((x+12,y+8), label, fill='#ffdf73')
    board.save(OUT/'in-game-effects.jpg', quality=92)
    page += '<h2>Packaged game captures</h2><a href="in-game-effects.jpg"><img class="native" src="in-game-effects.jpg" alt="Effects captured in the packaged game"></a>'
page += '</html>'
(OUT/'REVIEW.html').write_text(page, encoding='utf-8')
print(f'Review page: {OUT / "REVIEW.html"}; runtime captures: {runtime}')
