"""Give story marks their actual shapes: claw gouges, erased names and fractured seals."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFilter
import numpy as np
R=Path(r'J:\Lonemoore_Regional_Art');N=2048;rng=np.random.default_rng(976)
for name in ['Deep_ClawScars','Crypts_ErasedName','Infernal_BrokenSeal','Hell_BrokenSigil']:
 mask=Image.new('L',(N,N));d=ImageDraw.Draw(mask)
 if 'Claw' in name:
  for x,y,lean,length in [(620,420,-240,1030),(970,280,-210,1330),(1300,440,-155,1150)]:
   d.polygon([(x-4,y),(x+25,y+120),(x+lean+14,y+length-120),(x+lean,y+length),(x+lean-12,y+length-150),(x-10,y+90)],fill=190)
   d.line([(x+15,y+180),(x+lean+20,y+length-220)],fill=100,width=7)
 elif 'Erased' in name:
  for y in [650,875,1100,1325]:
   for x in range(480,1550,145):
    d.line([(x,y+95),(x+int(rng.integers(0,80)),y),(x+85,y+90)],fill=115,width=8)
  for i in range(36):
   x=int(rng.integers(390,1480));y=int(rng.integers(580,1510));d.line([(x,y),(x+int(rng.integers(90,350)),y-int(rng.integers(25,65)))],fill=170,width=int(rng.integers(3,9)))
 else:
  for b in [(400,400,1648,1648),(460,460,1588,1588)]:
   for start,end in [(8,68),(85,147),(160,223),(245,322),(337,355)]:d.arc(b,start,end,fill=170,width=11)
  if 'Hell' in name:
   d.line([(570,700),(1470,700),(800,1480),(1024,450),(1260,1480),(570,700)],fill=180,width=13)
  else:
   for x in [820,1024,1228]:d.line([(x-40,1210),(x,760),(x+60,970)],fill=160,width=12)
  # Missing segments and a ragged fracture cross the carved motif.
  d.line([(460,1300),(870,1080),(1000,1150),(1370,870),(1540,925)],fill=0,width=45)
 noise=rng.random((256,256));noise=np.asarray(Image.fromarray((noise*255).astype('uint8')).resize((N,N),Image.Resampling.BICUBIC),dtype=np.float32)/255
 alpha=np.asarray(mask.filter(ImageFilter.GaussianBlur(.7)),dtype=np.float32)*(.4+.6*noise)
 rgb=np.empty((N,N,4),dtype=np.uint8);rgb[:,:,:3]=[64,55,45] if name.startswith(('Deep','Crypts')) else [79,37,18];rgb[:,:,3]=alpha.astype('uint8')
 Image.fromarray(rgb).save(R/'textures'/name/(name+'_BC.png'))
 print('DECAL_REFINED',name)
