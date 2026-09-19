# Phase 2 follow-up refinements

- HP/MP numerals reduced to fit within the colored bar interiors.
- Side inscriptions centered, uppercase and shaded as incised stone lettering.
- Inventory double-click uses a consumable from the clicked hero's bag.
- Recruitment refreshes the dungeon and suppresses portraits for recruited classes.
- Rogue Small Grenade replaces Smoke Bomb: physical damage to every enemy, 8 MP, level 10.
- Combat keys: 1 Attack, 2 Defend, 3 Health, 4 Mana, 5 Flee; F1/F2/F3 specials. Existing turn locks apply.
- Critical, divine, warlock, arcane and grenade sound cues are separate original synthesized effects.
- Physical specials scale with Attack; magic specials use Magic + half Attack, preserving caster attributes while benefiting from weapons. Existing skill multipliers apply.
- Equipment rewards select from equipment usable by at least one party member, including armor and accessories, instead of the lead class's first weapon family.
- Exit movement and interaction both require the exact floor key and the actual guardian's defeat record. A miniboss flag alone cannot unlock the descent.
- New games vary dead-end passages, encounter/supply/trap placement and map orientation. Objectives and structural gates are retained. Generated rows persist in saves; older saves retain their original layouts.

Validation: 2,126 content checks passed. All three Dungeon.Phase2 test groups passed, including randomized layout save/load. The 3440x1440 editor UI review passed actual potion double-click and combat hotkey checks; bar text and centered inscriptions were visually reviewed. Main Windows build and cooking succeeded, including all five new sound assets. Dungeon 3D artwork remains Phase 3.

Packaged main-game review also passed potion double-click, combat hotkey and all UI review steps without runtime errors. The latest Auto, BeforeAstra, Manual and Quick saves remain unchanged after installation/review; latest display settings restored. Logs: Saved/Validation/refinements_tests.log, refinements_ui.log, refinements_main.log. Build workaround: game target compiled with -NoPCH after a stalled shared-header load; explicit texture, mesh and sound includes are now present.
