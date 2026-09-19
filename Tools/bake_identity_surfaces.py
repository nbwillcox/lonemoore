"""Native 2K periodic physical surfaces. No lighting or photographs in albedo."""
from pathlib import Path
import json, math, sys
import numpy as np
from PIL import Image, ImageDraw
R=Path(r'J:\Lonemoore_Regional_Identity');M=json.loads((R/'manifest.json').read_text());N=2048
y,x=np.mgrid[:N,:N].astype(np.float32)/N
selection=sys.argv[1:]
def smooth(v,a=0,b=1):
 t=np.clip((v-a)/(b-a),0,1);return t*t*(3-2*t)
def noise(nx,ny,seed,xx=x,yy=y):
 rng=np.random.default_rng(seed);g=rng.random((ny,nx),dtype=np.float32)
 X=(xx%1)*nx;Y=(yy%1)*ny;ix=np.floor(X).astype(np.int32);iy=np.floor(Y).astype(np.int32);fx=X-ix;fy=Y-iy
 fx=fx**3*(fx*(fx*6-15)+10);fy=fy**3*(fy*(fy*6-15)+10)
 return (g[iy%ny,ix%nx]*(1-fx)+g[iy%ny,(ix+1)%nx]*fx)*(1-fy)+(g[(iy+1)%ny,ix%nx]*(1-fx)+g[(iy+1)%ny,(ix+1)%nx]*fx)*fy
def cells(nx,ny,seed,warp=0):
 rng=np.random.default_rng(seed);points=rng.random((ny,nx,2),dtype=np.float32)*.78+.11
 X=x*nx+warp*(noise(6,6,seed+21)-.5);Y=y*ny+warp*(noise(6,6,seed+23)-.5)
 ix=np.floor(X).astype(np.int32);iy=np.floor(Y).astype(np.int32);f1=np.ones_like(x)*100;f2=f1.copy();ids=np.zeros_like(ix)
 for oy in [-2,-1,0,1,2]:
  for ox in [-2,-1,0,1,2]:
   px=points[(iy+oy)%ny,(ix+ox)%nx,0]+ix+ox;py=points[(iy+oy)%ny,(ix+ox)%nx,1]+iy+oy
   d=(X-px)**2+(Y-py)**2;closer=d<f1;f2=np.where(closer,f1,np.minimum(f2,d));ids=np.where(closer,((iy+oy)%ny)*nx+(ix+ox)%nx,ids);f1=np.minimum(f1,d)
 vals=rng.random(nx*ny,dtype=np.float32)[ids];edge=np.sqrt(f2)-np.sqrt(f1)
 return edge,vals,np.sqrt(f1)
def grid(nx,ny,seed,stagger=.5,warp=.002):
 X=x+warp*(noise(29,29,seed)-.5);Y=y+warp*(noise(23,31,seed+1)-.5)
 row=np.floor(Y*ny).astype(np.int32);xx=X*nx+(row%2)*stagger
 col=np.floor(xx).astype(np.int32);dx=np.minimum(xx%1,1-xx%1)/nx;dy=np.minimum(Y*ny%1,1-Y*ny%1)/ny
 ids=(row%ny)*nx+col%nx;vals=np.random.default_rng(seed).random(nx*ny,dtype=np.float32)[ids]
 return np.minimum(dx,dy),vals
