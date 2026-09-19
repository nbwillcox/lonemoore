"""Verify final exported maps and refresh visual review sheets after surface polishing."""
from pathlib import Path
import json
import numpy as np
from PIL import Image, ImageDraw
R=Path(r'J:\Lonemoore_Regional_Art');M=json.loads((R/'manifest.json').read_text())
report={'errors':[],'maps':0,'decals':[],'normal_length_max_error':0}
for a in M['materials']+M['decals']:
 for role,path in a['maps'].items():
  im=Image.open(R/path);report['maps']+=1
  if im.size!=(2048,2048):report['errors'].append(a['id']+': wrong resolution')
  if role=='normal':
   n=np.asarray(im,dtype=np.float32)/127.5-1
   error=float(np.max(np.abs(np.linalg.norm(n,axis=2)-1)));report['normal_length_max_error']=max(report['normal_length_max_error'],error)
   if error>.018 or n[:,:,2].min()<0:report['errors'].append(a['id']+': invalid normal')
 if a['role']=='decal':
  im=Image.open(R/a['maps']['base_color']);assert im.mode=='RGBA'
  alpha=np.asarray(im)[:,:,3];edge=max(alpha[0].max(),alpha[-1].max(),alpha[:,0].max(),alpha[:,-1].max())
  row={'id':a['id'],'border_alpha':int(edge),'coverage':float((alpha>0).mean()),'alpha_max':int(alpha.max())};report['decals'].append(row)
  if edge or not .01<row['coverage']<.95 or row['alpha_max']<50:report['errors'].append(a['id']+': invalid transparency')
 else:
  tile=Image.open(R/a['maps']['base_color']).convert('RGB').resize((360,360),Image.Resampling.LANCZOS);sheet=Image.new('RGB',(1080,1080))
  for y in range(3):
   for x in range(3):sheet.paste(tile,(x*360,y*360))
  sheet.save(R/'previews'/(a['id']+'_3x3.jpg'),quality=93)
sheet=Image.new('RGB',(1600,1320),(32,35,39));draw=ImageDraw.Draw(sheet)
for i,a in enumerate(M['decals']):
 x=i%8*200;y=i//8*330;bg=Image.new('RGBA',(196,280))
 d=ImageDraw.Draw(bg)
 for yy in range(0,280,20):
  for xx in range(0,196,20):d.rectangle((xx,yy,xx+19,yy+19),fill=(83,86,89,255) if (xx+yy)//20%2 else (55,58,61,255))
 im=Image.open(R/a['maps']['base_color']).resize((196,280),Image.Resampling.LANCZOS);bg.alpha_composite(im);sheet.paste(bg.convert('RGB'),(x,y));draw.text((x+3,y+286),a['id'].replace('_','\n'),fill='white')
 preview=Image.new('RGBA',(512,512));pd=ImageDraw.Draw(preview)
 for yy in range(0,512,32):
  for xx in range(0,512,32):pd.rectangle((xx,yy,xx+31,yy+31),fill=(95,95,90,255) if (xx+yy)//32%2 else (145,145,135,255))
 preview.alpha_composite(Image.open(R/a['maps']['base_color']).resize((512,512),Image.Resampling.LANCZOS));preview.convert('RGB').save(R/'previews'/(a['id']+'_alpha.jpg'),quality=95)
sheet.save(R/'previews/All_Decals_Transparency.jpg',quality=94)
report['status']='PASS' if not report['errors'] else 'FAIL';(R/'reports/export_validation.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2));assert not report['errors']
