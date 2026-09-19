"""Summarize warmed native room-kit captures; do not compare different scenes as speedups."""
import argparse, csv, json, statistics
from pathlib import Path
csv.field_size_limit(20_000_000)
root = Path(__file__).resolve().parents[1] / 'Saved/RoomKitPass'
parser = argparse.ArgumentParser()
parser.add_argument('--label', default='Release')
args = parser.parse_args()
results = []
for path in sorted((root/'Performance'/args.label).glob('*.csv')):
    rows = []
    for row in csv.DictReader(path.open(encoding='utf-8-sig')):
        try:
            if float(row['FrameTime']) > 0:
                rows.append(row)
        except (ValueError, TypeError, KeyError):
            pass
    rows = rows[-1500:-200]
    assert len(rows) == 1300, path
    times = sorted(float(row['FrameTime']) for row in rows)
    mean = statistics.mean(times)
    p99 = times[int(len(times)*.99)]
    stats = {}
    for key in ['GameThreadTime', 'RenderThreadTime', 'GPUTime', 'GPU/TemporalSuperResolution',
                'GPU/RenderDeferredLighting', 'GPU/LumenSceneUpdate', 'GPU/LumenScreenProbeGather']:
        values = []
        for row in rows:
            try:
                values.append(float(row[key]))
            except (ValueError, TypeError, KeyError):
                pass
        if values:
            stats[key] = round(statistics.mean(values), 3)
    result = dict(label=path.parent.name, scene=path.stem, samples=len(times),
                  fps=round(1000/mean, 1), mean_ms=round(mean, 3), p99_ms=round(p99, 3),
                  p99_fps=round(1000/p99, 1), stats_ms=stats)
    results.append(result)
    print(json.dumps(result))
assert results, 'No native performance captures found'
(root/'performance_results.json').write_text(json.dumps(results, indent=2))
