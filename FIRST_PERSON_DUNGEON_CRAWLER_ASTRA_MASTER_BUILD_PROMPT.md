# FIRST PERSON DUNGEON CRAWLER
## Master Build Prompt for GPT Work / Codex / Astra

> **Purpose:** This document is the authoritative build specification for the game located at:
>
> `J:\First Person Dungeon Crawler Game`
>
> Build the game from the initial start screen through the final credits and postgame Hell Hunts.
>
> Do not redesign the game. Do not expand it into a giant RPG framework. Follow this specification as the source of truth.

---

# 1. ROLE AND EXECUTION MODE

You are the lead game-development agent for this project.

Your job is not merely to make a plan. Inspect the existing project, inspect the existing artwork, create or repair the Unreal project as needed, implement the systems, build the maps, wire the UI, integrate the assets, test the game, fix defects, and carry the project through to a playable packaged Windows build.

Work autonomously on implementation details that are already defined here.

Do **not** repeatedly ask the user to re-confirm decisions that are already specified in this document.

When an implementation detail is not specified:

1. Choose the smallest, clearest, most deterministic implementation.
2. Keep it data-driven where useful.
3. Avoid adding a new subsystem unless it is genuinely necessary.
4. Record any material assumption in `BUILD_STATUS.md`.
5. Do not invent major gameplay features.

If something requires missing artwork, follow the Missing Art Rules in this document instead of silently generating or replacing artwork.

---

# 2. NON-NEGOTIABLE DESIGN PHILOSOPHY

## DO NOT OVERENGINEER THIS GAME.

This is a polished throwback to a **1990s first-person dungeon crawler** with modern 3D dungeon environments and modern Unreal lighting.

The goal is:

- simple
- readable
- atmospheric
- responsive
- old-school
- turn-based
- first-person
- grid-based
- easy to understand
- fun to explore
- full of secrets

Do **not** add:

- Souls-like mechanics
- Souls-like combat
- dodge-roll systems
- stamina-management combat
- complex action combat
- open-world systems
- crafting systems
- survival meters
- hunger meters
- thirst meters
- procedural world generation
- free-roaming party management
- party formation systems
- front-row/back-row mechanics
- relationship meters
- romance systems
- faction reputation systems
- multiplayer
- live-service systems
- battle passes
- skill trees
- skill-point systems
- respec systems
- item-affix explosions
- Diablo-style rainbow loot
- hundreds of status effects
- elaborate weapon animation sets
- thousands of one-off art assets
- giant cinematic pipelines
- anything not specifically required by this spec

If there is a choice between a clever framework and a straightforward implementation, choose the straightforward implementation.

---

# 3. WORKING TITLE

Use:

**First Person Dungeon Crawler**

as the working project title until the user provides a final commercial title.

Do not invent a permanent title.

---

# 4. CORE PREMISE

The player controls a solitary adventurer drawn by an unnatural darkness to the isolated city of **Lonemoore**.

Beneath Lonemoore lies an ancient cursed descent.

The highest dungeon level is the ruined **Last Dawn Cathedral**.

Below it lies a chain of increasingly corrupted subterranean regions that ultimately descend into **Hell**.

The protagonist does not arrive because of a contract or formal quest.

**The darkness pulled them to Lonemoore.**

Over the course of the game the player recruits four guaranteed companions, uncovers the truth of the dungeon, reaches Hell, and confronts:

**Astra the Cosmic Chaos Nephilim Queen**

When this document refers to **Astra the Cosmic Chaos Nephilim Queen**, it means the fictional final boss character, not the development model/agent.

---

# 5. TARGET TECHNOLOGY

## Engine

Use **Unreal Engine**.

Prefer the newest stable Unreal Engine version already installed and compatible with the local project rather than unnecessarily migrating between engine versions.

Target:

- Windows x64
- keyboard and mouse
- 16:9 displays as the primary presentation
- responsive UI at common PC resolutions

## Code / Content Strategy

Prefer a simple hybrid:

- C++ for small reusable core gameplay systems where it improves reliability
- Blueprint for content wiring and straightforward gameplay presentation
- UMG for menus/HUD
- DataTables or Primary Data Assets for content definitions
- Unreal SaveGame objects for persistence

Do not create a giant abstraction framework.

## Dungeon Art Pipeline

The dungeon itself is fully 3D.

Use Blender to create a **modular dungeon kit**, not giant monolithic dungeon meshes.

Reuse core geometry wherever possible and differentiate regions using:

- materials
- textures
- props
- decals
- lighting
- fog
- color temperature
- environmental dressing

Use Unreal lighting to make the environments visually modern while retaining old-school layout and readability.

Use Lumen where appropriate and supported, but do not sacrifice stable performance or readability merely to use a particular rendering feature.

---

# 6. VISUAL PRESENTATION

The game combines:

- fully 3D dungeon architecture
- modern Unreal lighting
- old-school grid movement
- 2D illustrated enemy art where available
- 2D character cards
- 2D town artwork
- simple VFX for attacks and magic

Enemies do **not** need elaborate 3D character models.

Existing transparent monster artwork should be used as camera-facing sprites/billboards in the 3D dungeon where appropriate.

The game should visually feel like a modernized descendant of classic first-person dungeon crawlers rather than a modern action RPG.

---

# 7. PROJECT AND ART LOCATIONS

Project root:

`J:\First Person Dungeon Crawler Game`

Primary art root:

`J:\First Person Dungeon Crawler Game\.art`

Starting-character cards:

`J:\First Person Dungeon Crawler Game\.art\characters`

Recruitable-hero cards:

`J:\First Person Dungeon Crawler Game\.art\characters\heroes`

Town artwork:

`J:\First Person Dungeon Crawler Game\.art\locations\town`

Monster artwork:

`J:\First Person Dungeon Crawler Game\.art\monsters`

Weapon, armor, key, location, hunt-board, boss, and other artwork may exist elsewhere under `.art`.

**Recursively inspect `.art` before deciding an asset is missing.**

Do not modify the original source artwork in `.art`.

Import/copy approved assets into Unreal Content as necessary while preserving the source files.

Maintain a lightweight asset manifest mapping source artwork to Unreal assets.

---

# 8. MISSING ART RULES

Existing approved artwork takes priority over placeholders or generated replacements.

## Never silently generate new art.

If a required art asset is missing:

1. Add it to:
   `_MISSING_ART_LIST.md`
