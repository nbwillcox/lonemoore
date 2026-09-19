"""Original deterministic retro development loops and compact cues."""
import math,wave,struct,pathlib,random
root=pathlib.Path(__file__).resolve().parents[1]/'ArtReview/Audio';root.mkdir(parents=True,exist_ok=True)
rate=22050
def save(name,data):
    with wave.open(str(root/(name+'.wav')),'wb') as w:w.setparams((1,2,rate,0,'NONE','not compressed'));w.writeframes(b''.join(struct.pack('<h',int(max(-.95,min(.95,x))*25000)) for x in data))
def freq(note):return 440*2**((note-69)/12)
melodies={
 'town':[62,69,65,64,62,57,60,64,65,69,67,64,62,60,57,62],
 'dungeon':[50,57,53,50,56,53,48,45,50,53,57,56,53,50,48,45],
 'combat':[62,62,65,69,68,65,62,60,62,65,67,69,72,69,65,60],
 'boss':[50,56,57,62,65,63,60,56,50,57,62,68,65,62,56,50],
 'ending':[62,65,69,74,72,69,67,65,64,67,71,74,72,69,65,62]}
for name,notes in melodies.items():
    beat=.25 if name in ['combat','boss'] else .5
    samples=[]
    for n in range(int(rate*beat*len(notes)*4)):
        t=n/rate;k=int(t/beat);phase=(t%beat)/beat;note=notes[k%len(notes)];env=min(1,phase*25)*(1-phase)*.28
        tri=lambda x:2*abs(2*(x-math.floor(x+.5)))-1
        lead=tri(t*freq(note))*env;bass=math.sin(t*freq(notes[(k//4*4)%len(notes)]-24)*2*math.pi)*.16
        pulse=(1 if math.sin(t*freq(note-12)*2*math.pi)>0 else -1)*env*.12
        samples.append(lead+bass+pulse)
    save(name,samples)
rng=random.Random(713)
for name,length,pitch in [('ui',.1,880),('step',.14,110),('hit',.2,190),('interact',.3,660)]:
    samples=[]
    for n in range(int(rate*length)):
        t=n/rate;fade=(1-t/length)**2;s=math.sin(2*math.pi*pitch*t*(1-t/length*.4))*.35
        if name in ['hit','step']:s=s*.4+(rng.random()*2-1)*.32
        samples.append(s*fade)
    save(name,samples)
print('Original development audio written:',len(melodies)+4)
