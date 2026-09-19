# Development tools

The project uses Unreal Engine 5.8.2 at `D:\Epic Games\UE_5.8`. Original art in `.art` is read-only; imports and modeling outputs live elsewhere.

## Build and verify

From the project folder, run `Tools\Build.ps1 -Test -Package` in PowerShell. This builds the editor module, runs the `Dungeon` native automation suites, then packages Windows to `Builds\Windows`. Build tools also use Unreal's normal caches outside the project. The script uses `-UBANoDetour -nocache` for native compilation because the accelerator stalled on this machine; Unreal's asset cache is still used.

Run `Tools\validate_content.py` after editing `Content\Game\Data\campaign.json`. It checks authored-map connectivity, progression gates, definitions, recruitment, and hunt alcoves. Results go to `Saved\Validation\content.json`.

The packaged executable supports the internal `-DungeonReview` flag with `-RenderOffscreen -windowed -ResX=1600 -ResY=900 -ForceRes -unattended -nosplash -nosound`. It captures rendered screens, checks UI pointer handlers, and sends F5/F9 through Unreal's input system. Review saves use a unique prefix and are removed on normal review completion. Screenshots go under the running game's `Saved\Validation\Screenshots`; the review exits automatically. The party-layout screenshot uses an explicit fixture, not campaign progression.

## Content sources

- `make_content.py` creates base rules, items, enemies, and Cathedral data. **Always run `expand_campaign.py` afterward**; the base generator alone does not contain the full campaign.
- `expand_campaign.py` builds the 18 fixed maps and their progression data. It does not randomize topology.
- `build_dungeon_kit.py` runs in Blender 5.2 to create the development mesh kit in `ArtReview\ModularKit`.
- `make_audio.py` creates original temporary tones and music loops.
- `import_content.py` runs through Unreal's Python editor plugin to import images, meshes, audio, materials, and the boot map. Existing image/mesh imports are skipped.

## Runtime code

- `DungeonModel`: data, hero rules, inventories, services, and persistence.
- `DungeonExploration`: grid interaction, hunts, recruitment, and story transitions.
- `DungeonCombat`: initiative, skills, enemies, rewards, and death rules.
- `DungeonGame`: controller input, 3D scene, camera, audio, and rendered review.
- `DungeonUI` and `DungeonCommands`: interface drawing, hit regions, and actions.
- `DungeonTests`: rules, combat/death, persistence/hunts, quarry/endings persistence, and complete routes for all three starters.

The campaign test writes `Saved\Validation\campaign_routes.txt`. Native test results are exported to `Saved\Validation\Automation`. Review logs and screenshots are evidence for a development build; they do not replace a sustained human playtest.

## Phase 2 additions

Run phase2_import.py through Unreal's Python commandlet to rebuild the key atlas material and save instancing compatibility flags. Names are authored in Content/Game/Data/names.json. See PHASE_2.md for feature scope and verification evidence.

## Public release packaging

Use Tools/Build.ps1 -Test -Package -Configuration Shipping -ArchiveDirectory Builds/Distribution for a Shipping package. Then run python Tools/package_release.py to create the Windows ZIP and SHA-256 checksum in Dist. The archive excludes saves, logs, crash dumps, and debug symbols.

The packaged player launcher is `play-lonemoore.exe`. Unreal module names, runtime paths, and existing save folders keep their internal identifiers for compatibility.
