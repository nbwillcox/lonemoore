"""Compare identical packaged scenes at 3440x1369 with uncapped rendering."""
import csv,json,statistics,sys
from pathlib import Path
csv.field_size_limit(20_000_000)
root=Path(__file__).resolve().parents[1]/'Saved/PerformancePass'
results=[]
for label in sys.argv[1:]:
    for p in sorted((root/label).glob('*.csv')):
        raw=list(csv.DictReader(p.open(encoding='utf-8-sig')))
        rows=[]
        for row in raw:
            try:
                if float(row['FrameTime'])>0:rows.append(row)
            except (ValueError,TypeError,KeyError):pass
        # Fixed steady-state sample window: omit boot/loading and the final screenshot.
        rows=rows[-1500:-200]
        assert len(rows)==1300, f'Incomplete steady-state capture: {p}'
        times=sorted(float(row['FrameTime']) for row in rows)
        mean=statistics.mean(times);p99=times[min(len(times)-1,int(len(times)*.99))]
        stats={}
        for key in ['GameThreadTime','RenderThreadTime','GPUTime','GPU/TemporalSuperResolution','GPU/RenderDeferredLighting','GPU/Basepass','GPU/LumenSceneUpdate','GPU/LumenScreenProbeGather','GPU/LumenReflections']:
            vals=[]
            for row in rows:
                try:vals.append(float(row[key]))
                except (ValueError,TypeError,KeyError):pass
            if vals:stats[key]=round(statistics.mean(vals),3)
        result={'label':label,'scene':p.stem,'samples':len(times),'mean_ms':round(mean,3),'fps':round(1000/mean,1),'p99_ms':round(p99,3),'one_percent_low_fps':round(1000/p99,1),'stats_ms':stats}
        results.append(result);print(json.dumps(result))
(root/'performance_results.json').write_text(json.dumps(results,indent=2))
