"""Original deterministic combat-effect synthesis. No sampled recordings.

Use the bundled Python runtime (NumPy). Music, ambience and UI are untouched.
"""
import hashlib,html,json,math,wave
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
SOURCE=ROOT/'ArtSource/CombatAudio';WAV=SOURCE/'Wav'
REVIEW=ROOT/'ArtReview/AdventurePolish/Audio';QA=ROOT/'Saved/AdventurePolish'
for directory in (SOURCE,WAV,REVIEW,QA):directory.mkdir(parents=True,exist_ok=True)
RATE=44100;PEAK=.48
# Original console-inspired timbres, not samples or melodies from another game.
# PCM stays 16-bit for Unreal; per-voice quantization supplies the 8-bit color.
RETRO_8BIT={'hit','critical','miss','dodge','guard','defend','power_strike','cleave','backstab','fireball','chain_lightning','grenade','poison_blade'}

class Synth:
 def __init__(self,id,duration):
  self.id=id
  self.samples=np.zeros(round(duration*RATE),dtype=np.float64)
  self.rng=np.random.default_rng(int.from_bytes(hashlib.sha256(id.encode()).digest()[:8],'little'))
 def add(self,start,data):
  offset=round(start*RATE);count=min(len(data),len(self.samples)-offset)
  if count>0:self.samples[offset:offset+count]+=data[:count]
 @staticmethod
 def env(t,duration,attack=.003,decay=4,shape='decay'):
  u=t/duration;v=np.sin(np.pi*np.clip(u,0,1))**1.6 if shape=='swell' else np.exp(-decay*u)*(1-u)**.6
  return v*np.clip(t/max(attack,.0001),0,1)*np.clip((duration-t)/.018,0,1)
 def tone(self,start,duration,f0,f1=None,amp=.3,shape='sine',attack=.003,decay=4,envelope='decay',fm=0,mod=29):
  t=np.arange(round(duration*RATE))/RATE;f1=f0 if f1 is None else f1
  if max(f0,f1)>=RATE*.46:return  # Omit ultrasonic bell partials; do not alias them.
  # Tick-stepped pitch sweeps recreate a tiny console sound driver's updates.
  tick=np.floor(t*120)/120
  freq=f0*(max(.001,f1/f0)**(tick/duration));phase=2*np.pi*np.cumsum(freq)/RATE
  max_h=min(14,int((RATE*.43)/max(f0,f1)))
  if shape=='pulse':signal=sum((2/(k*np.pi))*math.sin(k*np.pi*.24)*np.cos(k*phase-k*np.pi*.24) for k in range(1,max_h+1))
  elif shape=='triangle':signal=sum(((-1)**((k-1)//2))*np.sin(k*phase)/k**2 for k in range(1,max_h+1,2))*.81
  else:signal=np.sin(phase+fm*np.sin(2*np.pi*mod*t))
  # Short sample tables / stepped pulse channels instead of clean modern sine beds.
  levels=31 if self.id in RETRO_8BIT else 255
  signal=np.round(signal*levels)/levels
  self.add(start,signal*amp*self.env(t,duration,attack,decay,envelope))
 def noise(self,start,duration,amp=.3,low=200,high=7000,attack=.002,decay=4,envelope='decay'):
  n=round(duration*RATE);t=np.arange(n)/RATE;freq=np.fft.rfftfreq(n,1/RATE)
  spectrum=np.fft.rfft(self.rng.normal(0,1,n))*(1-np.exp(-(freq/max(1,low))**4))*np.exp(-(freq/max(low+1,high))**6)
  signal=np.fft.irfft(spectrum,n);signal/=max(.00001,np.sqrt(np.mean(signal**2)))
  if self.id in RETRO_8BIT:
   # Clocked noise bursts are especially audible in slash, fire and dodge.
   signal=signal[(np.arange(n)//4)*4];signal=np.round(signal*31)/31
  self.add(start,signal*amp*self.env(t,duration,attack,decay,envelope))
 def bell(self,start,duration,frequency,amp=.3,bright=False):
  for i,ratio in enumerate((1,2.71,4.13,6.17) if bright else (1,2.01,3.98)):
   self.tone(start,duration/(1+i*.12),frequency*ratio,amp=amp/(1+i*2),decay=3+i*2)
 def metal(self,start,duration=.22,base=1150,amp=.3):
  for i,ratio in enumerate((1,1.41,2.37,3.83)):self.tone(start,duration/(1+i*.20),base*ratio,base*ratio*.965,amp/(1+i*1.7),decay=6+i)
  self.noise(start,.042,amp*.36,900,8000)
 def whoosh(self,start,duration=.16,amp=.23,low=550,high=6500):self.noise(start,duration,amp,low,high,.004,2,'swell')
 def thump(self,start,amp=.30,duration=.18):
  self.tone(start,duration,205,54,amp,'sine',decay=7);self.noise(start,duration*.5,amp*.45,80,800,decay=7)
 def echo(self,taps):
  dry=self.samples.copy()
  for delay,gain in taps:
   offset=round(delay*RATE);self.samples[offset:]+=dry[:-offset]*gain
 def finish(self):
  signal=self.samples.copy();signal-=np.mean(signal)
  # Compact early-console sample bandwidth with a short, dark echo on magic.
  clock=11025 if self.id in RETRO_8BIT else 22050
  positions=np.arange(len(signal))*clock/RATE
  sampled=signal[np.minimum((np.arange(math.ceil(len(signal)*clock/RATE))*RATE/clock).astype(int),len(signal)-1)]
  signal=np.interp(positions,np.arange(len(sampled)),sampled)
  ramp=round(.003*RATE);tail=round(.025*RATE)
  signal[:ramp]*=np.linspace(0,1,ramp);signal[-tail:]*=np.linspace(1,0,tail)
  rms=np.sqrt(np.mean(signal**2));peak=max(abs(signal));signal*=min(.105/max(rms,1e-8),PEAK/max(peak,1e-8))
  signal[0]=signal[-1]=0
  return np.rint(signal*32767).astype('<i2')

def sword(s,weight=1):
 s.whoosh(0,.15,.24);s.metal(.082,.26,1220,.27);s.thump(.088,.21*weight)
def arrow(s,start=0,weight=1):
 s.tone(start,.22,174,140,.30*weight,'triangle',decay=6);s.tone(start,.18,523,430,.085*weight,decay=7)
 s.whoosh(start+.025,.17,.17*weight,1500,8000);s.thump(start+.18,.20*weight,.13)

def make(id,duration):
 s=Synth(id,duration)
 if id=='hit':
  sword(s);s.tone(.018,.11,1900,380,.15,'pulse',decay=2)
 elif id=='critical':
  s.tone(0,.055,1046,2093,.25,'pulse',decay=1);s.whoosh(.035,.16,.30);s.metal(.095,.33,1620,.32);s.metal(.14,.24,2170,.15);s.thump(.10,.37,.20)
  s.tone(.16,.17,784,196,.22,'pulse',decay=3)
 elif id=='miss':s.whoosh(0,.22,.28,700,7300)
 elif id=='dodge':
  s.tone(0,.11,880,1760,.30,'pulse',decay=2);s.tone(.09,.12,1320,330,.22,'triangle',decay=3);s.whoosh(.025,.17,.18,1200,6200)
 elif id=='defend':
  for t,f in ((0,196),(.075,294),(.15,392)):s.tone(t,.17,f,amp=.30,shape='pulse',decay=4)
  s.metal(.15,.19,640,.15)
 elif id=='item':
  for t,f in ((0,660),(.075,880),(.15,1320)):s.tone(t,.18,f,amp=.27,shape='triangle',decay=3)
  s.bell(.18,.28,1760,.12)
 elif id=='mana':
  for t,f in ((0,440),(.065,659),(.13,988),(.195,1480)):s.tone(t,.20,f,f*1.08,.25,'pulse',decay=3)
  s.echo([(.06,.18)])
 elif id=='escape_fail':
  s.tone(0,.14,440,349,.30,'pulse',decay=2);s.tone(.12,.24,233,164,.30,'triangle',decay=3);s.thump(.13,.18)
 elif id=='escape':
  for t,f in ((0,784),(.07,659),(.14,523),(.21,392)):s.tone(t,.11,f,f*.8,.26,'pulse',decay=3)
  s.whoosh(.04,.29,.10,600,4000)
 elif id=='guard':s.thump(0,.3);s.metal(.018,.30,540,.32)
 elif id=='power_strike':s.whoosh(0,.19,.24,300,4000);s.thump(.135,.60,.25);s.metal(.14,.28,720,.34)
 elif id=='cleave':
  s.whoosh(0,.38,.32,350,6500)
  for t,f in ((.16,920),(.255,1320)):s.metal(t,.22,f,.27);s.thump(t,.18)
 elif id=='shield_wall':
  for t,f in ((0,390),(.125,460),(.26,550)):s.thump(t,.20);s.metal(t,.27,f,.25)
  s.tone(.29,.40,138,116,.18,'triangle')
 elif id=='fireball':
  # An original arcade contour: pulse drop, small secondary pop and hot fizz.
  s.tone(0,.19,1480,155,.64,'pulse',decay=2.1);s.tone(.106,.16,630,110,.28,'triangle',decay=3);s.thump(.030,.26,.17)
  for t,a in ((.008,.16),(.042,.10),(.083,.07)):s.noise(t,.055,a,850,4900,decay=7)
  s.noise(.10,.22,.055,250,1800,decay=7)
 elif id=='ice_lance':
  for t,f in ((0,1568),(.045,2093),(.09,3136)):s.tone(t,.12,f,f*.75,.20,'triangle',decay=2)
  s.whoosh(0,.14,.12,3000,11000)
  for t,f,a in ((.075,1640,.25),(.098,2387,.19),(.133,3497,.13),(.215,4493,.08)):s.bell(t,.53,f,a,True)
  s.noise(.075,.075,.18,4500,12500,decay=8)
 elif id=='chain_lightning':
  for t,f,a in ((0,3900,.37),(.145,2700,.29),(.305,4600,.25)):
   s.tone(t,.095,f,160,a,'pulse',decay=5);s.noise(t,.105,a*.65,1800,12500,decay=6);s.thump(t,.14,.085)
  s.noise(.31,.25,.07,2300,8000,decay=7)
 elif id=='power_shot':arrow(s,0,1.25)
 elif id=='multi_shot':
  for t in (0,.105,.225):arrow(s,t,.7)
 elif id=='hunters_mark':
  for t,f in ((0,392),(.08,523),(.17,784)):s.tone(t,.22,f,f*.94,.25,'triangle',decay=5)
  s.noise(.015,.045,.05,1000,4000)
 elif id=='heal':
  for t,f in ((0,523.25),(.14,659.25),(.28,783.99)):s.bell(t,.70,f,.25)
  s.noise(.03,.54,.022,900,5000,attack=.14,envelope='swell');s.echo([(.087,.16),(.159,.07)])
 elif id=='holy_smite':
  s.bell(0,.77,740,.30,True);s.bell(.028,.70,1110,.15);s.thump(.025,.19);s.noise(.02,.23,.08,1900,7500,decay=5);s.echo([(.069,.13)])
 elif id=='resurrection':
  for t,f in ((0,261.63),(.15,329.63),(.30,392),(.48,523.25)):
   s.tone(t,1.0,f,f*1.002,.16,attack=.055,decay=2);s.bell(t+.08,1.0,f*2,.10)
  s.whoosh(.10,.95,.024,1200,5500);s.echo([(.093,.12),(.183,.08)])
 elif id=='backstab':s.whoosh(0,.09,.21,1400,8500);s.metal(.043,.13,2260,.24);s.thump(.055,.32,.12)
 elif id=='poison_blade':
  sword(s,.6)
  for t,f in ((.16,275),(.235,410),(.33,190),(.42,335)):s.tone(t,.095,f,f*.42,.20,'sine',fm=1.3,mod=55)
  s.noise(.20,.37,.048,350,2400,decay=4)
 elif id=='grenade':
  s.metal(0,.08,2600,.09);s.whoosh(.035,.14,.11,450,3000);s.thump(.155,.62,.40);s.noise(.155,.48,.34,75,5700,decay=6);s.noise(.30,.45,.12,80,1300,decay=5)
 elif id=='holy_strike':sword(s,.95);s.bell(.085,.64,1046.5,.17);s.bell(.11,.57,1569.75,.07)
 elif id=='divine_shield':
  s.metal(0,.23,520,.20)
  for t,f in ((.07,587.33),(.15,880),(.24,1174.66)):s.bell(t,.63,f,.20)
  s.tone(.07,.62,293.66,amp=.10,attack=.04,decay=3)
 elif id=='lay_on_hands':
  for t,f in ((0,329.63),(.095,440),(.195,554.37)):s.tone(t,.79,f,amp=.21,attack=.025,decay=2.7);s.bell(t+.03,.67,f*2,.105)
  s.echo([(.079,.10),(.141,.06)])
 elif id=='shadow_bolt':
  for t,f in ((0,330),(.045,247),(.09,185),(.135,139)):s.tone(t,.12,f,f*.78,.24,'pulse',decay=2)
  s.tone(0,.44,116,54,.40,'triangle',fm=1.6,mod=38);s.tone(0,.37,123,59,.17,decay=3)
  s.noise(0,.23,.15,130,2300,envelope='swell');s.thump(.20,.40,.26);s.tone(.20,.32,230,67,.15,'pulse',decay=6)
 elif id=='curse':
  for f in (146.83,207.65):s.tone(0,.89,f,f*.83,.23,attack=.025,decay=2,fm=.6,mod=7)
  s.noise(.035,.72,.073,950,4100,envelope='swell');s.echo([(.097,-.13),(.191,.08)])
 elif id=='soul_drain':
  s.tone(0,.71,82,375,.29,'sine',attack=.04,decay=1.6,fm=1.5,mod=19);s.noise(0,.64,.095,400,4300,envelope='swell')
  for t in (.16,.39):s.thump(t,.18,.16)
  s.tone(.63,.42,392,196,.22,'triangle',decay=3);s.echo([(.089,.09)])
 elif id=='arcane_bolt':s.tone(0,.28,440,1760,.25,'triangle',decay=1.3);s.bell(.20,.44,1320,.19);s.noise(.21,.12,.07,2500,8500)
 elif id=='status_bind':
  for t,f in ((0,415),(.09,493),(.21,349)):s.tone(t,.23,f,f*.985,.23,'triangle',decay=5);s.metal(t,.10,f*2,.085)
  s.noise(.22,.19,.038,1000,3500)
 else:raise ValueError(id)
 return s.finish()

SPECS=[
 ('hit',.42,'Metallic slash','Air cut, inharmonic metal contact and compact impact.','Physical'),
 ('critical',.50,'Critical slash','Brighter double contact and deeper punch.','Physical'),
 ('miss',.26,'Missed swing','A swish without an impact.','Physical'),
 ('dodge',.30,'Dodge','Fast rising pulse and falling zip: an evasive step.','Physical'),
 ('defend',.42,'Defend','Three ascending square-channel notes lock the guard stance.','Physical'),
 ('item',.52,'Use item','Bright compact triangle arpeggio and restorative sparkle.','Physical'),
 ('mana',.53,'Restore mana','Rising violet pulse arpeggio with a tiny echo.','Physical'),
 ('escape',.39,'Escape','Descending quick-step pulses and a retreating swish.','Physical'),
 ('escape_fail',.42,'Escape blocked','Interrupted pulse and a descending denial thud.','Physical'),
 ('guard',.39,'Brace / block','One short low shield clang.','Physical'),
 ('power_strike',.54,'Power Strike','Heavy blade arrival and a weighted strike.','Warrior'),
 ('cleave',.62,'Cleave','Wide sweep with two cutting contacts.','Warrior'),
 ('shield_wall',.79,'Shield Wall','Three shields lock together.','Warrior'),
 ('fireball',.39,'Fireball','Punchy original retro pulse drop, bass pop and hot fizz.','Mage'),
 ('ice_lance',.86,'Ice Lance','Brittle glass attack, ice shards and cold air.','Mage'),
 ('chain_lightning',.68,'Chain Lightning','Three separated electrical cracks jump across targets.','Mage'),
 ('power_shot',.46,'Power Shot','Bowstring twang, arrow rush and impact.','Ranger'),
 ('multi_shot',.70,'Multi-Shot','Three arrow releases in an uneven volley.','Ranger'),
 ('hunters_mark',.51,"Hunter's Mark",'A dry three-note tracking signal.','Ranger'),
 ('heal',1.10,'Heal','Soft ascending chimes with a warm resolving tail.','Cleric'),
 ('holy_smite',.94,'Holy Smite','Sanctified bell attack with a solid impact.','Cleric'),
 ('resurrection',1.66,'Resurrection','A longer rising chord opens into a luminous return.','Cleric'),
 ('backstab',.31,'Backstab','Short knife scrape and sharp close impact.','Rogue'),
 ('poison_blade',.70,'Poison Blade','Blade hit followed by bubbling toxin.','Rogue'),
 ('grenade',.88,'Small Grenade','Pin tick, short throw and broad low explosion.','Rogue'),
 ('holy_strike',.86,'Holy Strike','Metal slash with a radiant overtone.','Paladin'),
 ('divine_shield',1.01,'Divine Shield','Armor clang blooms into a protective shimmer.','Paladin'),
 ('lay_on_hands',1.14,'Lay on Hands','Lower, fuller restoration than the cleric heal.','Paladin'),
 ('shadow_bolt',.73,'Shadow Bolt','Detuned dark launch collapses into a low impact.','Warlock'),
 ('curse',1.12,'Curse','Wavering dissonant whisper with an uneasy falling tone.','Warlock'),
 ('soul_drain',1.19,'Soul Drain','Inward rising pull, two pulses and falling release.','Warlock'),
 ('arcane_bolt',.76,'Arcane bolt fallback','Rising crystalline projectile for future untyped magic.','Fallback'),
 ('status_bind',.59,'Status fallback','Three dry interlocking magical taps.','Fallback')]

def metrics(pcm):
 x=pcm.astype(np.float64)/32768.;spectrum=np.abs(np.fft.rfft(x))**2;freq=np.fft.rfftfreq(len(x),1/RATE)
 return dict(durationSeconds=round(len(x)/RATE,5),sampleRate=RATE,channels=1,bitsPerSample=16,
  peakDbFS=round(20*np.log10(max(max(abs(x)),1e-10)),3),rmsDbFS=round(20*np.log10(max(np.sqrt(np.mean(x*x)),1e-10)),3),
  dcOffset=round(float(np.mean(x)),7),clippedSamples=int(np.sum(abs(x)>=.999)),firstSample=int(pcm[0]),lastSample=int(pcm[-1]),
  spectralCentroidHz=round(float(np.sum(freq*spectrum)/max(1e-15,np.sum(spectrum))),1),pcmBytes=pcm.nbytes)
def write_wave(path,pcm):
 with wave.open(str(path),'wb') as file:file.setparams((1,2,RATE,0,'NONE','not compressed'));file.writeframes(pcm.tobytes())

def main():
 campaign=json.loads((ROOT/'Content/Game/Data/campaign.json').read_text());all_pcm={};records=[]
 for id,duration,title,description,group in SPECS:
  pcm=make(id,duration);path=WAV/(id+'.wav');write_wave(path,pcm);all_pcm[id]=pcm;data=metrics(pcm)
  assert data['peakDbFS']<=-6 and data['rmsDbFS']>-32 and data['clippedSamples']==0 and data['firstSample']==data['lastSample']==0,(id,data)
  records.append(dict(id=id,title=title,description=description,group=group,source=str(path.relative_to(ROOT)).replace('\\','/'),asset=f'/Game/Game/Audio/A_{id}.A_{id}',sha256=hashlib.sha256(path.read_bytes()).hexdigest(),fileBytes=path.stat().st_size,**data))
 assert len({r['sha256'] for r in records})==len(SPECS)
 by_name={title:id for id,_,title,_,_ in SPECS};routing=[dict(className=c['name'],skill=name,cue=by_name[name]) for c in campaign['classes'] for name in c['skills']]
 assert len(routing)==21 and len({r['cue'] for r in routing})==21
 montage_ids=['hit','fireball','ice_lance','shadow_bolt','defend','dodge','critical','guard','miss','item','mana','escape','escape_fail']
 reel=np.concatenate([np.concatenate([all_pcm[id],np.zeros(round(RATE*.20),dtype='<i2')]) for id in montage_ids]);write_wave(REVIEW/'combat_comparison.wav',reel)
 manifest=dict(version=3,originalSynthesis=True,style='Original 8-bit pulse/noise and 16-bit sample-table console JRPG effects; 16-bit PCM container',seedPolicy='SHA256 of cue ID; reproducible NumPy synthesis',cueCount=len(SPECS),decodedPcmBytes=sum(r['pcmBytes'] for r in records),maxPeakDbFS=max(r['peakDbFS'] for r in records),runtimeVolume='Existing MasterVolume * EffectsVolume',cues=records,skillRouting=routing,montage=dict(source=str((REVIEW/'combat_comparison.wav').relative_to(ROOT)).replace('\\','/'),cueOrder=montage_ids))
 (SOURCE/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
 (QA/'combat_audio_source_qa.json').write_text(json.dumps(dict(status='PASS',cueCount=len(SPECS),skillCount=21,classCount=7,allOriginal=True,uniqueWaveforms=len(SPECS),clippedSamples=0,maxPeakDbFS=manifest['maxPeakDbFS'],decodedPcmBytes=manifest['decodedPcmBytes'],cues=records),indent=2)+'\n')
 cards=[]
 for r in records:
  href='../../../'+r['source'];cards.append(f'<article data-group="{r["group"]}"><small>{html.escape(r["group"])}</small><h2>{html.escape(r["title"])}</h2><p>{html.escape(r["description"])}</p><audio controls preload="none" src="{href}"></audio><footer>{r["durationSeconds"]:.2f}s · {r["peakDbFS"]:.1f} dBFS peak</footer></article>')
 doc='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Combat sound library</title><style>
body{margin:0;background:#121419;color:#eee9de;font:16px/1.55 system-ui}main{max-width:1220px;margin:auto;padding:40px 24px}h1{font-size:36px;margin:0 0 8px}h2{font-size:20px;margin:5px 0}p{color:#bfc2c9}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(290px,1fr));gap:16px}article,.compare{background:#20242c;border:1px solid #454036;border-radius:12px;padding:20px}small{color:#d9b06d}audio{width:100%;height:38px}footer{font-size:12px;color:#969ba5;margin-top:8px}.compare{margin:26px 0}select{padding:10px;background:#242a34;color:#eee;border:1px solid #6d6557;border-radius:6px;margin-bottom:18px}article[hidden]{display:none}</style><main>
<h1>Combat sound library</h1><p>Original 8-bit pulse/noise and 16-bit sample-table effects for all seven classes. Tick-stepped sweeps, clocked noise, short arpeggios and compact echoes evoke console JRPG combat. Metal, fire, ice and shadow have separate identities, as do defending, dodging and critical hits. All files use a 16-bit PCM container for game playback.</p>
<section class="compare"><h2>Quick comparison</h2><p>Metal slash → Fireball → Ice Lance → Shadow Bolt → Defend → Dodge → Critical → Block → Miss → Item → Mana → Escape → Escape blocked</p><audio controls preload="none" src="combat_comparison.wav"></audio><p>Every effect leaves at least 6 dB of headroom and follows the game’s Master and Effects controls.</p></section><label for="group">Show </label><select id="group"><option>All</option>'''+''.join('<option>'+g+'</option>' for g in ['Physical','Warrior','Mage','Ranger','Cleric','Rogue','Paladin','Warlock','Fallback'])+'''</select><section class="grid">'''+''.join(cards)+'''</section><p>33 effects · 21 distinct active-skill sounds · no sampled recordings · music unchanged</p></main><script>
document.querySelectorAll('audio').forEach(a=>a.addEventListener('play',()=>document.querySelectorAll('audio').forEach(b=>{if(a!==b)b.pause()})));document.querySelector('#group').addEventListener('change',e=>document.querySelectorAll('article').forEach(a=>a.hidden=e.target.value!=='All'&&a.dataset.group!==e.target.value));</script></html>'''
 (REVIEW/'REVIEW.html').write_text(doc,encoding='utf-8')
 print('COMBAT_AUDIO_SOURCE_READY cues=33 skills=21 classes=7 peakDbFS='+str(manifest['maxPeakDbFS'])+' decodedBytes='+str(manifest['decodedPcmBytes']))

if __name__=='__main__':main()
