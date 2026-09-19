# Lonemoore

Double-click **Play Lonemoore.cmd** to play. Start a **New Game** for the smaller floors and named-character save system.

Choose any of the seven classes: Warrior, Mage, Ranger, Cleric, Rogue, Paladin or Warlock. Enter your character's name, choose Normal or Hardcore, then begin. Names can contain up to 24 letters, numbers, spaces, apostrophes, hyphens or underscores. Each name identifies a separate journey; choose another name to start another character. Companions still join below, and every starting class can form a party of five different classes.

Each character has an **Autosave**, a **Quicksave**, and as many separate manual saves as you create. Open the pause menu, choose **Save**, then **Create new manual save**. **Load** lets you choose a character and a saved moment. **Continue** resumes the most recently saved usable journey. A successful save displays a **Game saved** popup. The initial autosave is created when your name is accepted; an interrupted introduction can resume safely in town.

The normal campaign stores these files in `Builds/Windows/DungeonCrawler/Saved/SaveGames/Characters/<your character name>/`. Older flat saves remain on disk but are not listed in the new character browser. This release is intended for fresh journeys.

New floors contain **68 room sections**, about **35% fewer** than before, with **45–46 regular encounter sites** spread across the smaller layout. The 25 Blender room designs, regional props, enemy artwork, bridges and Gothic boss approach remain. Dungeon lighting is dimmer, with a local torch around the player. Fog of war still hides unexplored areas.

Every descent chamber has a **save shrine beside the stairs**, accessible after the required key and guardian gates. Activate it with **E** to autosave and add a town return point before descending. Shrines do not restore health or mana. Chest rewards now include the full 33-item catalog—equipment, food, potions and relics—with higher qualities favored deeper down. A full bag leaves the chest's fixed reward available for later.

Combat has distinct short effects for all 21 active skills, a retro fireball and metallic attack sounds. The Audio settings control their volume.

**Play Authored Room Dungeon.cmd** remains a separate prepared crypt playtest. Choose **2** for a fresh random layout or **3** for the reviewed seed. Its saves are separate from named campaign characters. The expanded prototype launcher remains a separate legacy layout test. See the [current review](ArtReview/AdventurePolish/REVIEW.html) and [sound library](ArtReview/AdventurePolish/Audio/REVIEW.html).

The first launch may take about a minute on this machine. Let it reach the menu before opening another copy.

- **W / S:** step forward / backward
- **A / D:** turn 90 degrees
- **E:** inspect, open, collect, activate a shrine, or use stairs
- **M:** open/close the north-up automap; middle-drag to pan, scroll to zoom, and click explored ground to add/remove a marker. Use Recenter and the visited-floor buttons on the right. Radar range adjusts from two to six tiles.
- **I / C / J:** inventory / character sheet / hunt journal
- **T:** teleport to town outside combat
- **Esc:** menu
- **F5 / F9:** quicksave / confirm quickload

Click an enemy to select a target, then choose an action for the highlighted hero. Skills unlock at levels 1, 5, and 10. Assign three attribute points after each level on the character sheet.

In Inventory, select a ring or charm and choose **Equip to accessory 1** or **Equip to accessory 2**. Either accessory can use either slot. Replacing one returns the previous item to the bag and leaves the other slot equipped.

Enemy turns have a short preparation and result delay; action controls are unavailable until a hero can act again. **Action history** at the bottom opens recent town and combat events, with Older/Newer pages.

At the merchant, choose **Sell party items** to browse all heroes' belongings together. Each purchase and sale requires confirmation. Equipped sales are labelled and warn before removing the gear. At the smith, each upgrade shows current and next item power plus the hero's resulting Attack and Armor. Guardian equipment that does not fit in any bag can be collected later through the inventory's reward button.

Open **Settings** for the parchment handbook. Graphics includes display size, fullscreen, quality, brightness, VSync, frame limit and advanced options. Audio has separate master, music and effects levels. Key bindings are saved automatically; assigning an occupied key swaps the bindings. Escape cancels a key change. The list above shows the defaults.

Activate shrines to choose return destinations from the Dungeon Entrance. Examine floor grooves before stepping on traps, and inspect unusual stone seams for secrets. Return to town for healing, resurrection in Normal mode, rest, supplies, and upgrades.

Face a locked gate and press **E** after obtaining its key, then wait for it to finish opening. A key alone does not open the doorway. Step onto the stairs before using them. Explored map areas remain visible in dim colors; bright areas are currently visible. Enemy crosses disappear when you lose sight of them. Secret passages remain hidden until discovered.

The pre-choice **BeforeAstra** save lets you explore both endings. Killing Astra and continuing through the credits and portal scene unlocks Hell Hunts. Hardcore deaths are permanent for that journey.

This is a development build. See `BUILD_STATUS.md`, `KNOWN_ISSUES.md`, and `QUESTIONS_FOR_TOMORROW.md`.
