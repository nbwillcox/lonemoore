LONEMOORE — DUNGEON INTEGRITY, ENVIRONMENT, AUTOMAP & RADAR UPDATE

Work inside the existing Lonemoore project. Implement this as one coordinated update with internal testing checkpoints—not merely a proposal or disconnected demonstration.

GOAL

Create old-school first-person dungeons with modern 3D depth, atmosphere, and environmental storytelling.

Current problem: players can bypass locked walls/doors and reach the next floor’s stairs without completing the intended progression.

Fix the underlying dungeon structure and movement rules, substantially improve the environments using Blender and Unreal, and integrate a full exploration map plus compact radar.  Fog of war is a must.

Preserve Lonemoore’s existing grid-based movement, 90-degree turns, first-person camera, turn-based combat, party, inventory, and established progression. Do not convert it into a free-roaming action game.


1. INSPECT THE PROJECT AND ESTABLISH SHARED DUNGEON DATA

Read the current code, project instructions, dungeon definitions, approved concept art, and existing UI assets before changing architecture.

Use the installed Blender and Unreal versions and verify relevant APIs against those versions. Preserve uncommitted work and existing assets. Do not reset the project or perform unrelated upgrades.

Inspect the approved art in the project’s .art directory, including:
J:\First Person Dungeon Crawler Game\.art
when that remains the active project location.

Use existing approved assets first. Do not replace approved artwork without permission. Record missing assets in _MISSING_ART_LIST.md.

Reuse or extend the existing dungeon representation so these systems share authoritative data:
- Dungeon construction and connections.
- Legal player movement.
- Walls, doors, gates, and secrets.
- Keys, switches, and progression requirements.
- Stairs and floor transitions.
- Exploration map and radar.
- Saving and loading.

Represent connections explicitly. Two walkable tiles being adjacent must NOT automatically mean the player can cross between them.

Each boundary must have defined traversal and visibility behavior. These are separate: a barred opening may allow sight but prevent movement.

Use stable floor, cell, door, and object identifiers. Avoid competing versions of the dungeon layout in different systems.


2. FIX PROGRESSION BYPASSES AT THEIR SOURCE

Diagnose both:
A. Layout problems: alternate corridors, touching rooms, loops, or secrets bypassing a mandatory gate.
B. Movement/collision problems: walking through seams, around doorframes, across blocked tile boundaries, or outside the intended dungeon.

For mandatory progression gates, every legal route into the protected area must satisfy the intended gate requirement.

Build deliberate progression:
Entrance → explorable area → reachable prerequisite → required gate → protected area → stairs.

Allow branching, secrets, optional loot rooms, and meaningful loops. However, those connections must not accidentally cross a protected progression boundary.

Distinguish mandatory progression gates from optional treasure-room locks. Do not make every door mandatory or require clearing every enemy unless existing game rules already require it.

Required keys and switches must be obtainable before their own gate. Detect circular dependencies and consumable-key softlocks. Respect existing key types and region-scoped master keys.

Unlocking must persist correctly even after a key is consumed. Merely possessing a key must not silently make a physically closed door traversable.

Validate each movement across the intervening boundary—not just whether the destination tile is walkable. Interpolated movement must not ignore these checks.

Seal exterior boundaries and module junctions. Verify wall, doorframe, closed-door, floor, and relevant ceiling collision. Preserve doorway openings when building simplified collision.

Prevent diagonal corner slipping, movement through void cells, accidental connections between touching rooms, and interaction through solid walls.

Keep logical door state, physical collision, and door animation synchronized. Do not permit passage before the doorway is sufficiently open.

Place stair triggers entirely within their intended accessible area. Floor transitions must validate existing progression requirements as a second safeguard.

Do NOT “fix” the issue only by refusing to activate the stairs after the player already bypassed the dungeon. Repair the route and geometry too.


3. MAKE THE DUNGEONS FEEL LIKE PLACES

The visual target is:
Classic dungeon-crawler navigation, modern dark-fantasy environments.

Not:
Identical rectangular hallways with the same wall texture and random barrels.

Use authored room templates and deliberate connections. Where procedural generation already exists, constrain it with region-specific architectural rules.

Give rooms believable purposes: burial chambers, ossuaries, drainage junctions, guard posts, abandoned storerooms, ritual rooms, ruined chapels, or other spaces appropriate to the existing region.

Create depth through:
- Thick doorways, recessed tombs, wall niches, arches, pillars, and vaulted ceilings.
- Contrasts between cramped passages and larger chambers.
- Distant spaces visible through grates or across inaccessible drops.
- Foreground, middle-distance, and background composition.
- Memorable landmarks that help players recognize locations.

Preserve readable tile navigation. Visual elevation and scenic spaces must not accidentally become new routes or require a new locomotion system.

Use region-specific materials and storytelling. Sewers should not look like catacombs with different-colored lights. Follow the existing region definitions and approved concept art rather than inventing a new campaign.

Dress environments intentionally:
Water staining beneath leaks; soot above flames; rubble below damaged masonry; possessions around abandoned camps; disturbed tombs where something escaped.

Add restrained environmental motion and sound: dripping water, flickering flames, drifting dust, occasional vermin, creaking metal, distant movement, and room-appropriate ambience.

These are atmosphere systems, not permission to replace turn-based combat or add a complex ecosystem simulation.

Use darkness with readable shapes and purposeful lighting. Do not hide unfinished environments behind blackness, excessive fog, bloom, or blur.

REFERENCE EXPERIENCE:
A narrow burial passage opens into a tall ossuary. Inset tombs line the walls. A broken funerary statue becomes a navigation landmark. Dust drifts through a shaft of light. A locked iron gate visibly separates the lower stairwell. The key is obtainable in an accessible side chamber. Optional exploration loops return to the entrance side of the gate—not behind it.

