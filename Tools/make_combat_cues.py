import math,wave,struct,random
from pathlib import Path
out=Path('ArtReview/Audio');out.mkdir(parents=True,exist_ok=True)
r=random.Random(612);rate=22050
for name,duration in [('critical',.42),('divine',.85),('warlock',.85),('spell',.6),('grenade',.65)]:
 samples=[]
 for i in range(int(rate*duration)):
  t=i/rate;fade=(1-t/duration)**2;noise=r.uniform(-1,1)
  if name=='divine':v=sum(math.sin(2*math.pi*f*t)*math.exp(-t*3) for f in (660,880,1100))*.22
  elif name=='warlock':v=(math.sin(2*math.pi*(95*t-25*t*t))*.5+noise*.18)*(.6+.4*math.sin(t*44))
  elif name=='critical':v=noise*.4+math.sin(2*math.pi*(900*t-700*t*t))*.4
  elif name=='grenade':v=noise*.7+math.sin(2*math.pi*55*t)*.3
  else:v=math.sin(2*math.pi*(330*t+550*t*t))*.55+noise*.12
  samples.append(int(max(-.95,min(.95,v*fade))*30000))
 with wave.open(str(out/(name+'.wav')),'wb') as w:w.setparams((1,2,rate,0,'NONE','not compressed'));w.writeframes(struct.pack('<'+'h'*len(samples),*samples))
