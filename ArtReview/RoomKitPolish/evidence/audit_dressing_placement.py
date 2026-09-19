"""Read-only simulation of current dressing bounds; writes evidence beside this file.

This supplements native review. It does not open Unreal or Blender, modify assets,
or claim that bounding boxes prove detailed visual/physical mesh clearance.
"""
from pathlib import Path
from datetime import datetime, timezone
import collections
import hashlib
import itertools
import json
import math

ROOT = Path(__file__).resolve().parents[2]
OUT = Path(__file__).resolve().parent
paths = {
    'roomTemplates': ROOT / 'Content/Game/Data/room_kit.json',
    'wallAnchors': ROOT / 'Content/Game/Data/room_dressing_anchors.json',
    'importedPropBounds': ROOT / 'Content/Game/Data/regional_dressing.json',
    'placementImplementation': ROOT / 'Source/DungeonCrawler/DungeonRoomDressing.cpp',
}
templates = {room['id']: room for room in json.loads(paths['roomTemplates'].read_text())['templates']}
anchors = json.loads(paths['wallAnchors'].read_text())['templates']
props = json.loads(paths['importedPropBounds'].read_text())['assets']
groups = {region: [prop for prop in props if prop['regionIndex'] == region] for region in range(9)}
assert len(templates) == len(anchors) == 25 and len(props) == 24
assert all(groups.values())
implementation = paths['placementImplementation'].read_text()
for fragment in ['(J*2+Index)%Sites->Num()', '(Index+J)%Options.Num()', 'Site.Inward.Rotation().Yaw-90',
                 '1.f,.75f,.55f,.4f', 'Depth+10', 'Local.Z=2.f-LocalBounds.Min.Z', 'ExpandBy(FVector(45,45,0))']:
    assert fragment in implementation, 'Simulation needs review after implementation changed: ' + fragment


def segment_overlaps_box(a, b, low, high):
    return (max(a[0], b[0]) >= low[0] and min(a[0], b[0]) <= high[0] and
            max(a[1], b[1]) >= low[1] and min(a[1], b[1]) <= high[1])


def clears_lanes(rows, low, high):
    if low[2] > 245 or high[2] < 10:
        return True
    low = [low[0]-45, low[1]-45, low[2]]
    high = [high[0]+45, high[1]+45, high[2]]
    for y, row in enumerate(rows):
        for x, cell in enumerate(row):
            if cell != '.':
                continue
            start = ((x-3)*400, (y-3)*400)
            if segment_overlaps_box(start, start, low, high):
                return False
            for dx, dy in [(1, 0), (0, 1)]:
                nx, ny = x+dx, y+dy
                if nx < 7 and ny < 7 and rows[ny][nx] == '.' and segment_overlaps_box(start, (start[0]+dx*400, start[1]+dy*400), low, high):
                    return False
            if x in (0, 6) or y in (0, 6):
                length = math.hypot(*start)
                end = (start[0]+start[0]/length*200, start[1]+start[1]/length*200)
                if segment_overlaps_box(start, end, low, high):
                    return False
    return True


failures = []
overlaps = []
outside = []
scales = collections.Counter()
types = collections.Counter()
per_room = []
configurations = selected = accepted = rejected = scale_attempts = 0
min_wall_clearance = float('inf')
min_facing_dot = 1.
for record in anchors:
    room = templates[record['id']]
    sites = record['anchors']
    room_accept = room_reject = room_configurations = 0
    for region, options in groups.items():
        # Index only affects modulo Sites.Num and Options.Num. One least-common-
        # multiple period exhausts the current selector for this room/region.
        for index in range(math.lcm(len(sites), len(options))):
            configurations += 1
            room_configurations += 1
            placed = []
            for j in range(min(4, len(sites))):
                selected += 1
                site = sites[(j*2+index) % len(sites)]
                prop = options[(index+j) % len(options)]
                length = math.sqrt(sum(v*v for v in site['inwardNormal']))
                inward = [v/length for v in site['inwardNormal']]
                angle = math.atan2(inward[1], inward[0])-math.pi/2
                cosine, sine = math.cos(angle), math.sin(angle)
                front = (-sine, cosine)
                min_facing_dot = min(min_facing_dot, front[0]*inward[0]+front[1]*inward[1])
                done = False
                for scale in [1., .75, .55, .4]:
                    scale_attempts += 1
                    half = [(prop['maxCm'][k]-prop['minCm'][k])*.5*scale for k in range(3)]
                    extent = [abs(cosine)*half[0]+abs(sine)*half[1], abs(sine)*half[0]+abs(cosine)*half[1], half[2]]
                    depth = abs(inward[0])*extent[0]+abs(inward[1])*extent[1]
                    center = [site['position'][k]+inward[k]*(depth+10) for k in range(2)]
                    low = [center[0]-extent[0], center[1]-extent[1], 2.]
                    high = [center[0]+extent[0], center[1]+extent[1], 2.+extent[2]*2]
                    if not clears_lanes(room['rows'], low, high):
                        continue
                    descriptor = dict(room=record['id'], region=region, index=index, prop=prop['id'], scale=scale)
                    for prior, prior_low, prior_high in placed:
                        intersection = [min(high[k], prior_high[k])-max(low[k], prior_low[k]) for k in range(3)]
                        if min(intersection) > .01:
                            overlaps.append(dict(**descriptor, otherProp=prior, overlapCm=[round(v, 3) for v in intersection]))
                    if min(low[:2]) < -1400-.01 or max(high[:2]) > 1400+.01:
                        outside.append(descriptor)
                    for corner in itertools.product(*[(low[k], high[k]) for k in range(2)]):
                        clearance = sum((corner[k]-site['position'][k])*inward[k] for k in range(2))
                        min_wall_clearance = min(min_wall_clearance, clearance)
                    placed.append((prop['id'], low, high))
                    accepted += 1
                    room_accept += 1
                    types[prop['id']] += 1
                    scales[str(scale)] += 1
                    done = True
                    break
                if not done:
                    rejected += 1
                    room_reject += 1
    per_room.append(dict(id=record['id'], configurations=room_configurations, accepted=room_accept, rejected=room_reject))
