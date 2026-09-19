# Regional dressing placement simulation

Status: PASS. This is a source-data simulation, not a native runtime pass.

- 25 room templates × nine regional prop lists, exhausting each room/region selector period: 4,200 configurations.
- 16,800 selected placements; 16,800 accepted; 0 rejected after trying scales 1.0 / 0.75 / 0.55 / 0.4.
- 24,258 scale candidates considered. Accepted scales: {'1.0': 11824, '0.75': 2534, '0.55': 2402, '0.4': 40}.
- 0 overlapping new-prop bounding boxes; 0 boxes outside the 2,800cm module; 0 unrepresented prop types.
- Ground minimum Z: 2cm. Minimum selected-wall-plane clearance: 10.00cm. Native +Y facing dot: 1.000000.

## Coverage and limits

- Pure Python simulation using current JSON bounds and the documented C++ selector; not execution of Unreal placement code.
- Exhausts 25 templates, nine regional prop lists and each selector modulo period. These are hypothetical configurations, not 4,200 played floors or sampled dungeon seeds.
- Checks the new props against 45cm-expanded logical centre-to-centre lanes and each other using conservative axis-aligned bounding boxes.
- Wall clearance is relative to the selected anchor plane. It does not prove clearance against the detailed curved shell, embedded columns, existing older props, doors, braziers or stair geometry.
- Quarter-turn room rotation and grid translation preserve these local checks; independent physics/gate sweeps remain necessary.
- Does not verify native visibility, textures, Nanite culling, visual intersections, performance or player experience. Native captures and gameplay checks remain separate.

Run `audit_dressing_placement.py` with ordinary Python. It reads project data and writes only these audit files. Input SHA-256 hashes and per-room counts are recorded in `placement_simulation.json`.
