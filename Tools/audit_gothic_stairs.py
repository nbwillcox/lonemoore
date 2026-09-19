import bpy,json
from pathlib import Path
r=Path(__file__).resolve().parents[1]
bpy.ops.wm.open_mainfile(filepath=str(r/'ArtReview/CampaignExpansion/Stairs/Gothic_Stairs.blend'))
report=[]
for o in bpy.data.objects:
    if not o.name.startswith('Stair_'):continue
    exposed=[];coplanar=[]
    for face in o.data.polygons:
        if face.normal.z<.999:continue
        points=[o.matrix_world@o.data.vertices[v].co for v in face.vertices]
        if max(p.z for p in points)-min(p.z for p in points)>.0001:continue
        z=sum(p.z for p in points)/len(points)
        if abs(z)<.003:coplanar.append(face.index)
        if z>=0:exposed.append(z)
    assert not coplanar,(o.name,coplanar)
    report.append({'mesh':o.name,'upward_faces_coplanar_with_floor':len(coplanar),'lowest_upward_surface_cm':min(exposed)*100})
assert len(report)==18
(r/'Saved/CampaignExpansion/stair_geometry_audit.json').write_text(json.dumps(report,indent=2))
print('STAIR_FLOOR_SEPARATION_PASS',len(report))
