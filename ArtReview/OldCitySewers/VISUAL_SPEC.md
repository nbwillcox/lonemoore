# Old City Sewers — sample v01

STATUS: Staged art only. Visual approval required before integration.

## Authority
Existing campaign regionIndex 1, Old City Sewers: The Drowned Conduit and The Rat King's Cistern. Shared lore: “The old channels carry no rain. Every drop seeps upward from below.” DUNGEON AND LOCATION LIST describes arched drainage passages, shallow murky water, leaking pipes, and a larger underground reservoir. DUNGEON NAVIGATION ARTWORK identifies brick, runoff, slime, and rusted pipes/grates.

The actual approved `.art/.concepts/sewer-tunnels-cistern.png` was opened and inspected: massive coursed stone, brick arches, corroded iron, cool dark recesses, restrained ochre torchlight, dark green wet margins, hanging growth, and shallow water. No inaccessible regional reference was encountered. Reference stays read-only; no concept pixels are used as textures. Campaign lore takes precedence over apparent rain in the concept.

## Visual specification
Municipal drainage masonry: coarse grey-brown blocks, smaller fired brick lining the vault, pale worn limestone coping, broad flagstone maintenance paths. Rounded/chipped edges and recessed mortar; real geometry for arch openings, hollow pipe and grate gaps.

Earthy grey, muted brown and olive. Rust is dark orange-brown with small surviving metal areas. Drier upper masonry remains rough and legible; dampness accumulates at the base, pipe joints and waterline. Mineral salt blooms suggest rising seepage. Moss is localized below wet joints rather than covering every surface. No luminous mushrooms, funerary symbols or invented civic insignia.

Preserve central silhouette space for existing 2D enemy billboards; no new enemy models. This is a standalone illustrative corridor, not a replacement dungeon layout.

## Sample selection
Eight 2K tileable materials: coursed conduit brick, reservoir ashlar, maintenance flagstone, narrow vault brick, limestone coping, corroded iron, moss/slime mat, shallow murky water. Four 2K straight-alpha decals: rising damp, mineral bloom, pipe-joint leak, mortar spall. Three decorative geometry props: hollow pipe outlet, open drain grate, hanging moss.

All regular surfaces span 2 m per UV unit: 1024 texels/m at 2048 pixels. No 4K or upscaled textures. Normal maps derived from periodic physical height fields; color contains pigment variation only, without rendered lighting or shadows. Water uses an opaque shallow-water surface for this sample; it is not a deep-water simulation.

Camera read from current source: horizontal FOV 76 degrees, eye height 155 cm, grid spacing 400 cm. Preview at 1600x900 without game HUD. Neutral and torchlit lighting are art-review rigs, not a claim of matching current game lighting.

Image generation is available. It was not used: no additional illustration was necessary for these physically constructed surfaces.
