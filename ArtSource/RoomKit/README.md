# Authored dungeon room kit

25 editable Blender rooms feed the existing seeded dungeon generator. Each new floor contains 104 room placements, including exploration loops, two consecutive guardian approach halls, an arena and a sealed descent chamber.

The kit includes circular and octagonal chambers, two kinds of turns, straight and crossing passages, burial galleries, chapels, a library, barracks, ruins, a cavern, bridges over void and fire, and a cistern crossing. Regional materials and existing enemy artwork remain in use.

## Source and runtime

- `Scenes/`: editable Blender scenes, in metres.
- `Meshes/`: exported FBX files with Unreal coordinate conversion applied to export copies.
- `Renders/`: Blender asset previews; these are not game screenshots.
- `validation.json`: geometry counts and duplicate/degenerate-face checks.
- `../../Content/Game/Data/room_kit.json`: footprints, sockets, furnishing positions and lighting positions.
- `../../ArtReview/RoomKit/REVIEW.html`: native packaged screenshots and final validation, once produced.

Rooms keep their authored scale. A 7-cell envelope provides matching connections while actual room shapes, corridors, ceiling heights, curves and pits vary. Every connection shares a 4-metre-wide vaulted profile, with a 5.6-metre shoulder and 6.4-metre peak. Ceiling transitions and stone reveals contain the texture changes at joins. Solid caps are owned once per unused connection. No brick-by-brick meshes are generated.

Existing explored, occupied, completed and corpse-recovery floors retain their saved layouts. New games and untouched floors use content version 4. The legacy renderer remains available for old saved floors. The separate authored-room launcher starts a new crypt playtest without touching the main journey.

## Rendering choices

The room surfaces use metric UVs and the existing high-resolution PBR textures. Meshes are grouped for rendering and imported with Nanite; the game keeps its occlusion culling and asynchronous scene preparation. Movement and door collision come from the same validated room socket graph. Native reviews compare that collision with legal movement; screenshots and Blender geometry checks also assess the visible surfaces.

Implementation references: [Epic Nanite guidance](https://dev.epicgames.com/documentation/unreal-engine/working-with-naniteenabled-content), [visibility and occlusion](https://dev.epicgames.com/documentation/unreal-engine/visibility-and-occlusion-culling-in-unreal-engine), [texture coordinate expressions](https://dev.epicgames.com/documentation/unreal-engine/coordinates-material-expressions-in-unreal-engine).
