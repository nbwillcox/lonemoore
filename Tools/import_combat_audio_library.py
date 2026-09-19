"""Selective import of manifest-listed combat WAVs; full editor only.

No music/UI/ambience changes. Assets use inline PCM for compact, immediate SFX.
The editor exits after ordinary ticks drain asynchronous audio work.
"""
import json,hashlib,time
from pathlib import Path
import unreal as u

ROOT=Path(u.Paths.project_dir()).resolve()
MANIFEST=json.loads((ROOT/'ArtSource/CombatAudio/manifest.json').read_text())
OUT=ROOT/'Saved/AdventurePolish/combat_audio_import.json';OUT.parent.mkdir(parents=True,exist_ok=True)
report=dict(status='RUNNING',expected=MANIFEST['cueCount'],imported=[],failures=0)
def save_report():OUT.write_text(json.dumps(report,indent=2)+'\n')
save_report()
try:
 assert len(MANIFEST['cues'])==MANIFEST['cueCount']
 for cue in MANIFEST['cues']:
  source=ROOT/cue['source'];assert hashlib.sha256(source.read_bytes()).hexdigest()==cue['sha256'],source
  task=u.AssetImportTask();task.filename=str(source);task.destination_path='/Game/Game/Audio'
  task.destination_name='A_'+cue['id'];task.automated=True;task.replace_existing=True;task.save=False
  u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
  sound=u.load_asset(cue['asset']);assert isinstance(sound,u.SoundWave),cue['asset']
  sound.set_editor_property('looping',False);sound.set_editor_property('volume',1.0);sound.set_editor_property('pitch',1.0)
  sound.set_sound_asset_compression_type(u.SoundAssetCompressionType.PCM)
  sound.set_editor_property('loading_behavior',u.SoundWaveLoadingBehavior.FORCE_INLINE)
  assert u.EditorAssetLibrary.save_loaded_asset(sound)
  duration=float(sound.get_editor_property('duration'));channels=int(sound.get_editor_property('num_channels'))
  assert abs(duration-cue['durationSeconds'])<.015 and channels==1,(cue['id'],duration,channels)
  report['imported'].append(dict(id=cue['id'],asset=sound.get_path_name(),sourceSha256=cue['sha256'],durationSeconds=duration,channels=channels,looping=False,volume=1,compression='PCM',loading='ForceInline'))
  save_report();print('COMBAT_AUDIO_IMPORTED',cue['id'],flush=True)
 report['status']='PASS';save_report();print('COMBAT_AUDIO_IMPORT_COMPLETE cues='+str(len(report['imported']))+' failures=0',flush=True)
except Exception as error:
 report['status']='FAILED';report['failures']+=1;report['error']=str(error);save_report();raise
finally:
 if '-KeepEditorForCombatAudio' not in u.SystemLibrary.get_command_line():
  _exit_started=time.monotonic();_exit_handle=None
  def finish_after_ticks(delta_seconds):
   global _exit_handle
   if time.monotonic()-_exit_started<10.:return
   u.unregister_slate_post_tick_callback(_exit_handle)
   print('COMBAT_AUDIO_EDITOR_SETTLED_EXIT',report['status'],flush=True);u.SystemLibrary.quit_editor()
  _exit_handle=u.register_slate_post_tick_callback(finish_after_ticks)