missing_types = [prop['id'] for prop in props if not types[prop['id']]]
if overlaps:
    failures.append(f'{len(overlaps)} accepted prop bounding boxes overlap another new prop.')
if outside:
    failures.append(f'{len(outside)} accepted bounding boxes extend outside their module.')
if min_wall_clearance < 10-.01 or min_facing_dot < .9999:
    failures.append('Wall-plane clearance or native +Y facing failed.')
if missing_types:
    failures.append('Some prop types never fit in this selection-space simulation.')
limitations = [
    'Pure Python simulation using current JSON bounds and the documented C++ selector; not execution of Unreal placement code.',
    'Exhausts 25 templates, nine regional prop lists and each selector modulo period. These are hypothetical configurations, not 4,200 played floors or sampled dungeon seeds.',
    'Checks the new props against 45cm-expanded logical centre-to-centre lanes and each other using conservative axis-aligned bounding boxes.',
    'Wall clearance is relative to the selected anchor plane. It does not prove clearance against the detailed curved shell, embedded columns, existing older props, doors, braziers or stair geometry.',
    'Quarter-turn room rotation and grid translation preserve these local checks; independent physics/gate sweeps remain necessary.',
    'Does not verify native visibility, textures, Nanite culling, visual intersections, performance or player experience. Native captures and gameplay checks remain separate.',
]
report = dict(status='PASS' if not failures else 'FAIL', audit='Static regional dressing placement simulation', generatedUtc=datetime.now(timezone.utc).isoformat(),
              inputs={key:dict(path=str(path.relative_to(ROOT)),sha256=hashlib.sha256(path.read_bytes()).hexdigest()) for key,path in paths.items()},
              roomTemplates=25, regions=9, propTypes=24, configurations=configurations, selectedPlacements=selected,
              scaleAttempts=scale_attempts, acceptedPlacements=accepted, rejectedPlacements=rejected, scaleDistribution=dict(scales),
              overlappingNewPropAabbs=len(overlaps), aabbsOutsideModule=len(outside), missingPropTypes=missing_types,
              minimumAnchorPlaneClearanceCm=round(min_wall_clearance, 6), minimumNativeFrontDot=round(min_facing_dot, 6),
              groundedMinimumZCm=2, laneHalfWidthCm=45, failures=failures, overlapExamples=overlaps[:12], outsideExamples=outside[:12],
              perRoom=per_room, placementsByProp=dict(types), limitations=limitations)
(OUT/'placement_simulation.json').write_text(json.dumps(report, indent=2))
notes = ['# Regional dressing placement simulation', '', f'Status: {report["status"]}. This is a source-data simulation, not a native runtime pass.', '',
         f'- 25 room templates × nine regional prop lists, exhausting each room/region selector period: {configurations:,} configurations.',
         f'- {selected:,} selected placements; {accepted:,} accepted; {rejected:,} rejected after trying scales 1.0 / 0.75 / 0.55 / 0.4.',
         f'- {scale_attempts:,} scale candidates considered. Accepted scales: {dict(scales)}.',
         f'- {len(overlaps)} overlapping new-prop bounding boxes; {len(outside)} boxes outside the 2,800cm module; {len(missing_types)} unrepresented prop types.',
         f'- Ground minimum Z: 2cm. Minimum selected-wall-plane clearance: {min_wall_clearance:.2f}cm. Native +Y facing dot: {min_facing_dot:.6f}.',
         '', '## Coverage and limits', '', *['- '+line for line in limitations], '',
         'Run `audit_dressing_placement.py` with ordinary Python. It reads project data and writes only these audit files. Input SHA-256 hashes and per-room counts are recorded in `placement_simulation.json`.']
(OUT/'placement_simulation.md').write_text('\n'.join(notes)+'\n')
print(f'PLACEMENT_SIMULATION {report["status"]} configurations={configurations} accepted={accepted} rejected={rejected} overlaps={len(overlaps)} outside={len(outside)}')
