from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
import json
R=Path(r'J:\Lonemoore_Regional_Identity');M=json.loads((R/'manifest.json').read_text())
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',18)
for page in range(4):
 rows=M['materials'][page*9:(page+1)*9];board=Image.new('RGB',(1440,1530),(20,22,24));draw=ImageDraw.Draw(board)
 for i,a in enumerate(rows):
  im=Image.open(R/'previews'/(a['id']+'_3x3.jpg')).crop((1152,44,2304,1196)).resize((480,480),Image.Resampling.LANCZOS)
  xx=i%3*480;yy=i//3*510;board.paste(im,(xx,yy+30));draw.text((xx+8,yy+5),a['id'],fill='white',font=font)
 board.save(R/'previews'/('Tiled_QA_'+str(page+1)+'.jpg'),quality=94)
print('TILED_BOARDS_READY')