2. Keep the list simple.
3. State:
   - what asset is missing
   - where it is used
   - a short description of what needs to be generated
4. Continue building gameplay with a clearly labeled **development placeholder** if necessary.
5. Do not treat the placeholder as final art.
6. Do not add newly generated artwork to the game without explicit user approval.

Example entry:

- **Fire Spider**
  - Used in: Infernal Ruins
  - Needed: transparent front-facing monster image
  - Description: use the existing spider design as the basis, but make it a fire-corrupted variant instead of the normal poison spider.

Do not create an enormous art-production document.

The user will inspect `_MISSING_ART_LIST.md`.

---

# 9. CORE GAME LOOP

The primary loop is:

1. Begin in Lonemoore.
2. Review town services.
3. Accept optional hunts.
4. Enter the dungeon.
5. Explore grid-based 3D levels.
6. Fight visible enemy groups in turn-based combat.
7. Find weapons, armor, potions, food, keys, gold, secrets, and lore.
8. Recruit guaranteed story companions.
9. Activate waypoints.
10. Defeat mini-bosses and region bosses.
11. Teleport back to town when needed.
12. Heal, resurrect, repair, upgrade, rest, shop, and collect hunt rewards.
13. Return to the most recently selected activated waypoint.
14. Descend deeper.
15. Reach Hell.
16. Confront Astra.
17. Choose one of two endings.
18. If Astra is killed, unlock postgame Hell Hunts.

---

# 10. START SCREEN AND MAIN MENU

Main menu options:

- New Game
- Continue
- Load Game
- Settings
- Credits
- Quit

## New Game

New Game opens a character-selection screen using the existing preset character cards.

The player chooses exactly one starting hero:

1. Male Warrior
2. Female Mage
3. Male Ranger

There is:

- no custom character creator
- no appearance editor
- no stat reroll screen
- no custom class selection beyond choosing one of these three presets

The two unchosen starting heroes are gone from that playthrough.

They do not become recruitable later.

## Difficulty

Only:

- Normal
- Hardcore / Permadeath

No Easy/Normal/Hard ladder is required.

---

# 11. INTRODUCTION

After selecting the starting hero, play a short illustrated-panel introduction.

Use static or lightly animated illustrated panels rather than expensive cinematic production.

The introduction establishes:

- the protagonist is a loner
- rumors surround Lonemoore
- the city sits above a cursed descent
- something dark is calling the protagonist
- the protagonist arrives at Lonemoore
- the first interactive view is the main town square

Keep exposition concise.

The mystery should unfold during dungeon exploration.

---

# 12. LONEMOORE TOWN HUB

Lonemoore is a **small illustrated hub**, not a freely explorable 3D town.

Existing town artwork is located at:

`J:\First Person Dungeon Crawler Game\.art\locations\town`

Known town scenes include:

- Main Town Square
- Back Alley
- Blacksmith
- Dungeon Entrance
- Gambler
- Healer
- Hunt Board
- Merchant
- Tavern

## Main Town Square Interaction

The Main Town Square is the central screen.

Implement it as a full-screen illustrated scene with normalized hotspot regions.

When the mouse moves over an interactive area:

- subtly highlight the area
- show the location name
- optionally show one short description

When clicked:

- open that location scene/interface

When closed:

- return to Main Town Square

Do not stretch the art in a way that makes hotspots drift.

Use aspect-safe scaling and normalized hotspot coordinates.

---

# 13. TOWN SERVICES

## Back Alley

Leads to the Gambler.

## Blacksmith

Functions:

- repair damaged equipment
- upgrade equipment quality

Upgrades use **gold only**.

No crafting materials.

No blacksmith skill tree.

## Gambler

The Gambler deals in:

- rare antiquities
- random unidentified equipment
- expensive mystery items
- randomized item purchases for gold

Keep the system simple.

Do not build a casino simulator.

## Healer

Functions:

- heal wounded party members
- resurrect dead characters
- remove basic negative conditions where appropriate

Healing costs gold.

Resurrection costs substantially more.

All prices must be data-driven and level-scalable.

## Hunt Board

Provides optional side content during the normal game and becomes the Hell Hunt board after the good ending.

Detailed rules appear later in this document.

## Merchant

Sells:

- health potions
- mana potions
- food
- basic utility consumables
- basic gear where appropriate

## Tavern

Provides:

- food
- potions
- paid rest
- full recovery
- simple temporary rest/meal enhancements

Do not add hunger or thirst.

Any meal/rest bonus should be straightforward and temporary.

---

# 14. PARTY STRUCTURE

The game begins with one hero.

Four additional heroes are guaranteed story recruits.

The seven possible classes across the game are:

- Warrior
- Mage
- Ranger
- Cleric
- Rogue
- Paladin
- Warlock

The final active party contains exactly:

- the chosen starting hero
- Cleric
- Rogue
- Paladin
- Warlock

Maximum active party size:

**5**

There is no reserve roster.

There are no characters waiting in town.

There is no party-switching screen.

There are no front/back positions.

All recruited characters travel together and participate in combat together.

---

# 15. RECRUITABLE HEROES

All four recruits are guaranteed and found during the main dungeon story.

Suggested story placement:

## Cleric

Found in the Forgotten Catacombs, connected to a ruined chapel or failed holy mission.

## Paladin

Found deeper in the Ancient Crypts or related holy ruin as the survivor of a failed expedition.

## Rogue

Found in the Buried Fortress, prison, locked chamber, or similar location after getting trapped while pursuing treasure.

## Warlock

Found in the Infernal Ruins inside a sealed or dangerous chamber and knows more about the curse than the other companions.

Do not make recruitment missable in a normal playthrough.

Newly recruited characters immediately match the protagonist's current level.

For the levels they did not personally earn, automatically assign class-appropriate historical attribute growth so the player is not dumped into a screen with dozens of retroactive points to allocate.

After recruitment, normal level-up rules apply.

---

# 16. COMPANION DIALOGUE

Aim for richer companion personality without building a relationship simulator.

Use a lightweight data-driven dialogue/bark system.

Companions can comment on:

- entering a new region
- finding major secrets
- unusual lore
- bosses
- town visits
- their own recruitment
- major story reveals
- approaching Hell
- the final confrontation
- the player's final choice

Optional longer conversations can occur in the Tavern or rare dungeon safe areas.

Do not implement:

- affection meters
- romance
- approval scores
- branching companion loyalty systems

If full rich dialogue becomes impractical, retain at least moderate contextual dialogue.

