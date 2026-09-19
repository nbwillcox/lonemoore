"""Original restrained mono room loops, authored procedurally; no source audio replaced."""
import wave, struct, math, random, pathlib
root=pathlib.Path(__file__).resolve().parents[1];dest=root/'ArtReview/IntegrityKit/Audio';dest.mkdir(parents=True,exist_ok=True)
rate=22050;duration=12
for kind in ('drips','stone','infernal'):
    rng=random.Random(105);samples=[];low=0
    drops=[(.9,950),(3.7,1150),(7.1,800),(10.2,1300)]
    for i in range(rate*duration):
        t=i/rate;low=.985*low+.015*rng.uniform(-1,1)
        v=low*.10+math.sin(2*math.pi*(55 if kind=='infernal' else 80)*t)*(.012 if kind=='infernal' else .002)
        if kind=='drips':
            for when,hz in drops:
                d=t-when
                if 0<d<.7:v+=.17*math.sin(2*math.pi*(hz*d-150*d*d))*math.exp(-d*15)
        if kind=='stone':v+=.005*math.sin(2*math.pi*170*t)*(.5+.5*math.sin(t*.4))
        fade=min(1,t/.15,(duration-t)/.15);samples.append(struct.pack('<h',int(max(-1,min(1,v*fade))*32767)))
    with wave.open(str(dest/('ambient_'+kind+'.wav')),'wb') as f:f.setnchannels(1);f.setsampwidth(2);f.setframerate(rate);f.writeframes(b''.join(samples))
