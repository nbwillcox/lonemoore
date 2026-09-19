"""Build review boards from actual game captures without altering the images."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageStat,ImageFont
import json,sys
G=Path(__file__).resolve().parents[1];P=G/'Builds/Windows/DungeonCrawler/Saved/RegionalArt';O=G/'Saved/RegionalArt';camp=json.loads((G/'Content/Game/Data/campaign.json').read_text())
for start in [0,6,12]:
 paths=[(i,P/f'Floor_{i+1:02d}_Passage.png') for i in range(start,start+6) if (P/f'Floor_{i+1:02d}_Passage.png').exists()]
 if not paths:continue
 im=Image.new('RGB',(1600,1485),(20,23,27));d=ImageDraw.Draw(im);font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',14)
 for j,(i,p) in enumerate(paths):
  pic=Image.open(p).convert('RGB');assert pic.size==(1600,900);x=j%2*800;y=j//2*495;im.paste(pic.resize((800,450),Image.Resampling.LANCZOS),(x,y));d.text((x+10,y+462),camp['floors'][i]['name'],fill='white',font=font)
 im.save(O/f'Floors_{start+1:02d}_{start+6:02d}_Packaged.jpg',quality=95)
if '--final' in sys.argv:
 rows=[]
 for i in range(18):
  for kind in ['Entrance','Passage','Key','Lever','Map','Combat']:
   p=P/f'Floor_{i+1:02d}_{kind}.png';im=Image.open(p).convert('RGB');assert im.size==(1600,900);mean=sum(ImageStat.Stat(im).mean)/3;assert mean>10,(p,mean);rows.append({'file':p.name,'mean_rgb':mean})
 (O/'capture_validation.json').write_text(json.dumps({'status':'PASS','captures':len(rows),'images':rows},indent=2));print('PACKAGED_CAPTURE_VALIDATION_PASS',len(rows))