---

# 17. RPG ATTRIBUTES

Core attributes:

- Strength
- Dexterity
- Vitality
- Intelligence
- Wisdom
- Luck

Derived stats include:

- HP
- MP
- Armor
- Attack
- Magic Power
- Accuracy
- Evasion
- Critical Chance
- Initiative / Speed
- elemental/status resistances where appropriate

## Attribute Roles

Keep their effects intuitive.

### Strength
Primary physical damage scaling.

### Dexterity
Primary initiative/speed influence.
Also modestly influences:
- accuracy
- evasion
- critical chance

### Vitality
Primary HP growth and physical durability.

### Intelligence
Primary offensive magic scaling and contributes to MP.

### Wisdom
Healing effectiveness, supportive magic, status resistance, and contributes to MP.

### Luck
Modestly influences:
- critical chance
- unusual loot chances
- status application/resistance rolls
- other small probability systems

Avoid opaque formulas.

---

# 18. LEVELING

Traditional XP system.

Maximum level:

**99**

There is no attribute hard cap.

Players who enjoy postgame grinding are allowed to become extremely powerful.

The normal story should not require level 99.

Target normal campaign completion roughly in the level range that feels natural after 15-25 hours, with Hell Hunts supporting continued progression toward 99.

## Every Level

Each level provides:

- normal class-based automatic HP/MP/stat growth
- **3 freely assignable attribute points**

No skill points.

No skill trees.

No respec.

Class choice is permanent.

All currently recruited party members receive the full encounter XP reward so the party does not drift into annoying level mismatches.

---

# 19. CLASS SKILLS

Each class has exactly:

- 3 active class skills
- 1 innate passive

No additional skill trees.

No equipped-skill menu is needed because the class only has three active skills.

## Unlock Rules

- Skill 1: level 1
- Skill 2: level 5
- Skill 3: level 10
- Passive: always active

## Warrior

Active:
1. Power Strike
2. Cleave
3. Shield Wall

Passive:
- increased physical defense

## Mage

Active:
1. Fireball
2. Ice Lance
3. Chain Lightning

Passive:
- increased magic damage

## Ranger

Active:
1. Power Shot
2. Multi-Shot
3. Hunter's Mark

Passive:
- increased accuracy and modest critical bonus

## Cleric

Active:
1. Heal
2. Holy Smite
3. Resurrection

Passive:
- increased healing effectiveness

## Rogue

Active:
1. Backstab
2. Poison Blade
3. Smoke Bomb

Passive:
- increased critical chance and improved trap detection

## Paladin

Active:
1. Holy Strike
2. Divine Shield
3. Lay on Hands

Passive:
- increased armor and holy resistance

## Warlock

Active:
1. Shadow Bolt
2. Curse
3. Soul Drain

Passive:
- increased dark damage

All active skills use MP.

---

# 20. HP AND MP RECOVERY

## HP

HP regenerates naturally outside combat, but **extremely slowly**.

The rate should be slow enough that potions, food, the Healer, and returning to town remain the meaningful recovery tools.

Do not turn this into modern rapid health regeneration.

## MP

MP also regenerates naturally outside combat, but **extremely slowly**.

Mana potions are the primary field-recovery mechanism.

Town rest is the reliable full recovery.

Keep both regeneration rates data-driven for tuning.

---

# 21. INVENTORY

Every party member has:

**20 inventory slots**

The UI should present party inventory as one convenient large loot-bag style screen while still preserving each character's 20-slot ownership/capacity internally.

Maximum eventual inventory capacity with five characters:

**100 character inventory slots**

There is:

- no weight system
- no town storage chest
- no unlimited stash

## Gold

Gold is a global party currency.

Gold never consumes inventory space.

## Key Items

Keys and major quest items do not consume normal inventory slots.

## Stacking

Keep stacking simple:

- equipment: one item per slot
- potions/food: small sensible stacks
- key items: separate key-item collection

If all normal inventory slots are full, the player must make space before taking additional normal loot.

---

# 22. EQUIPMENT SLOTS

Use the simpler equipment layout:

- Weapon
- Off Hand
- Head
- Body
- Hands
- Feet
- Accessory 1
- Accessory 2

Do not add a dozen armor micro-slots.

---

# 23. WEAPON TYPES

Supported weapon families:

- Daggers
- Swords
- Broadswords
- Axes
- Hammers / Maces
- Spears
- Bows
- Crossbows
- Holy Staff
- Mage Staff

Use existing generated weapon/icon art when available.

If an off-hand shield asset already exists, support shields for appropriate classes without expanding the primary weapon-family list.

---

# 24. ARMOR TYPES

Supported armor families:

- Cloth
- Leather
- Studded Leather
- Chain
- Scale
- Scale and Plate
- Plate
- Ornate Plate
- Priest Robes
- Mage Robes

Use existing generated art when available.

---

# 25. CLASS EQUIPMENT RESTRICTIONS

Keep class restrictions intuitive and data-driven.

Suggested initial restrictions:

## Warrior
Weapons:
- dagger
- sword
- broadsword
- axe
- hammer/mace
- spear
- shield/off-hand where available

Armor:
- leather and heavier armor through ornate plate

## Mage
Weapons:
- dagger
- mage staff

Armor:
- cloth
- mage robes

## Ranger
Weapons:
- dagger
- sword
- spear
- bow
- crossbow

Armor:
- leather
- studded leather
- chain
- light scale where balance permits

## Cleric
Weapons:
- hammer/mace
- holy staff

Armor:
- cloth
- priest robes
- chain
- modest medium armor where appropriate

## Rogue
Weapons:
- dagger
- sword
- bow
- crossbow

Armor:
- cloth
- leather
- studded leather

## Paladin
Weapons:
- sword
- broadsword
- hammer/mace
- spear
- holy staff
- shield/off-hand where available

Armor:
- chain through ornate plate

## Warlock
Weapons:
- dagger
- mage staff

Armor:
- cloth
- mage robes

These restrictions are configuration data, not hard-coded condition spaghetti.

---

# 26. EQUIPMENT QUALITY

Exactly five quality tiers:

1. Worn
2. Standard
3. Fine
4. Masterwork
5. Legendary

Quality increases the item's normal stats.

Do not build random affix soup.

Do not generate dozens of suffixes and prefixes.

A simple quality multiplier is sufficient.

Suggested initial tunable multipliers:

