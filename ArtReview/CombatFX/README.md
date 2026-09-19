# Combat sprite effects

Original 96 x 96 pixel artwork inspired by the timing and limited palettes of
16-bit console RPG combat. No artwork from Final Fantasy or another game is used.

- `core-effects.png`: eight animation frames for the seven requested core effects.
- `contact-sheet.png`: all 30 effect sequences, one row per cue.
- `<cue>.png`: transparent eight-frame strip (768 x 96).
- `<cue>.gif`: looping enlarged animation on a dark review background.

The runtime uses the exact same indexed pixels, encoded as horizontal runs in
`Source/DungeonCrawler/DungeonCombatVFXData.inl`. `DungeonCombatVFX::Paint` renders
one batched Slate draw per active effect, so no texture import or asset cooking is
required. The user-facing game does not load these review files.

Rebuild these assets and runtime data with `Tools/generate_combat_vfx.py` using a
Python installation with Pillow. Normal effects last 0.8 seconds, projectile
effects 0.9 seconds, and critical slashes 0.7 seconds. Last frames fade out.
`guard`, `miss`, and `escape_fail` reuse the defend or dodge animations.

Generator QA: 30 cues, 240 nonempty frame records, maximum 508 scanline runs per
frame (2,032 vertices). Some shield, targeting, and lightning frames deliberately
hold/repeat within their eight-frame timelines. The core contact sheet was
visually inspected. Gameplay placement and packaged behavior require the
integration build's review.
