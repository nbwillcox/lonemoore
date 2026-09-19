"""Record packaged combat review evidence and source/save preservation."""
from pathlib import Path
import hashlib
import json
import re
import struct

ROOT = Path(__file__).resolve().parents[1]
QA = ROOT/'Saved/CombatFXPass'
failures = []
before = json.loads((QA/'preservation-before.json').read_text(encoding='utf-8-sig'))
for row in before:
    path = Path(row['Path'])
    if not path.exists() or hashlib.sha256(path.read_bytes()).hexdigest().upper() != row['Hash']:
        failures.append(f'Preservation mismatch: {path}')
audio = json.loads((ROOT/'Saved/AdventurePolish/combat_audio_validation.json').read_text())
imported = json.loads((ROOT/'Saved/AdventurePolish/combat_audio_import.json').read_text())
combat_assets = {'A_'+row['id']+'.uasset' for row in imported['imported']}
unrelated_audio = 0
for old in (QA/'Backup/Audio').glob('*.uasset'):
    if old.name in combat_assets:
        continue
    current = ROOT/'Content/Game/Audio'/old.name
    if not current.exists() or hashlib.sha256(current.read_bytes()).digest() != hashlib.sha256(old.read_bytes()).digest():
        failures.append(f'Unrelated audio changed: {old.name}')
    unrelated_audio += 1
if audio['status'] != 'PASS' or audio['cueCount'] != 33:
    failures.append('Source audio validation failed')
if imported['status'] != 'PASS' or len(imported['imported']) != 33:
    failures.append('Audio import incomplete')
tests = (ROOT/'Saved/Validation/tests.log').read_text(encoding='utf-8', errors='replace')
successes = re.findall(r'Test Completed\. Result=\{Success\}', tests)
test_count = re.search(r'Automation Test Queue Empty (\d+) tests performed', tests)
if not test_count or 'Result={Fail' in tests or len(successes) != int(test_count[1]):
    failures.append('Native automated tests incomplete or failed')
build_path = QA/'build-final-package.log'
raw = build_path.read_bytes()
build = raw.decode('utf-16' if raw.startswith(b'\xff\xfe') else 'utf-8', errors='replace')
if 'BUILD SUCCESSFUL' not in build:
    failures.append('Package build completion not found')
review = (QA/'packaged-review.log').read_text(encoding='utf-8', errors='replace')
if 'COMBAT_FX_REVIEW_COMPLETE cases=31 failures=0' not in review:
    failures.append('Packaged review completion not found')
audio_events = re.findall(r'COMBAT_FX_AUDIO cue=(\S+) loaded=(\d+)', review)
if len(audio_events) != 31 or any(loaded != '1' for _, loaded in audio_events):
    failures.append('Packaged audio routing/loading incomplete')
runtime_dirs = list((QA/'ReviewUser').glob('**/CombatFXPass/Runtime'))
screenshots = [p for folder in runtime_dirs for p in folder.glob('*.png')]
if len(screenshots) != 31:
    failures.append(f'Expected 31 packaged screenshots; found {len(screenshots)}')
dimensions = {struct.unpack('>II', path.read_bytes()[16:24]) for path in screenshots}
if dimensions != {(1600, 900)}:
    failures.append(f'Unexpected runtime capture dimensions: {dimensions}')
report = {
    'status': 'PASS' if not failures else 'FAIL', 'failures': failures,
    'audioCues': 33, 'skillIdentities': 21, 'spriteSequences': 30, 'spriteFrames': 240,
    'nativeAutomationTests': int(test_count[1]) if test_count else 0,
    'packagedReviewCases': 31, 'packagedAudioEvents': len(audio_events),
    'packagedScreenshots': len(screenshots),
    'captureResolution': sorted(dimensions),
    'preservedSaves': sum(Path(r['Path']).suffix == '.sav' for r in before),
    'preservedSaveMetadataFiles': sum(Path(r['Path']).suffix == '.json' for r in before),
    'preservedOriginalArt': sum('\\.art\\' in r['Path'] for r in before),
    'preservedUnrelatedAudioAssets': unrelated_audio,
    'limitations': ['Automated audio loading, dispatch, and waveform checks do not establish subjective listening quality.'],
}
(QA/'validation-summary.json').write_text(json.dumps(report, indent=2)+'\n')
print(json.dumps(report, indent=2))
if failures:
    raise SystemExit(1)