- Worn: 0.80x
- Standard: 1.00x
- Fine: 1.12x
- Masterwork: 1.28x
- Legendary: 1.50x

Tune during playtesting.

## UI Quality Indicator

Use a subtle quality indicator:

- thin outline
- small quality icon
- restrained text treatment

Do not use loud Diablo-style rainbow loot presentation.

---

# 27. FOOD AND POTIONS

There is **no hunger meter**.

Food is simply:

- healing
- rest support
- temporary minor enhancement where appropriate

Potion categories should remain small.

At minimum:

- Health Potion
- Greater Health Potion
- Mana Potion
- Greater Mana Potion

Optional simple utility consumables are acceptable if they serve an existing status system.

Do not create a huge alchemy catalog.

---

# 28. FIRST-PERSON DUNGEON MOVEMENT

Dungeon movement is strict old-school grid movement.

Controls:

- `W` = move forward one grid tile
- `S` = move backward one grid tile
- `A` = rotate exactly 90 degrees left
- `D` = rotate exactly 90 degrees right

Additional controls:

- `E` = interact
- `M` = automap
- `I` = inventory
- `C` = character sheet
- `J` = hunt / quest journal
- `Esc` = pause/menu
- `F5` = quicksave
- `F9` = quickload

Mouse is used for:

- menus
- town hotspots
- combat cards/actions
- inventory
- dialogue choices
- map interaction where useful

There is no normal free-look FPS movement in the dungeon.

Use snappy transitions.

A very short ease/tween between grid positions and 90-degree turns is acceptable, but the player must always end perfectly snapped to the grid and canonical facing.

---

# 29. DUNGEON GRID AND MODULAR KIT

Use one consistent grid scale across the game.

Build modular pieces such as:

- straight corridor
- left/right corner
- T-junction
- four-way junction
- dead end
- small room
- large room
- stairs
- doorway
- locked doorway
- iron gate
- secret wall
- arch
- shrine alcove
- chest area
- trap tile
- lever
- pressure plate

Do not model each entire floor in Blender as one giant mesh.

Prefer:

1. modular pieces authored in Blender
2. clean pivots and grid snapping
3. region-specific material/decor sets
4. handcrafted floor layout data in Unreal

The primary floor topology is handcrafted.

Do not procedurally generate the main dungeon layouts at runtime.

---

# 30. DUNGEON STRUCTURE

The dungeon progresses linearly by major region, with optional branches and secrets.

Order:

## Prologue
**Last Dawn Cathedral**

Then:

1. **Old City Sewers**
2. **Forgotten Catacombs**
3. **Goblin Warrens**
4. **Ancient Crypts**
5. **Buried Fortress**
6. **The Deep**
7. **Infernal Ruins**
8. **Hell**

Players cannot skip the main region progression.

Within each level include:

- optional rooms
- branches
- shortcuts
- locked areas
- secrets
- traps
- side-hunt locations
- occasional lore
- occasional mini-bosses

---

# 31. FLOOR COUNT AND CAMPAIGN TARGET

Campaign target:

**15-25 hours**

Keep individual floors compact rather than sprawling.

Recommended normal floor size:

approximately **15x15 to 30x30 grid cells**

Initial content target:

- Last Dawn Cathedral: 1 tutorial/prologue level
- Regions 1-7: approximately 2 meaningful levels each
- Hell: approximately 3 levels / final-act spaces

This is a starting content target, not a requirement to inflate empty floors.

Prefer dense, memorable floors over large empty maps.

Each floor should have a name and region identity.

Example style:

**Forgotten Catacombs — Level 2: The Ossuary**

Use both:

- numbered level identity
- named area identity

---

# 32. LAST DAWN CATHEDRAL

Last Dawn Cathedral is the first dungeon environment and tutorial space.

Teach naturally:

- movement
- turning
- interaction
- opening doors
- picking up loot
- basic combat
- automap
- first trap
- first key
- first waypoint

Avoid excessive tutorial popups.

Use brief contextual prompts.

The player should feel that the real descent begins beneath the Cathedral.

---

# 33. WAYPOINTS

Waypoint visual:

**Ancient stone shrines**

Use existing shrine artwork/reference assets when available.

Each meaningful dungeon level should normally have two waypoints:

1. one near the start
2. one near the end

Do not scatter waypoints everywhere.

Waypoints are teleport destinations.

They are not automatically full-heal stations unless explicitly combined with a rare safe/rest area.

## Teleport Rules

Outside combat, the player may freely choose:

**Teleport to Town**

No scroll.
No potion.
No consumable cost.

When returning from town:

- the player returns to an activated waypoint
- never directly to the exact arbitrary tile they teleported from

Allow the player to select among discovered waypoints where useful.

---

# 34. RESTING IN THE DUNGEON

Normal resting is not allowed everywhere.

Dungeon rest areas are:

- rare
- sometimes secret
- clearly special

Examples:

- hidden sanctuary
- protected camp
- ancient safe room
- purified chapel
- rare shrine chamber

Do not let the player sleep after every fight.

---

# 35. AUTOMAP

The player has an automatic map.

It reveals:

- explored grid cells
- known doors
- known stairs
- activated waypoints

It does **not** reveal undiscovered:

- secret walls
- secret rooms
- hidden passages
- hidden treasure
- undiscovered traps

Once a secret is found, add it to the map appropriately.

---

# 36. SECRETS

Players love secrets.

Each dungeon level should contain roughly:

- 1-2 secret rooms

Each major region should contain:

- 1 **super secret** room with a substantially better reward

Super-secret rooms should feel special.

Potential rewards:

- unusually strong equipment
- large gold cache
- rare consumables
- permanent stat bonus
- rare lore
- rare healing fountain
- hidden rest area
- optional mini-boss
- special antiquity

Do not mark secret doors with obvious glowing arrows.

Give observant players reasonable visual/audio clues.

---

# 37. PUZZLES

Most puzzles are simple.

Primary puzzle types:

- levers
- keys
- pressure plates
- rotating statues
- door sequences
- light/rune matching
- simple environmental clues

Occasional moderate multi-room puzzles are acceptable.

Do not turn the game into a puzzle adventure.

---

# 38. TRAPS

Use a combination of:

- player-observable visual clues
- character-stat/passive detection

Rogue passive provides improved detection.

Possible traps:

- pressure plates
- dart traps
- blade traps
- collapsing floor
- poison vents
- magical rune traps
- fire jets

Allow interaction/disarming when appropriate.

Do not make traps unavoidable random damage.

