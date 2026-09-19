# Expanded Cistern prototype

Launch **Play Expanded Dungeon Prototype.cmd** in the project root. Choose a new random floor, continue the prototype autosave, or play the reviewed seed (73519).

**Atmosphere revision:** choose **1 — Continue** to see the changes in your existing save. The random miniature arches are removed. Lower ceilings, full-height columns, ceiling ribs, stocked shelves, pumping machinery and pipe banks make the chambers feel more enclosed. A warm player light and soft ambient fill follow the camera. World enemy sprites dim with distance; combat art is retained. [Before/after views](AtmosphereRevision/REVIEW.html).

**Approved and expanded across the campaign:** the latest build includes the full eighteen-floor rollout and new gothic stair art. [Current campaign gallery](../CampaignExpansion/REVIEW.html) and [save behavior](../CampaignExpansion/HANDOFF.md). Use **Play Lonemoore.cmd** for the normal campaign. This launcher remains a separate Cistern starting point with its own saves; unvisited floors beyond it also expand.

## Play

- W/S: move. A/D: turn. E: inspect, collect, operate gates and descend.
- M: map. Scroll: zoom. Middle mouse: pan. **Fit floor** frames the expanded map. Click explored ground to place a marker.
- F5/F9: prototype quicksave/load. T: return to town for supplies and care. Activated shrines provide return waypoints.
- A level-three warrior, mage and cleric form the review party. Ordinary combat rules apply; six attribute points per hero can be assigned on the character screen.
- Find the rusted key, open the guardian chamber, defeat the Rat King, then open the separate stair seal. The lever vault and hidden treasury are optional.

## Changes

- Seeded chamber shapes, sizes and connections, broad halls, narrow passages, loops, side rooms and a large reservoir with crossing bridges.
- Reviewed seed: 2,112 walkable cells, up from 158 (13.4 times the area). One hundred tested seeds contain 1,913–2,508 walkable cells, all exceeding the tenfold target.
- Twenty regular encounters, up from two. Each original encounter roster is repeated ten times, retaining the same enemy identities and exactly ten times the regular enemies for the same party and run. The Rat King remains one boss.
- Normal encounter experience is divided across the increased encounter count, preserving roughly the former total experience budget. Supplies and gold remain available throughout the longer expedition. Cleared prototype encounters stay cleared during town trips.
- Existing sewer materials, enemies, UI art and shared interactables are reused. The bridge, railing, reservoir pier and pump-wheel modules are supplemented by three new editable furnishing meshes: stocked wall stores, a pump station and a pipe bank. The former broken-aqueduct arch asset is retained on disk but is no longer placed. Existing moss, pipes, grates and surface decals dress the spaces.
- Walls span different lengths. Revised ceilings are 5.6m in passages, 6–9.5m in ordinary chambers and 14m in the reservoir. Full-height supports form smaller structural bays within larger rooms. Movement still follows the existing grid; visible architecture is no longer restricted to identical room boxes. The campaign rollout adds fire pits and other regional cavern treatments.
- Map and radar retain discovered terrain, dim remembered terrain and hide unexplored spaces. Walls and opaque gates block sight; reservoir gaps admit sight but block movement. Enemy markers require current detection.

## Preservation and implementation

Prototype saves use `ExpansionPrototype_` slots in the separate packaged build. Original layouts are never regenerated on load. Version-three floor records store geometry, seed, room volumes, encounter sources, door state and map discovery. Existing version-one/two saves remain supported.

The key barrier encloses the entire guardian area. A second boundary encloses the stairs and requires recorded boss victory. Generation validates reachability with keys, boss victory and the optional lever independently withheld. Physical wall/bridge/gate collision is checked against the same navigation boundaries.

The Blender source and FBX exports are in this folder and `AtmosphereRevision`. New imported assets are in `Content/ExpansionPrototype`. Initial expansion evidence is in `Saved/ExpansionPrototype`; atmosphere-revision backups and evidence are in `Saved/ExpansionAtmosphere`.

The original images and validation in this folder document the approved prototype. Use the current campaign gallery for the final regional rollout and corrected stairs.
