"""Native 2K periodic height-field texture production; sRGB pigments, DirectX normals."""
from pathlib import Path
import numpy as np,json
from PIL import Image,ImageDraw,ImageFont
R=Path(r'J:\Lonemoore_Regional_Art');N=2048;y,x=np.mgrid[0:N,0:N].astype(np.float32)/N
regions=json.loads((R/'regions.json').read_text());assets=[];decals=[];checks=[];rng=np.random.default_rng(93016)
def smooth(a,l,h):
 t=np.clip((a-l)/(h-l),0,1);return t*t*(3-2*t)
def noise(f):
 a=rng.random((f,f),dtype=np.float32);gx=x*f;gy=y*f;ix=gx.astype(int)%f;iy=gy.astype(int)%f;tx=gx%1;ty=gy%1;tx=tx*tx*(3-2*tx);ty=ty*ty*(3-2*ty)
 return (a[iy,ix]*(1-tx)+a[iy,(ix+1)%f]*tx)*(1-ty)+(a[(iy+1)%f,ix]*(1-tx)+a[(iy+1)%f,(ix+1)%f]*tx)*ty
def save(a,p):Image.fromarray(np.uint8(np.clip(a,0,1)*255+.5)).save(p)
def normal(h):
 dx=(np.roll(h,-1,1)-np.roll(h,1,1))/(4/N);dy=(np.roll(h,-1,0)-np.roll(h,1,0))/(4/N);n=np.stack([-dx,-dy,np.ones_like(h)],2);n/=np.linalg.norm(n,axis=2,keepdims=True);return n*.5+.5
def output(name,role,c,h,rough,metal=None,emission=None):
 d=R/'textures'/name;d.mkdir(exist_ok=True);maps={};n=normal(h)
 for k,a in [('BC',c),('N',n),('R',rough),('M',metal),('E',emission)]:
  if a is None:continue
  save(a,d/(name+'_'+k+'.png'));maps[{'BC':'base_color','N':'normal','R':'roughness','M':'metallic','E':'emission'}[k]]=str((d/(name+'_'+k+'.png')).relative_to(R))
 np.save(R/'sources'/(name+'_height_m.npy'),h.astype(np.float32))
 for k,a in [('BC',c),('N',n),('R',rough)]:
  seam=float((abs(a[0]-a[-1]).mean()+abs(a[:,0]-a[:,-1]).mean())/2);inner=float((abs(np.diff(a,axis=0)).mean()+abs(np.diff(a,axis=1)).mean())/2);checks.append(dict(id=name,map=k,seam=seam,interior=inner,ratio=seam/max(inner,1e-7)))
 a=dict(id=name,role=role,region=reg['index'],maps=maps,uv_span_m=2,resolution=N);assets.append(a)
 tile=Image.open(d/(name+'_BC.png')).resize((320,320),Image.Resampling.LANCZOS);board=Image.new('RGB',(960,996),(23,25,25))
 for j in range(3):
  for i in range(3):board.paste(tile,(i*320,j*320))
 ImageDraw.Draw(board).text((12,970),name+' | 3x3 repeats / 6 metres | base color',fill='white');board.save(R/'previews'/(name+'_3x3.jpg'),quality=94)