---

# 39. KEYS

The key system includes at least these six named key types:

1. Rusted Key
2. Crypt Key
3. Warden Key
4. Master Key
5. Drake Lair Key
6. Hell Key

Keys are key items and use no inventory slots.

## Key Roles

### Rusted Key
Common early/simple dungeon lock.

### Crypt Key
Used in catacomb/crypt content.

### Warden Key
Used in fortress/prison content.

### Master Key
A rare regional key that can open most normal locked doors in a specific region.

Not every region has a Master Key.

### Drake Lair Key
Used for a major optional or required drake location in The Deep.

### Hell Key
A major late-game story item connected directly to Astra.

On the good ending, obtaining the Hell Key from Astra can also serve as the logical postgame access token for persistent Hell/Hell Hunts.

---

# 40. ENEMY ENCOUNTERS

Enemies are visible in the 3D dungeon before combat.

No random invisible encounter screen.

Combat begins when the player and a hostile enemy group come into encounter range or a scripted ambush triggers.

Normal maximum enemies in one encounter:

**6**

Enemy positioning on screen is visual only.

There are:

- no enemy front rows
- no enemy back rows
- no party front rows
- no party back rows
- no formation-swapping mechanics

Groups can be spread across visual anchor points so six enemies remain readable.

Do not assign tactical row meaning to those visual positions.

---

# 41. TURN-BASED COMBAT

Combat remains first-person.

When combat starts:

- grid movement is locked
- enemy group remains presented in front of the player
- party character cards appear across the bottom
- initiative order is calculated
- combat proceeds turn-by-turn

Turn order mixes heroes and enemies according primarily to:

- Dexterity
- Speed / Initiative modifiers
- small random tie-breaking where necessary

Dexterity must meaningfully affect attack order.

## Universal Actions

Every hero has:

- Attack
- Defend
- Skill
- Item
- Flee

There is no Swap Position action.

## Basic Attack

Every class has a basic attack.

The equipped weapon determines normal attack statistics/presentation.

Class uniqueness primarily comes from the three class skills.

---

# 42. COMBAT CARD UI

The party cards remain across the bottom of the screen.

The current acting hero's card:

- highlights
- expands or visually elevates
- exposes available actions

Each card should show at minimum:

- portrait
- name
- class
- HP
- MP
- major status icons
- dead/disabled state

Keep the cards stable in their normal party order rather than constantly reordering them by initiative.

If useful, show a small turn-order strip elsewhere.

Do not let the combat HUD cover most of the monster artwork.

---

# 43. COMBAT PRESENTATION

Keep combat animation intentionally simple.

For physical attacks use combinations of:

- slash overlay
- thrust overlay
- impact flash
- small camera shake
- hit sound
- damage number
- color variation based on weapon/damage type

Do **not** show the player's weapon model in first person.

No sword arms.
No bow arms.
No giant animation library.

For magic use simple VFX:

- fire burst
- ice shard
- lightning arc
- holy flash
- dark pulse
- poison effect

Enemy attack presentation can use:

- slight sprite/model lunge
- shake
- flash
- hit effect
- sound
- status icon

This is a 1990s-style crawler presentation with modern polish, not an action game.

---

# 44. ACCURACY, CRITICALS, AND FLEEING

Attacks can miss.

Misses should be uncommon.

Dexterity, equipment, enemy evasion, and class effects may influence accuracy.

Critical hits exist.

Critical chance is influenced modestly by:

- Dexterity
- Luck
- class passive/equipment

Avoid extreme random streaks.

## Flee

Flee can be attempted during normal combat.

Use a simple party-based chance influenced by average Dexterity/initiative.

On successful flee:

- combat ends
- player retreats safely to the prior navigable grid tile or a deterministic nearby safe tile

Boss and major scripted fights may disable fleeing.

---

# 45. DAMAGE TYPES

Use a small elemental/damage system:

- Physical
- Fire
- Ice
- Lightning
- Poison
- Holy
- Dark

Enemies may have:

- weakness
- normal resistance
- strong resistance
- immunity only when thematically justified

Keep the values readable and data-driven.

Do not create 20 elements.

---

# 46. STATUS EFFECTS

Use a controlled status list.

Initial recommended set:

- Poison
- Bleed
- Burn
- Stun
- Blind
- Curse
- Fear

Add only a small number of positive combat effects where class skills require them.

Do not build a giant condition encyclopedia.

Status icons should be clear.

---

# 47. ENEMY FAMILIES BY REGION

Use existing artwork from:

`J:\First Person Dungeon Crawler Game\.art\monsters`

where available.

The following families are approved.

## Last Dawn Cathedral

- rats
- spiders
- cultists
- undead

## Old City Sewers

- fungal rats
- sewer beetles
- slimes
- diseased humanoids

## Forgotten Catacombs

- skeletons
- zombies
- ghosts
- necromancers

## Goblin Warrens

- goblins
- goblin archers
- shamans
- trolls

## Ancient Crypts

- armored undead
- wraiths
- gargoyles
- vampires

## Buried Fortress

- corrupted soldiers
- war beasts
- constructs
- dark knights

## The Deep

- cave horrors
- drakes
- giant insects
- aberrations

## Infernal Ruins

- imps
- succubi
- incubi
- demons
- hell knights

## Hell

- greater demons
- fallen angels
- infernal beasts
- principalities
- devils

Multiple variants within a family are encouraged when existing art supports them.

If a needed variant is missing, add it to `_MISSING_ART_LIST.md`.

---

# 48. ENEMY RESPAWN AND LOOT

Normal enemies eventually respawn.

Unique enemies and bosses do not.

Use a simple deterministic respawn rule based on one of:

- leaving/re-entering a region after sufficient progress
- elapsed game time
- a small number of dungeon transitions

Do not respawn enemies immediately after the player steps away.

## Chests

Normal treasure chests:

- do not refill
- do not respawn

Enemy drops remain the renewable source of:

- gold
- potions
- equipment
- postgame grinding rewards

---

# 49. REGION BOSSES AND MINI-BOSSES

Every major region has a major boss.

Use occasional mini-bosses for:

- recruit events
- major keys
- secrets
- hunts
- rare loot
- waypoint or progression gates

## Major Boss Presentation

Each major boss should have:

- unique artwork/presentation
- unique or distinct music cue
- increased HP
- 2-4 signature attacks
- simple phase adjustment around 50% HP where thematically useful
- clear victory event

Do not make bosses Souls-like.