Recreate the feeling and architectural richness of the concept art from the actual gameplay camera, not just from cinematic editor angles.


4. USE BLENDER AND UNREAL AS A REPEATABLE ART PIPELINE

Create or improve a reusable modular dungeon kit in Blender:
Straight sections, hard corners, junctions, dead ends, walls, floors, ceilings, doorway frames, doors, arches, stairs, pillars, niches, trim, and region-specific props.

Modules must fit the existing grid precisely, with consistent scale, pivots, connection points, wall thickness, and door clearances.

Use actual geometry for meaningful silhouettes, recesses, and architectural depth. Use controlled bevels, good normals, consistent UV density, and coherent material treatment.

Preserve editable .blend sources and repeatable generation/export scripts. Verify the imported scale and alignment in Unreal before propagating the kit.

Do not deliver a beautiful Blender render that is not integrated into the playable game.

Keep environmental variation away from connection boundaries and required walking/interaction clearances. Procedural dressing must not create holes, block keys, or cover readable doorways.

In Unreal, instance repeated static architecture where appropriate, grouped sensibly for visibility and maintenance. Keep interactive doors and other stateful objects independently controllable.

Use suitable lighting, materials, decals, particles, and localized atmosphere. Evaluate Nanite/Lumen or other rendering features against the current project and hardware budget rather than enabling everything indiscriminately.

Establish one polished, playable reference floor, then apply the validated construction and regional dressing system to the existing dungeon pipeline. Do not leave the old broken generation path active.


5. BUILD A FULL EXPLORATION MAP

Add a functional automap, opened with M unless that conflicts with an established binding.

Requirements:
- North-up layout, player location, and facing arrow.
- Current region/floor name.
- Pan, zoom, recenter, and a readable legend.
- View previously visited floors without revealing unvisited floors.
- Distinguish unexplored, discovered, and currently visible areas.
- Mark discovered stairs, doors, shrines, and relevant interactables.
- Support simple player-placed markers.

Reveal geometry through legitimate exploration and visibility—not an unconditional radius that exposes rooms through walls.

Unknown secret doors must appear as ordinary walls until discovered. Do not reveal hidden rooms, undiscovered loot, or future objectives through map icons.

Remember explored areas after leaving them, but do not provide remote live updates about unseen changes.

Derive the map from the same dungeon boundaries used by movement. Do not maintain a separate hand-built approximation.

Opening and interacting with the map must not also move the player or activate gameplay underneath it.


6. BUILD A COMPACT RADAR

Add a small HUD radar distinct from the full map.

Design it as a restrained medieval-fantasy instrument matching the approved UI—not a bright sci-fi scanner.

Default behavior:
- Player-centered local view with a clear facing indicator.
- Configurable short detection range.
- Current floor only.
- Nearby discovered navigation landmarks.
- Live enemy indicators only when legitimately detected.

For this pass, use range plus line of sight for enemy detection. Do not invent through-wall magical sensing or a new player ability.

Remove live enemy tracking when detection is lost. Never expose hidden enemies, secret rooms, or undiscovered treasure through radar.

Use distinguishable symbols as well as color.

Share coordinate transforms, floor IDs, visibility rules, and discovery data with the full map. Test alignment while moving and turning.

Render map/radar from filtered dungeon data rather than continuously capturing the entire level with an overhead camera.

Use event-driven updates for exploration and object state. Use a bounded update timer for detection rather than scanning every actor every frame.

Preserve icon aspect ratios. Use appropriate scalable panel borders instead of stretching decorative artwork. Verify standard and ultrawide HUD layouts.


7. SAVE STATE AND PROVE THE UPDATE WORKS

Extend the existing save system to preserve:
Exploration, discovered secrets, map markers, unlocked/opened doors, switches, collected prerequisites, and existing progression state.

Use stable identifiers. For generated floors, preserve the required layout data or seed plus generator/content version. Do not silently regenerate a different floor underneath an existing save.

Build automated validation for:
- Required prerequisites reachable before their gates.
- Protected stairs unreachable while their required gate remains unavailable.
- Correct progression making the stairs reachable.
- Optional routes and secrets not bypassing mandatory progression.
- Save/reload preserving all of the above.
- Collision and movement agreeing with dungeon boundaries.
- Map/radar not leaking undiscovered information.

Use state-aware reachability tests accounting for keys, switches, and persistent unlocks—not a flood fill that assumes every door is open.

For each bypass test, hold that gate’s requirement unavailable while allowing other legal actions. Report any bypass route using floor/cell/gate IDs.

Test missing key, wrong key, correct key with door still closed, opened door, consumed key after unlocking, and reload.

For procedural generation, test fixed regression cases plus at least 100 reproducible seeds per supported configuration. Validate newly generated layouts before allowing play. Use bounded retries and an explicit validated fallback or failure—not endless regeneration or silently accepting a broken floor.

Also perform in-engine movement and interaction tests. Graph validation alone does not prove the collision geometry is correct.

Provide actual gameplay-camera screenshots of improved environments, automap, and radar, plus a concise report of:
What changed, files/assets affected, tests actually run, measured performance, and remaining issues.

Never claim a tool ran, an asset was imported, or a test passed unless it actually happened.

PRIORITY:
Correct progression first.
Compelling, readable environments second.
Integrated map/radar and persistent state.
Finish with regression and performance verification.

This update succeeds when the dungeons are harder to bypass, more interesting to explore, visibly closer to the approved artwork, and reliably represented by both navigation systems.