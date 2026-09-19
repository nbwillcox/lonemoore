"""Validate decoded WAVs, reproducibility, all-class routing and review links."""
import hashlib,importlib.util,json,re,wave
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('combat_synthesis',ROOT/'Tools/make_combat_audio_library.py')
synthesis=importlib.util.module_from_spec(spec);spec.loader.exec_module(synthesis)
manifest=json.loads((ROOT/'ArtSource/CombatAudio/manifest.json').read_text())
code=(ROOT/'Source/DungeonCrawler/DungeonCombatAudio.cpp').read_text()
mapping=dict(re.findall(r'\{TEXT\("([^"]+)"\),TEXT\("([^"]+)"\)\}',code))
failures=[];records=[]
for cue in manifest['cues']:
 path=ROOT/cue['source']
 with wave.open(str(path),'rb') as file:
  fmt=(file.getnchannels(),file.getsampwidth(),file.getframerate());data=file.readframes(file.getnframes())
 pcm=np.frombuffer(data,dtype='<i2');metrics=synthesis.metrics(pcm)
 deterministic=synthesis.make(cue['id'],cue['durationSeconds']).tobytes()==data
 if fmt!=(1,2,44100) or not deterministic:failures.append([cue['id'],'format or deterministic PCM'])
 if hashlib.sha256(path.read_bytes()).hexdigest()!=cue['sha256']:failures.append([cue['id'],'manifest hash'])
 if metrics['peakDbFS']>-6 or metrics['rmsDbFS']<=-32 or metrics['clippedSamples'] or abs(metrics['dcOffset'])>.003 or metrics['firstSample'] or metrics['lastSample']:
  failures.append([cue['id'],'waveform headroom or edge quality'])
 records.append(dict(id=cue['id'],deterministic=deterministic,**metrics))
for row in manifest['skillRouting']:
 if mapping.get(row['skill'])!=row['cue']:failures.append([row['skill'],'native cue routing differs'])
review=ROOT/'ArtReview/AdventurePolish/Audio/REVIEW.html'
for source in re.findall(r'<audio[^>]+src="([^"]+)"',review.read_text(encoding='utf-8')):
 if not (review.parent/source).resolve().exists():failures.append([source,'broken listening link'])
assert len(records)==manifest['cueCount']==33 and len(manifest['skillRouting'])==21
assert len({r['sha256'] for r in manifest['cues']})==len(records)
assert {'hit','critical','miss','guard','defend','dodge','item','mana','escape','escape_fail'}.issubset({r['id'] for r in records})
report=dict(status='PASS' if not failures else 'FAIL',failures=failures,cueCount=len(records),skills=21,classes=7,
 distinctSkillCues=len({r['cue'] for r in manifest['skillRouting']}),deterministicWaveforms=sum(r['deterministic'] for r in records),
 decodedPcmBytes=sum(r['pcmBytes'] for r in records),maxPeakDbFS=max(r['peakDbFS'] for r in records),
 minRmsDbFS=min(r['rmsDbFS'] for r in records),maxRmsDbFS=max(r['rmsDbFS'] for r in records),
 nativeRoutingChecked=True,reviewLinksChecked=True,cues=records,
 limitations=['Waveform metrics and routing do not constitute a subjective in-game listening test.','Native cue-action and imported-asset suites must run after Unreal import.'])
(ROOT/'Saved/AdventurePolish/combat_audio_validation.json').write_text(json.dumps(report,indent=2)+'\n')
assert not failures,failures
print('COMBAT_AUDIO_VALIDATION_PASS cues='+str(len(records))+' skills=21 classes=7 deterministic='+str(report['deterministicWaveforms'])+' peakDbFS='+str(report['maxPeakDbFS'])+' PCMbytes='+str(report['decodedPcmBytes']))