No dodge-window choreography.
No real-time pattern memorization.
No action-combat phases.

Bosses remain turn-based.

---

# 50. DUNGEON CHANGES AFTER BOSSES

Major boss victories should sometimes visibly affect the world.

Examples:

- sealed descent opens
- corrupted barrier disappears
- bridge extends
- elevator activates
- shortcut unlocks
- new NPC dialogue appears in Lonemoore
- a new hunt category becomes available
- environmental corruption changes
- next region becomes accessible

Keep these scripted and simple.

Do not require new town artwork for every change.

Use dialogue, UI state, simple overlays, and existing assets when practical.

---

# 51. HUNT BOARD — NORMAL GAME

The Hunt Board is the primary side-quest system.

Supported hunt templates:

- kill named monster
- kill X enemies of a family
- find a target
- retrieve a relic
- hunt a mini-boss
- rare boss hunt

Maximum active hunts:

**5**

The board should also maintain a small persistent set of offered hunts.

A hunt offer remains visible until the player:

- accepts it
- declines it

When a hunt slot becomes available after a completion/decline, generate one replacement offer.

Do not refresh the whole board every time the player opens it.

Hunts should feel persistent rather than like a slot machine.

Rewards:

- gold
- XP
- occasional gear
- occasional rare antiquity

Use data-driven templates.

---

# 52. HELL HUNTS — POSTGAME

Hell Hunts unlock only if the player chooses the good ending:

**Kill Astra.**

After the good ending:

1. play the final victory cutscene
2. roll credits
3. play a secondary stinger showing that the Hell portal is still not fully closed
4. return to postgame
5. the old Hunt Board catches fire / burns away
6. normal hunts are replaced by **HELL HUNTS**

Hell Hunts take place in Hell.

Hell Hunt templates include:

- kill X demons
- kill rare demon variant
- kill named demon
- kill demon mini-boss
- locate and destroy a powerful infernal target
- hunt exceptionally difficult Hell bosses

Hell Hunts provide:

- high XP
- high gold
- Masterwork/Legendary gear opportunities
- long-term level-99 progression
- repeatable endgame challenge

Do not make Hell Hunts a separate live-service system.

It is simply the postgame hunt board.

---

# 53. STORY STRUCTURE

Keep the main story clear and compact.

## Act 1 — The Call

- protagonist is drawn to Lonemoore
- town rumors point to Last Dawn Cathedral
- Cathedral introduces the dungeon
- early evidence shows the corruption is deeper than expected

## Act 2 — The Descent

- Sewers and Catacombs show that the city has been built over layers of forgotten history
- first companions are recruited
- undead/cult activity hints at a deeper breach

## Act 3 — Buried Civilizations

- Warrens, Crypts, and Fortress reveal that previous cultures tried to contain or exploit what lies below
- more companions are found
- ancient keys and sealed routes become important

## Act 4 — The Deep

- drakes, aberrations, and impossible phenomena reveal that the dungeon is no longer entirely part of the mortal world
- the player learns the portal is real

## Act 5 — Infernal Descent

- Infernal Ruins transition the architecture and enemy population toward Hell
- the Warlock helps interpret the breach and Astra's nature
- the party enters Hell

## Final Act — Astra

The party reaches Astra the Cosmic Chaos Nephilim Queen.

She is central to the portal and the Hell Key.

The final encounter presents a direct choice.

Avoid adding ten political factions or convoluted cosmology.

---

# 54. FINAL BOSS — ASTRA THE COSMIC CHAOS NEPHILIM QUEEN

Astra is an adult Nephilim offspring of an angel and demon.

Use existing approved artwork if present.

Do not regenerate or replace her art without user approval.

Core visual identity:

- tall
- pale
- extremely lean and scarred
- powerful mage and swordswoman
- one radiant blue eye
- one burning red eye
- very long hair combining black and ice-blue with fiery red tips
- one enormous radiant angelic wing
- one enormous dead-drake/demonic wing
- breath manifests as unnatural cold
- rage manifests as fire
- a long black infernal tail with a lethal pointed end
- broken halo resembling a cracked void in space
- pale yellow lightning around the halo
- footsteps leave burning-hot ice
- carries the Hell Key around her neck
- carries an exaggerated two-handed broadsword
- sword mixes angelic and demonic runes
- hilt crystal glows pure white with black lightning

Her intersex physiology is part of the lore.

Any depiction must remain **non-explicit** and use the already approved character artwork as the visual source of truth.

Do not turn this into pornographic or explicit content.

## Boss Combat Identity

Astra combines:

- sword damage
- fire
- ice
- holy
- dark/void effects

Keep her turn-based.

Use 2-4 signature attacks and one simple phase shift around half HP.

Do not make the fight Souls-like.

---

# 55. FINAL CHOICE AND ENDINGS

Present the final choice during the confrontation with Astra.

## Ending 1 — Reject Astra / Kill Her

The player rejects Astra's offer.

Combat proceeds.

The party kills Astra.

Then:

1. play final victory scene
2. show consequences in Lonemoore
3. roll credits
4. show post-credit/stinger scene
5. reveal the Hell portal remains accessible
6. unlock Hell Hunts
7. return the player to postgame Lonemoore with the Hell Hunt system active

The Hell Key may become the logical persistent access key after Astra's death.

## Ending 2 — Side With Astra

The player accepts Astra's offer.

The companions reject the betrayal.

Trigger a special final encounter in which:

- the chosen protagonist is temporarily empowered by Nephilim/Hell power
- the four companions oppose the protagonist
- the protagonist fights the former party

If the protagonist wins:

- the companions die
- Lonemoore burns
- the town is destroyed
- the protagonist takes control of the portal
- the protagonist becomes the new Hell Lord
- Astra's power becomes part of the protagonist's ascension
- play the dark ending cutscene
- roll credits
- no Hell Hunts postgame is unlocked from this ending

If the protagonist loses this special betrayal fight:

- Game Over
- allow load/retry from the pre-choice save/autosave

Do not build a third ending unless the user explicitly requests one later.

---

# 56. DEATH — NORMAL MODE

## Individual Party Member Death

If one hero dies but the remaining party wins:

- the party keeps its loot
- the dead hero remains dead
- the player may continue temporarily
- the player may teleport to Lonemoore
- the Healer can resurrect the dead hero for gold
- Cleric Resurrection may also provide an in-dungeon recovery option when available

No loot is dropped merely because one companion died and the party won.

