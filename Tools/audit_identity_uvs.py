import bpy,json,math
from pathlib import Path
rows=[]
for o in bpy.data.objects:
 if o.type!='MESH' or not o.name.startswith('SM_ID_'):continue
 o.data.calc_loop_triangles();bad=0;areas=[]
 for t in o.data.loop_triangles:
  uv=[o.data.uv_layers.active.data[li].uv for li in t.loops]
  ua=abs((uv[1].x-uv[0].x)*(uv[2].y-uv[0].y)-(uv[1].y-uv[0].y)*(uv[2].x-uv[0].x))/2
  if ua<1e-9 and t.area>1e-7:bad+=1
  if t.area>1e-7:areas.append(ua/t.area)
 rows.append(dict(name=o.name,bad=bad,min=min(areas),max=max(areas),uv=list(o.data.uv_layers.active.data[0].uv)))
print('UV_AUDIT',json.dumps(rows))