for reg in regions:
 for sp in reg['materials']:
  name=sp['id'];kind=sp['kind'];print('MATERIAL',name,kind,flush=True);rng=np.random.default_rng(91600+sum((i+1)*ord(ch) for i,ch in enumerate(name)))
  f=(noise(5)-.5)*.5+(noise(17)-.5)*.28+(noise(67)-.5)*.15+(noise(271)-.5)*.07;mid=noise(89)-.5;fine=noise(569)-.5;base=np.array(sp['color'],np.float32);h=f*.002+mid*.0003;rough=.82+f*.2;c=base+f[:,:,None]*.16;metal=em=None
  if kind in ['ashlar','brick','flags','engraved','plate']:
   nx,ny=sp['nx'],sp['ny'];row=np.floor(y*ny).astype(int);offset=(row%2)*(.5 if kind!='flags' else .25);u=(x*nx+offset)%1;v=y*ny%1;dist=np.minimum(np.minimum(u,1-u)*2/nx,np.minimum(v,1-v)*2/ny)
   chip=smooth(noise(143),.62,.88)*(.006 if kind!='plate' else .0005);body=smooth(dist-chip,.004,.016);ids=(np.floor(x*nx+offset).astype(int)%nx)+row*nx;var=rng.uniform(-.035,.035,nx*ny)[ids]
   h=body*(.010+f*.004+mid*.0006+fine*.0002);c=(base+var[:,:,None]+f[:,:,None]*.14)*body[:,:,None]+(base*.65)*(1-body[:,:,None]);rough=.80+f*.18+(1-body)*.12
   if kind=='flags':
    crack=(1-smooth(abs(np.sin(2*np.pi*(x*2+y*3+f*.4))),.006,.023))*smooth(noise(7),.55,.7)*body;h-=crack*.002;c-=crack[:,:,None]*.025
   if kind=='engraved':
    ring=abs(np.sqrt(((u-.5)*1.15)**2+((v-.5)*.8)**2)-.23);mark=(1-smooth(ring,.014,.025))*body;bar=(1-smooth(abs(u-.5),.011,.022))*smooth(v,.18,.23)*(1-smooth(v,.77,.82));mark=np.maximum(mark,bar)*body
    h-=mark*.0025;c-=mark[:,:,None]*.038
   if kind=='plate':
    studs=np.exp(-((u-.09)**2+(v-.10)**2)/.0005)+np.exp(-((u-.91)**2+(v-.90)**2)/.0005);h+=studs*.004;rough=.65+f*.16;metal=.55*body
  elif kind=='plaster':
   loss=smooth(noise(7)+mid*.2,.59,.77);h=f*.003+mid*.0007-loss*.004;c=base+f[:,:,None]*.13-loss[:,:,None]*.075;rough=.9+f*.09
  elif kind in ['strata','rock','slag','heat','veins']:
   ridge=np.sin(2*np.pi*(y*(13 if kind=='strata' else 5)+x*2+np.sin(x*2*np.pi*3)*.25+f*.7));breaks=1-smooth(abs(np.sin(2*np.pi*(x*3+y*2+f))),.008,.03)
   if kind=='rock':ridge=np.cos(2*np.pi*x*7+f*15)*np.sin(2*np.pi*y*5+f*9)
   h=f*.012+ridge*.0018-breaks*.003+mid*.0006;c=base+f[:,:,None]*.21+ridge[:,:,None]*.025
   if kind=='veins':c+=breaks[:,:,None]*np.array([.10,.11,.08]);h+=breaks*.001;rough=.69+f*.18
   if kind in ['slag','heat']:
    pores=smooth(noise(217),.65,.88);h-=pores*.002;c-=pores[:,:,None]*.025;rough=.84+f*.13
   if kind=='heat':em=breaks*smooth(noise(7),.35,.62);c+=em[:,:,None]*np.array([.18,.045,.008])
  elif kind=='gravel':
   grains=smooth(noise(137),.35,.77);h=f*.003+grains*.0018+fine*.0003;c=base+f[:,:,None]*.17+grains[:,:,None]*.027;rough=.94+f*.06
  elif kind=='wood':
   warp=f*.018+.015*np.sin(y*2*np.pi);grain=(.5+.5*np.sin((x+warp)*2*np.pi*109))**14;plank=(x*sp['nx'])%1;edge=smooth(np.minimum(plank,1-plank),.009,.024);h=edge*.004-grain*.00035+f*.0005;c=(base+f[:,:,None]*.17-grain[:,:,None]*.037)*(.55+.45*edge[:,:,None]);rough=.82+f*.15
  elif kind=='metal':
   rust=smooth(noise(11)+mid*.4,.34,.63);c=np.array([.31,.32,.33])*(1-rust[:,:,None])+base*rust[:,:,None]+f[:,:,None]*.12;h=f*.001+mid*.0002+fine*.00012;rough=.46*(1-rust)+.86*rust;metal=(1-rust)*.88
  elif kind=='cloth':
   weave=np.sin(x*2*np.pi*420)*np.cos(y*2*np.pi*360);h=weave*.00008+f*.0003;c=base+f[:,:,None]*.09+weave[:,:,None]*.012;rough=.94+f*.05
  elif kind=='bone':
   h=f*.0005+mid*.0001;c=base+f[:,:,None]*.11;rough=.73+f*.1
  elif kind=='growth':
   tend=(.5+.5*np.sin(2*np.pi*(x*58+np.sin(y*2*np.pi*7)*.7)))**5;h=f*.003+tend*.0005;c=base+f[:,:,None]*np.array([.10,.15,.06]);rough=.9+f*.09
  output(name,sp['role'],c,h,np.clip(rough,.4,.98),metal,em)
 for k,name in enumerate(reg['decals']):
  print('DECAL',name,flush=True);f=noise(9)-.5;fine=noise(111);edge=smooth(x,.03,.12)*(1-smooth(x,.88,.97))*smooth(y,.03,.1)*(1-smooth(y,.9,.97));col=np.array([.20,.18,.14]);rough=.9
  if any(z in name for z in ['Wax','Chalk','Salt','Mineral']):
   a=smooth(f+np.sin(x*23+y*9)*.08,.02,.22)*edge*.85;col=np.array([.56,.52,.40])
  elif any(z in name for z in ['Seal','Sigil','Name','Scars']):
   rad=np.sqrt((x-.5)**2+(y-.5)**2);a=(1-smooth(abs(rad-.28),.012,.024))*edge*(.45+.55*fine);a=np.maximum(a,(1-smooth(abs(x-.5),.012,.025))*smooth(y,.19,.22)*(1-smooth(y,.78,.81))*.7)*edge
   col=np.array([.28,.14,.075]) if reg['index']>=7 else np.array([.24,.23,.20])
  elif any(z in name for z in ['Run','Seep','Fall','Trace']):
   a=np.zeros_like(x)
   for cx,w,length in [(.26,.025,.67),(.45,.055,.86),(.68,.026,.59)]:a+=np.exp(-((x-cx-.02*np.sin(y*18))/w)**2)*(1-smooth(y,length,length+.13))
   a=np.clip(a,0,1)*edge*(.45+.35*fine);col=np.array([.22,.115,.055]) if 'Rust' in name else np.array([.18,.19,.13])
  else:
   rad=np.sqrt(((x-.5)/.44)**2+((y-.62)/.32)**2);a=(1-smooth(rad+f*.4,.55,1))*edge*(.3+.5*fine);col=np.array([.10,.085,.07]) if any(z in name for z in ['Soot','Scorch']) else np.array([.28,.255,.21])
  d=R/'textures'/name;d.mkdir(exist_ok=True);rgb=np.broadcast_to(col,(N,N,3)).copy()+f[:,:,None]*.04;save(np.dstack([rgb,a]),d/(name+'_BC.png'));save(np.full_like(a,rough),d/(name+'_R.png'))
  maps={'base_color':str((d/(name+'_BC.png')).relative_to(R)),'roughness':str((d/(name+'_R.png')).relative_to(R))};decals.append(dict(id=name,region=reg['index'],role='decal',maps=maps,size_m=[1.3,1.6],alpha='straight RGBA; transparent border'))
  im=Image.open(d/(name+'_BC.png')).resize((384,384));checker=Image.new('RGBA',(384,384));dr=ImageDraw.Draw(checker)
  for yy in range(0,384,32):
   for xx in range(0,384,32):dr.rectangle((xx,yy,xx+31,yy+31),fill=(95,95,90,255) if (xx+yy)//32%2 else (145,145,135,255))
  checker.alpha_composite(im);checker.convert('RGB').save(R/'previews'/(name+'_alpha.jpg'))
manifest=dict(status='PRODUCED_PENDING_QA',regions=regions,materials=assets,decals=decals,props=[],texture_conventions={'base_color':'sRGB pigment, no baked illumination','normal':'DirectX, green flip false in Unreal; invert green in Blender','roughness':'linear','physical_repeat_m':2,'resolution':2048},camera={'height_cm':155,'fov':76,'grid_cm':400},image_generation='Tool available; not used. Procedural surfaces and modeled geometry.')
(R/'manifest.json').write_text(json.dumps(manifest,indent=2));(R/'reports/seams.json').write_text(json.dumps(checks,indent=2));print('TEXTURES_COMPLETE',len(assets),len(decals),flush=True)