## Full Party Wipe

If the entire active party dies:

- Normal mode attempts town resurrection
- resurrection requires enough gold
- if the player cannot pay the required resurrection cost: **Game Over**

To preserve the previously selected corpse-recovery concept without overcomplicating it:

- create one recovery bundle/corpse marker at the wipe location
- key items are never dropped
- gold is never placed in the corpse bundle
- recoverable normal loot/equipment from the wipe can be stored in that one bundle
- do not create five separate corpse inventories

If the protagonist is still alone early in the game and dies, use the same basic normal-mode resurrection rule.

---

# 57. HARDCORE / PERMADEATH

Hardcore is optional.

In Hardcore:

- dead characters do not resurrect
- Healer resurrection is disabled
- Cleric Resurrection cannot undo permanent death
- companion death permanently reduces the party
- protagonist death is Game Over
- full party wipe is Game Over

Keep the rest of the game's mechanics the same.

Do not rebalance the entire game into a separate difficulty mode unless necessary.

---

# 58. SAVING

Allow saving anywhere outside combat.

Required:

- autosave
- manual save
- quicksave with `F5`
- quickload with `F9`

Suggested autosave moments:

- entering Lonemoore
- entering a new dungeon level
- activating a waypoint
- defeating a major boss
- recruiting a companion
- immediately before the final Astra choice

Do not allow saving during active combat.

Use a versioned SaveGame schema so future changes do not immediately destroy saves.

Keep save data straightforward.

---

# 59. BLACKSMITH DURABILITY / REPAIR

If equipment durability is implemented, keep it extremely simple.

Acceptable:

- one durability value
- equipment effectiveness does not need a complex degradation curve
- Blacksmith restores durability for gold

If durability adds more annoyance than gameplay value during implementation, it may be omitted and the Blacksmith can focus on quality upgrades.

Do not build an elaborate repair-material economy.

---

# 60. RANDOMIZATION RULES

The game is not a fully procedural dungeon crawler.

Handcraft:

- region order
- floor topology
- boss placement
- companion recruitment
- major keys
- waypoints
- super secrets
- main story events

Randomize selectively:

- normal enemy group composition
- some spawn-node activation
- normal enemy drops
- some chest contents
- hunt-board offers
- rare enemy variants
- some optional events

Never randomize the game into an unwinnable state.

---

# 61. AUDIO DIRECTION

Use original retro-inspired audio.

Target feel:

- 8-bit / 16-bit NES/SNES-era game music
- dark fantasy
- memorable loops
- simple melodic hooks
- compact sound effects

Needed categories:

- main menu music
- Lonemoore ambience/music
- Cathedral ambience
- one ambience palette per major region
- standard combat music
- boss music
- final boss music
- Hell Hunt/postgame music variation
- ending music
- UI sounds
- sword/slash sounds
- bow/crossbow sounds
- magic sounds
- damage sounds
- chest/door/lever/key sounds
- shrine/teleport sounds

Do not copy melodies from copyrighted games.

If no final audio exists, create original simple placeholder audio or integration hooks rather than scraping copyrighted assets.

---

# 62. UI SCREENS

At minimum implement:

## Main Menu
- New Game
- Continue
- Load Game
- Settings
- Credits
- Quit

## Character Selection
- Warrior card
- Mage card
- Ranger card
- difficulty toggle

## Town Hub
- main square art
- hover hotspots
- location labels
- location interaction

## Dungeon HUD
- first-person dungeon view
- party cards
- compact HP/MP
- active status indicators
- contextual interact prompt
- optional small minimap only if it does not undermine the full automap

## Combat HUD
- party cards
- active card expansion/highlight
- Attack
- Defend
- Skill
- Item
- Flee
- enemy HP presentation where appropriate
- turn order indicator if useful

## Inventory
- combined party loot-bag view
- visible 20-slot capacity per hero
- equipment comparison
- simple move/use/equip/drop actions

## Character Sheet
- level
- XP
- class
- six attributes
- derived stats
- three class skills
- passive
- equipment

## Automap
- explored dungeon map
- doors
- stairs
- waypoints
- discovered secrets

## Hunt Journal
- up to five active hunts
- completion state
- rewards

## Pause
- Resume
- Save
- Load
- Settings
- Main Menu
- Quit

## Ending / Credits
- ending scene/panels
- scrolling credits
- post-credit stinger for good ending

---

# 63. DATA-DRIVEN CONTENT

Use lightweight structured content data for:

- classes
- skills
- items
- equipment restrictions
- item qualities
- enemies
- bosses
- encounter groups
- regions
- levels
- loot tables
- hunt templates
- dialogue
- waypoints
- keys
- town services

Do not hard-code every monster and item into branching C++ code.

At the same time, do not invent an enterprise content-management framework.

A small set of DataTables / Data Assets is enough.

---

# 64. SUGGESTED UNREAL CONTENT ORGANIZATION

Keep the project organized approximately like:

`Content/Game/Core`

`Content/Game/UI`

`Content/Game/Data`

`Content/Game/Characters`

`Content/Game/Enemies`

`Content/Game/Items`

`Content/Game/Maps/Town`

`Content/Game/Maps/Dungeon`

`Content/Game/Environment`

`Content/Game/Materials`

`Content/Game/VFX`

`Content/Game/Audio`

`Content/Game/ImportedArt`

`Content/Game/Cinematics`

Do not rename user source artwork in `.art` destructively.

---

# 65. BUILD ORDER

Implement the game in a vertical-slice-first sequence.

## Stage 1 — Preflight

- inspect the existing project
- inspect installed Unreal/Blender tooling
- inspect `.art` recursively
- identify existing code and content
- create lightweight asset manifest
- create/update `BUILD_STATUS.md`
- create `_MISSING_ART_LIST.md` only if needed

Do not delete existing work.

## Stage 2 — Core Shell

Build:

- Unreal project boot
- main menu
- save framework
- settings shell
- character selection
- loading into Lonemoore

## Stage 3 — Town Vertical Slice

Build:

- Main Town Square
- hotspot interaction
- one working shop
- Healer
- Dungeon Entrance
- return-to-square behavior

Then expand to all town services.

## Stage 4 — Dungeon Vertical Slice

Build Last Dawn Cathedral with:

- modular 3D kit
- grid movement
- 90-degree turning
- doors
- interaction
- chest
- first enemy group
- turn-based combat
- loot
- first key
- trap
- automap
- start/end shrine
- teleport town/out-and-back loop

