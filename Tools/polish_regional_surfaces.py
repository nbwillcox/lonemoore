"""Final irregular corrosion and plaster wear pass; avoids grid-like value-noise blotches."""
from pathlib import Path
import numpy as np,json
from PIL import Image
R=Path(r'J:\Lonemoore_Regional_Art');N=2048;y,x=np.mgrid[0:N,0:N].astype(np.float32)/N
def smooth(a,l,h):t=np.clip((a-l)/(h-l),0,1);return t*t*(3-2*t)
def noise(f):
 a=rng.random((f,f),dtype=np.float32);gx=x*f+1.1*np.sin(2*np.pi*(y*3+x*2))+.6*np.sin(2*np.pi*(y*7-x*3));gy=y*f+np.sin(2*np.pi*(x*4-y*2));ix=np.floor(gx).astype(int);iy=np.floor(gy).astype(int);tx=gx%1;ty=gy%1;tx=tx*tx*(3-2*tx);ty=ty*ty*(3-2*ty)
 return (a[iy%f,ix%f]*(1-tx)+a[iy%f,(ix+1)%f]*tx)*(1-ty)+(a[(iy+1)%f,ix%f]*(1-tx)+a[(iy+1)%f,(ix+1)%f]*tx)*ty
def save(a,p):Image.fromarray(np.uint8(np.clip(a,0,1)*255+.5)).save(p)
checks=[]
for reg in json.loads((R/'regions.json').read_text()):
 for sp in reg['materials']:
  if sp['kind'] not in ['metal','plaster']:continue
  name=sp['id'];print('POLISH',name,flush=True);rng=np.random.default_rng(sum(ord(c) for c in name));f=(noise(5)-.5)*.4+(noise(19)-.5)*.35+(noise(89)-.5)*.25;fine=noise(401)-.5;base=np.array(sp['color']);d=R/'textures'/name
  if sp['kind']=='metal':
   rust=smooth(noise(13)*.65+noise(47)*.35,.22,.72);c=np.array([.225,.23,.235])*(1-rust[:,:,None])+base*rust[:,:,None]+f[:,:,None]*.10;h=f*.0008+fine*.00009;rough=.60+.26*rust;save((1-rust)*.80,d/(name+'_M.png'))
  else:
   loss=smooth(noise(11)*.65+noise(67)*.35,.55,.78);c=base+f[:,:,None]*.14-loss[:,:,None]*.07;h=f*.002+fine*.0002-loss*.003;rough=.91+f*.09
  dx=(np.roll(h,-1,1)-np.roll(h,1,1))/(4/N);dy=(np.roll(h,-1,0)-np.roll(h,1,0))/(4/N);n=np.stack([-dx,-dy,np.ones_like(h)],2);n/=np.linalg.norm(n,axis=2,keepdims=True);n=n*.5+.5
  for role,a in [('BC',c),('N',n),('R',rough)]:
   save(a,d/(name+'_'+role+'.png'));seam=float((abs(a[0]-a[-1]).mean()+abs(a[:,0]-a[:,-1]).mean())/2);inner=float((abs(np.diff(a,axis=0)).mean()+abs(np.diff(a,axis=1)).mean())/2);checks.append(dict(id=name,map=role,seam=seam,interior=inner,ratio=seam/max(inner,1e-7)))
  np.save(R/'sources'/(name+'_height_m.npy'),h)
old=json.loads((R/'reports/seams.json').read_text());ids={a['id'] for a in checks};(R/'reports/seams.json').write_text(json.dumps([a for a in old if a['id'] not in ids]+checks,indent=2))