report=[]
for n,a in enumerate(M['materials']):
 if selection and a['id'].split('_')[0] not in selection:continue
 print('BAKING_IDENTITY',a['id'],flush=True);seed=188+n*77;k=a['kind'];c=np.array(a['color'],np.float32)
 macro=noise(5,5,seed);meso=noise(23,23,seed+2);grain=noise(137,137,seed+3);fine=noise(701,701,seed+4)
 f=macro*.52+meso*.27+grain*.14+fine*.07
 h=(grain-.5)*.00045+(fine-.5)*.00012
 bc=np.ones((N,N,3),np.float32)*c*(.78+f[...,None]*.44);rough=.78+grain*.13;metal=None;em=None
 if k in ['lime','plaster','scorch','chalk','cutstone']:
  pits=smooth(noise(211,211,seed+19),.75,.92);h+=(meso-.5)*.0012-pits*.00065
  if k=='lime':
   loss=smooth(macro+noise(17,17,seed+20)*.24+noise(127,127,seed+24)*.10,.67,.72);edge,ids=grid(3,5,seed);block=smooth(edge,.002,.012)
   h+=(1-loss)*.004+loss*block*.005
   bare=np.array([.33,.30,.24])*(.80+ids[...,None]*.38);bare=bare*block[...,None]+np.array([.26,.24,.20])*(1-block[...,None]);bc=bc*(1-loss[...,None])+bare*loss[...,None]
  if k=='cutstone':
   chisel=smooth(np.sin(x*math.tau*97+(meso-.5)*6),.86,1);h-=chisel*.0006;bc*=1-chisel[...,None]*.035
  if k=='scorch':
   burn=smooth(macro,.40,.74);bc*=1-burn[...,None]*.46;h+=meso*.002
 elif k in ['rubble','brokenflags','rock','bedrock','mineral','basalt','basaltroof','lava','obsidian','shale','earth','roots']:
  nx,ny={'rubble':(7,6),'brokenflags':(4,4),'rock':(4,5),'bedrock':(5,4),'mineral':(6,5),'basalt':(6,2),'basaltroof':(5,3),'lava':(4,4),'obsidian':(3,4),'shale':(5,10),'earth':(29,29),'roots':(17,17)}[k]
  edge,ids,dist=cells(nx,ny,seed,warp=.09 if k in ['earth','roots','shale'] else .025)
  chips=noise(97,97,seed+34)*.013+noise(251,251,seed+35)*.006
  joint=smooth(edge-chips,.008,.06 if k not in ['lava','basalt','obsidian'] else .035)
  depth={'rubble':.011,'brokenflags':.008,'rock':.014,'bedrock':.007,'mineral':.010,'basalt':.015,'basaltroof':.018,'lava':.007,'obsidian':.008,'shale':.007,'earth':.002,'roots':.003}[k]
  h+=joint*(depth+ids*depth*.5)+(meso-.5)*depth*.22
  # Per-fragment mineral composition, not painted shadows.
  bc*=.65+ids[...,None]*.65
  mortar=np.array([.16,.15,.13]) if k in ['rubble','brokenflags'] else c*.55
  bc=bc*joint[...,None]+mortar*(1-joint[...,None]);rough=.76+grain*.18
  if k in ['rock','basalt','basaltroof','bedrock','shale']:
   lam=noise(45,8 if k=='basalt' else 71,seed+41);h+=(lam-.5)*.0018*joint
   flecks=smooth(grain,.73,.88);bc+=flecks[...,None]*np.array([.028,.031,.025])
  if k in ['lava','basaltroof']:
   # Only some cracks retain heat; most of the material is cold stone.
   em=(1-smooth(edge,.003,.017))*smooth(macro,.55,.77)*(.6+meso*.4)
  if k=='obsidian':
   rough=.52+meso*.24;h+=(noise(15,61,seed+55)-.5)*.001*joint
  if k=='mineral':
   veins=1-smooth(edge,.018,.055);bc=bc*(1-veins[...,None])+np.array([.51,.55,.46])*veins[...,None];h+=veins*.0008
  if k in ['earth','roots']:
   peb=smooth(ids,.53,.80)*smooth(edge,.018,.08);h=meso*.001+peb*(.003+ids*.003)+fine*.00015
   bc=c*(.78+f[...,None]*.48)+peb[...,None]*np.array([.055,.055,.039]);rough=.88+fine*.08
   if k=='roots':
    root=np.zeros_like(x)
    for off in [.17,.52,.81]:
     path=(x-off+.028*(noise(5,11,seed+50)-.5))%1;line=np.minimum(path,1-path);root=np.maximum(root,1-smooth(line,.004,.013))
    h+=root*.009;bc=bc*(1-root[...,None])+(c*.68)*root[...,None]
 elif k in ['largeblocks','smallbrick','monolith','seal']:
  nx,ny={'largeblocks':(2,3),'smallbrick':(8,12),'monolith':(2,1),'seal':(4,2)}[k]
  edge,ids=grid(nx,ny,seed,stagger=.5 if k in ['smallbrick','largeblocks'] else 0)
  chips=noise(79,79,seed+25)*.003;face=smooth(edge-chips,.002,.014)
  h+=face*(.009+ids*.003);bc*=.72+ids[...,None]*.48
  bc=bc*face[...,None]+c*.58*(1-face[...,None])
  if k in ['monolith','seal']:
   u=(x*nx)%1;v=(y*ny)%1
   border=(1-smooth(np.abs(u-.10),.006,.014))+(1-smooth(np.abs(u-.90),.006,.014))
   if k=='seal':border+=1-smooth(np.abs(v-.20),.006,.015)
   border=np.clip(border,0,1)*face;h-=border*.004;bc*=1-border[...,None]*.18
 elif k in ['marble','darkmarble','checker','inlay']:
  warp=noise(7,7,seed+20);v=noise(11,8,seed+21,xx=x+(warp-.5)*.13,yy=y+(warp-.5)*.08)
  vein=1-smooth(np.abs(v-.47),.008,.036);vein2=1-smooth(np.abs(v-.54),.003,.009)
  bc*=1-vein[...,None]*.30-vein2[...,None]*.13;rough=.64+meso*.17;h-=vein*.00022
  if k in ['checker','inlay']:
   edge,ids=grid(4,4,seed,stagger=0,warp=.0005);face=smooth(edge,.001,.0045);h+=face*.003
   if k=='checker':
    alt=((np.floor(x*4)+np.floor(y*4))%2);bc=bc*(1-alt[...,None])+bc*np.array([.40,.43,.47])*alt[...,None]
   else:
    stripe=(np.minimum(x% .5,.5-x% .5)<.035)|(np.minimum(y% .5,.5-y% .5)<.035)
    bc=np.where(stripe[...,None],bc*np.array([.51,.43,.29]),bc)
   bc=bc*face[...,None]+c*.50*(1-face[...,None])
 elif k=='herringbone':
  X=x*8;Y=y*8;ix=np.floor(X).astype(int);iy=np.floor(Y).astype(int);d=(ix-iy)%4;orient=d<2
  # Periodic 2:1 domino paving. Brick identity follows the whole brick,
  # including the pieces that cross the texture's boundaries.
  ox=ix-np.where(orient&(d==1),1,0);oy=iy-np.where((~orient)&(d==2),1,0)
  ux=X-ox;vy=Y-oy;ww=np.where(orient,2,1);hh=np.where(orient,1,2)
  dist=np.minimum(np.minimum(ux,ww-ux),np.minimum(vy,hh-vy))
  face=smooth(dist,.015,.06);ids=(np.sin(((ox%8)*71+(oy%8)*139+orient*29)*213.17)*713.31)%1
  h+=face*.006;bc*=.76+ids[...,None]*.40;bc=bc*face[...,None]+np.array([.18,.16,.13])*(1-face[...,None])
 elif k=='wood':
  w=noise(6,6,seed+15);grainlong=noise(163,5,seed+17,xx=x+(w-.5)*.009,yy=y)
  edge,ids=grid(6,1,seed,stagger=0,warp=.0009);face=smooth(edge,.0015,.004)
  fibers=(1-smooth(grainlong,.19,.29));h+=face*.004-fibers*.00065
  bc*=.68+ids[...,None]*.50;bc*=1-fibers[...,None]*.26;bc=bc*face[...,None]+c*.43*(1-face[...,None]);rough=.79+grain*.15
 elif k in ['iron','forge']:
  oxide=smooth(macro+meso*.28,.53,.78);metal=(1-oxide)*.88;rough=.56+oxide*.26+grain*.06
  bc=bc*(1-oxide[...,None])+np.array([.29,.155,.07])*oxide[...,None];h+=grain*.0006-oxide*.00045
  if k=='forge':
   edge,ids=grid(2,2,seed,stagger=0,warp=.0002);face=smooth(edge,.001,.006);h+=face*.006;bc*=.85+ids[...,None]*.24;bc=bc*face[...,None]+c*.55*(1-face[...,None])
   ux=(x*2)%1;vy=(y*2)%1
   for xx in [.09,.91]:
    for yy in [.09,.91]:
     rivet=1-smooth(np.sqrt((ux-xx)**2+(vy-yy)**2),.015,.028);h+=rivet*.002;bc+=rivet[...,None]*.018
   grooves=1-smooth(np.abs(((x+y)*32)%1-.5),.05,.14);h+=grooves*.00065*face;bc*=.97+grooves[...,None]*.025
 bc=np.clip(bc,0,1);rough=np.clip(rough,.42,.98)
 dx=(np.roll(h,-1,axis=1)-np.roll(h,1,axis=1))/(4/N);dy=(np.roll(h,-1,axis=0)-np.roll(h,1,axis=0))/(4/N)
 normal=np.stack([-dx,-dy,np.ones_like(dx)],axis=-1);normal/=np.linalg.norm(normal,axis=-1,keepdims=True)
 out=R/'textures'/a['id'];out.mkdir(parents=True,exist_ok=True)
 maps={'base_color':('BC',bc),'normal':('N',normal*.5+.5),'roughness':('R',rough)}
 if metal is not None:maps['metallic']=('M',metal)
 if em is not None:maps['emission']=('E',em)
 a['maps']={}
 for role,(suffix,data) in maps.items():
  path=out/(a['id']+'_'+suffix+'.png');im=Image.fromarray((np.clip(data,0,1)*255+.5).astype('uint8'));im.save(path);a['maps'][role]=str(path.relative_to(R)).replace('\\','/')
  deltas=[]
  for axis in [0,1]:
   seam=float(np.mean(np.abs(np.take(data,0,axis)-np.take(data,-1,axis))));inner=float(np.mean(np.abs(np.diff(data,axis=axis))));deltas.append(dict(axis=axis,edge_delta=seam,interior_delta=inner,ratio=seam/max(inner,.00001)))
  report.append(dict(id=a['id'],map=role,checks=deltas))
 np.save(R/'sources'/(a['id']+'_Height.npy'),h.astype(np.float32))
 # Unlit and neutral shaded 3x3 repeats. The latter is explicitly a preview only.
 unlit=Image.fromarray((bc*255).astype('uint8')).resize((384,384),Image.Resampling.LANCZOS)
 light=np.array([-.30,-.42,.855]);shade=np.maximum(np.sum(normal*light,axis=-1),0)*.62+.38
 shaded=Image.fromarray(np.clip(bc*shade[...,None]*255,0,255).astype('uint8')).resize((384,384),Image.Resampling.LANCZOS)
 board=Image.new('RGB',(2304,1196),(22,23,24));d=ImageDraw.Draw(board);d.text((16,12),a['id']+' | 2 m repeat | LEFT: unlit albedo | RIGHT: neutral normal preview',fill='white')
 for yy in range(3):
  for xx in range(3):board.paste(unlit,(xx*384,44+yy*384));board.paste(shaded,(1152+xx*384,44+yy*384))
 board.save(R/'previews'/(a['id']+'_3x3.jpg'),quality=91)
 (R/'manifest.json').write_text(json.dumps(M,indent=2))
(R/'reports/surface_seams.json').write_text(json.dumps(report,indent=2))
for reg in M['regions']:
 assets=[a for a in M['materials'] if a['region']==reg['index']]
 if any('maps' not in a for a in assets):continue
 board=Image.new('RGB',(1600,440),(18,20,22));d=ImageDraw.Draw(board)
 for i,a in enumerate(assets[:4]):
  im=Image.open(R/a['maps']['base_color']).resize((400,400),Image.Resampling.LANCZOS);board.paste(im,(i*400,40));d.text((i*400+12,12),a['role']+' / '+a['id'].split('_')[1],fill='white')
 board.save(R/'previews'/(reg['code']+'_Surfaces.jpg'),quality=94)
print('IDENTITY_SURFACES_COMPLETE',flush=True)