This vertical slice must prove the complete core loop before mass-producing floors.

## Stage 5 — RPG Systems

Complete:

- attributes
- XP
- level-up
- 3 assignable points per level
- class skills
- passives
- HP/MP recovery
- inventory
- equipment
- quality tiers
- merchants
- blacksmith
- healer
- consumables

## Stage 6 — Party and Recruitment

Implement:

- party cards up to 5
- recruit events
- level matching
- dialogue barks
- resurrection behavior
- shared XP behavior

## Stage 7 — Region Production

Build all major regions in order:

- Sewers
- Catacombs
- Warrens
- Crypts
- Fortress
- Deep
- Infernal Ruins
- Hell

For each region:

- materials/lighting identity
- approximately two meaningful floors unless design needs differ
- enemy families
- start/end shrines per floor
- secrets
- one super secret per region
- keys/puzzles
- mini-bosses
- region boss
- post-boss world-state change

## Stage 8 — Hunts

Implement:

- normal Hunt Board
- 5 active-hunt cap
- persistent offers
- hunt generation
- reward turn-in
- rare boss hunts

## Stage 9 — Final Act

Implement:

- Astra encounter
- pre-choice autosave
- good ending
- Astra boss fight
- victory cutscene
- credits
- portal stinger
- Hell Hunt transformation

Then implement:

- side-with-Astra choice
- empowered protagonist
- party betrayal fight
- dark ending
- credits
- no Hell Hunts

## Stage 10 — Postgame

Implement Hell Hunts:

- repeatable difficult hunts
- rare/named demons
- boss hunts
- high XP/gold
- top-tier loot
- progression toward level 99

## Stage 11 — Polish and Packaging

- fix collision
- fix grid snapping
- test every door/key
- test every waypoint
- test every save/load path
- test all three starting heroes
- test all four recruitments
- test both endings
- test Normal and Hardcore
- test Hell Hunts
- remove accidental dev art from shipping
- verify missing-art list
- package Windows build
- run packaged-build smoke test

---

# 66. DEFINITION OF DONE

The project is not considered complete merely because core systems exist.

A complete build must allow a fresh player to:

1. Launch the packaged game.
2. Reach the main menu.
3. Start a new game.
4. Choose Warrior, Mage, or Ranger.
5. View the intro panels.
6. Arrive in Lonemoore.
7. Use town hotspots.
8. Shop/rest/heal.
9. Enter Last Dawn Cathedral.
10. Move on the grid with WASD.
11. Interact with doors, chests, traps, keys, and shrines.
12. Fight enemies in turn-based first-person combat.
13. Gain XP and levels.
14. Assign 3 attribute points each level.
15. Use class skills.
16. Equip weapons and armor.
17. Manage the 20-slot-per-character inventory.
18. Teleport to town outside combat.
19. Return through activated waypoints.
20. Explore secrets and super secrets.
21. Accept and complete hunts.
22. Recruit Cleric.
23. Recruit Paladin.
24. Recruit Rogue.
25. Recruit Warlock.
26. Defeat every required region boss.
27. Reach Hell.
28. Meet Astra.
29. Complete the good ending.
30. View credits and the Hell portal stinger.
31. Return to postgame and access Hell Hunts.
32. Load a pre-choice save.
33. Complete the dark side-with-Astra ending.
34. View its ending and credits.
35. Save/load/quick-save/quick-load reliably.
36. Complete the game without developer intervention.

---

# 67. QA REQUIREMENTS

At minimum test:

## Movement
- no half-grid states
- no incorrect 90-degree facing
- no movement through closed doors
- no getting trapped by transitions

## Combat
- 1 through 6 enemies
- all party sizes 1 through 5
- dead-character handling
- initiative ordering
- class skills
- all damage types
- all status effects
- flee behavior
- boss no-flee rules
- victory/defeat transitions

## Inventory
- full inventory
- item transfer
- equip restrictions
- consumable stacking
- key items
- gold
- quality upgrades

## Save
- manual save
- autosave
- F5
- F9
- save after recruit
- save after boss
- save before final choice
- load postgame
- load Hardcore game

## Story
- no recruit can become permanently missed by accident
- all progression keys obtainable
- no boss blocks progression incorrectly
- both endings reachable
- Hell Hunts only unlock from good ending

---

# 68. DOCUMENTATION TO MAINTAIN

Keep documentation minimal.

Maintain only what is useful:

## `BUILD_STATUS.md`

Short current state:

- working
- next
- blocked
- important assumptions

## `_MISSING_ART_LIST.md`

Simple missing-art list as defined earlier.

## `KNOWN_ISSUES.md`

Only real unresolved issues.

Do not create dozens of planning documents unless needed.

---

# 69. IMPORTANT DESIGN GUARDRAILS

Before adding any new feature, ask:

**Is it explicitly required by this specification?**

If no:

- do not add it unless it is a tiny implementation necessity

Remember:

- No Souls-like anything.
- No free-look dungeon FPS.
- No action combat.
- No front/back party rows.
- No party-switching.
- No crafting.
- No hunger.
- No skill trees.
- No respec.
- No randomized affix garbage.
- No giant storage system.
- No town open world.
- No procedural dungeon topology.
- No huge cinematic burden.
- No silent art generation.
- No replacing approved art.

The game's strength should come from:

- atmosphere
- dungeon layout
- secrets
- satisfying exploration
- clean turn-based combat
- party growth
- loot progression
- boss fights
- the descent toward Hell
- the final choice
- Hell Hunts

---

# 70. FINAL INSTRUCTION TO THE DEVELOPMENT AGENT

Begin by inspecting:

`J:\First Person Dungeon Crawler Game`

and recursively inspecting:

`J:\First Person Dungeon Crawler Game\.art`

Do not assume the project is empty.

Reuse existing approved work.

Do not delete or replace user-created assets without a clear reason.

Do not generate new art silently.

Build the smallest complete version of every required system first, then expand content.

Compile and test frequently.

Do not stop at a design document or prototype.

Carry the project from:

**Main Menu → Character Selection → Lonemoore → Last Dawn Cathedral → Eight Dungeon Regions → Hell → Astra → Both Endings → Credits → Hell Hunts → Packaged Windows Build.**

The intended finished product is:

> **A simple, polished, atmospheric, old-school first-person dungeon crawler RPG with modern 3D dungeon art and Unreal lighting.**

That is the game.

Do not turn it into something else.
