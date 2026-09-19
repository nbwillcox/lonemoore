"""Create labeled contact sheets from real packaged-game screenshots."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import json

G = Path(r'J:\First Person Dungeon Crawler Game')
S = G / 'Builds/Windows/DungeonCrawler/Saved/RegionalIdentity'
D = G / 'ArtReview/RegionalIdentity/Runtime'
D.mkdir(parents=True, exist_ok=True)
font = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 21)
names = ['Cathedral', 'Sewers / Drowned Conduit', 'Sewers / Rat King',
         'Catacombs / Failed Chapel', 'Catacombs / Ossuary', 'Warrens / Scavenger Roads',
         'Warrens / Thorn Court', 'Crypts / Pilgrim Vigil', 'Crypts / Crimson Sepulchre',
         'Fortress / Warden Cells', 'Fortress / Hollow Keep', 'Deep / Chasm of Wings',
         'Deep / Drake Heart', 'Infernal / Sealed Witness', 'Infernal / Ashen Compact',
         'Hell / Ashen Threshold', 'Hell / Hellforge', 'Hell / Broken Halo']
boards = []
for category in ['Entrance', 'Passage', 'Key', 'Lever', 'Map', 'Combat']:
    for start in range(0, 18, 6):
        paths = [S / f'Floor_{floor+1:02d}_{category}.png' for floor in range(start, start+6)]
        if not all(p.exists() for p in paths):
            continue
        board = Image.new('RGB', (1920, 800), (16, 18, 20))
        draw = ImageDraw.Draw(board)
        for i, path in enumerate(paths):
            im = Image.open(path)
            assert im.size == (1600, 900), (path, im.size)
            x, y = i % 3 * 640, i // 3 * 400
            board.paste(im.convert('RGB').resize((640, 360), Image.Resampling.LANCZOS), (x, y))
            draw.text((x+10, y+363), f'{start+i+1:02d} {names[start+i]}', font=font, fill='#eadcc2')
        path = D / f'{category}_{start+1:02d}-{start+6:02d}.jpg'
        board.save(path, quality=95)
        boards.append(str(path))
(G / 'Saved/RegionalIdentity/gameplay_boards.json').write_text(json.dumps(boards, indent=2))
comparison_paths = [S / 'Floor_02_Passage.png', S / 'Floor_16_Passage.png']
if all(p.exists() for p in comparison_paths):
    board = Image.new('RGB', (1600, 494), (16, 18, 20))
    draw = ImageDraw.Draw(board)
    for i, (path, label) in enumerate(zip(comparison_paths, ['Old City Sewers | main game', 'Hell: Ashen Threshold | main game'])):
        im = Image.open(path).convert('RGB').resize((800, 450), Image.Resampling.LANCZOS)
        board.paste(im, (i*800, 0))
        draw.text((i*800+12, 458), label, font=font, fill='#eadcc2')
    board.save(D / 'Hell_vs_Sewers_Main_Game.jpg', quality=96)
print('PACKAGED_GAMEPLAY_BOARDS', len(boards))
