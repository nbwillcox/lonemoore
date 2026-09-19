# Inventory and status update

Inventory opens as a parchment sheet over the current dungeon view. All party bags remain visible, with the normal party cards below. The X, I key, and Escape close the inventory. Gold sits at the sheet's lower right. The old title, town image, notice feed, and redundant navigation are hidden while the bag is open. Use, equip, both accessory slots, transfers, discard confirmation, double-click use, and unclaimed guardian rewards remain available.

Small status badges overlay character portraits. Hover a badge for its effect and remaining turns.

| Effect | Icon | Enemy examples | Behavior |
|---|---|---|---|
| Poison | Skull and crossbones | Cave Spider, Reaper Drake | Damage each turn |
| Bleeding | Blood drop | Vampire, Archer, Hound, Bat | Damage each turn |
| Burning | Flame | Infernal Enforcer, Agni Drake | Damage each turn |
| Plague | Red cross | Fungal Rat, Rat King | Damage each turn; halves healing |
| Slow | Snowflake | Slime, Widow Queen | Halves initiative |
| Weakness | Down arrow | Cursed Rat, Succubus | Reduces outgoing damage by 20% |
| Silence | Crossed speech bubble | Shaman, Gatekeeper | Prevents skill use |
| Stun | Lightning bolt | Brute, Minotaur, Gargoyle | Skips one turn |
| Fear | Frightened face | Wraith, Castellan, Keeper | Skips one turn |
| Blind | Crossed eye | Bandit | Reduces accuracy |
| Curse | Rune | Witch, Matriarch | Reduces outgoing damage by 30% |
| Mark | Target | Hero skills | Increases enemy damage taken by 30% |
| Guard | Shield | Defend and protection skills | Reduces incoming damage |

Temporary conditions expire at the start of affected actors' turns, over up to three turns; damage effects tick three times. Stun and Fear skip one turn. Reapplication refreshes duration instead of stacking identical effects. Victory and escape clear temporary conditions. Cleansing tonics remove ailments while preserving Guard; town healing/rest also removes conditions. The existing saved status map format is retained.

Validation: `Tools/Build.ps1 -Test -Package`, followed by `Tools/Review-InventoryStatus.ps1` for packaged interaction and screenshot checks. Results are recorded in `Saved/InventoryStatusReview`.

## Verified 2026-09-18

- All 37 Dungeon automation suites passed, including status effects, both accessory slots, legacy saves, and complete campaign routes/endings.
- Windows package built successfully; final executable hash matches `Binaries/Win64/DungeonCrawler.exe`.
- Packaged reviews passed at 1600x900 and 3440x1440 with zero failures: five bags, item use, accessory equipment, transfers, discard cancellation, party selection, X close, I toggle, and Escape close.
- Final screenshots inspected at both resolutions, including the 13-badge crowding fixture. Parchment contrast was corrected after the first visual review.
- Eight pre-existing save files remain byte-identical. Hash results and backup are in `Saved/InventoryStatusReview`.
- Campaign test drivers now select basic attacks while silenced; progression assertions remain unchanged.

Launch normally with `Play Lonemoore.cmd` and press I in the dungeon. The normal launcher uses the updated packaged game.
