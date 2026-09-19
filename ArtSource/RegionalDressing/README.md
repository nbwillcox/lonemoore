# Regional dressing library

24 independent props across the nine existing regions. All original regional art is unchanged. These images use Blender studio lighting; they are not in-game captures.

## Integration contract

- All assets stand on the floor. `wall` means against a wall, not mounted in the air.
- Blender front is **-Y**; native Unreal front is **+Y**. Imported FBX reflects Y; the importer writes measured native bounds to the runtime manifest.
- Place the transformed minimum Z at floor + 2cm. Use full bounds when fitting against a wall or keeping movement lanes clear.
- Unit scale is the authored landmark size. `cornerScale` fits the largest footprint dimension to 100cm; maxScale is 1.
- Fixed slots: Wall / Trim / Iron / Bone / Accent. Mesh defaults use the correct region material instances.
- Opaque Nanite-compatible meshes, shared textures, no collision, no per-prop lights or ticking actors. Accent motion is shared shader time only.
- Use full-scale drake skulls, ribcages and lava basins sparingly in recesses; reduced props fit existing wall/corner anchors.

Source validation: **24 props, 80,576 total triangles, 12,572 maximum per mesh**, all source/render files present. Unreal import and runtime presentation still require the separate native checks.

## Asset catalog

| Asset | Region | Bounds X × Y × Z (cm) | Triangles | Placement |
|---|---|---:|---:|---|
| cathedral_votive | Cathedral | 126 × 113 × 240 | 3308 | wall |
| cathedral_bell | Cathedral | 179 × 153 × 141 | 2904 | corner |
| cathedral_lectern | Cathedral | 92 × 77 × 145 | 300 | wall |
| sewer_ooze_pipe | Sewer | 190 × 238 × 164 | 2140 | wall |
| sewer_sump_grate | Sewer | 235 × 178 × 21 | 1508 | floor_patch |
| sewer_sludge_bank | Sewer | 234 × 139 × 54 | 2464 | corner |
| catacombs_bone_pile | Catacombs | 135 × 101 × 74 | 12572 | corner |
| catacombs_ossuary | Catacombs | 168 × 79 × 138 | 12020 | wall |
| warrens_barricade | Warrens | 197 × 82 × 166 | 4688 | wall |
| warrens_cache | Warrens | 150 × 100 × 192 | 2694 | corner |
| warrens_fungus | Warrens | 158 × 124 × 91 | 2280 | corner |
| crypts_sarcophagus | Crypts | 134 × 191 × 123 | 3424 | wall |
| crypts_vigil | Crypts | 82 × 74 × 243 | 988 | wall |
| crypts_standard | Crypts | 134 × 66 × 291 | 532 | wall |
| fortress_armory | Fortress | 179 × 50 × 230 | 428 | wall |
| fortress_shackles | Fortress | 113 × 88 × 76 | 4380 | wall |
| deep_drake_skull | Deep | 179 × 233 × 188 | 2536 | corner |
| deep_ribcage | Deep | 194 × 224 × 173 | 3396 | corner |
| deep_crystals | Deep | 143 × 123 × 159 | 610 | corner |
| infernal_seal | Infernal | 142 × 80 × 159 | 3360 | wall |
| infernal_brazier | Infernal | 124 × 124 × 169 | 4520 | corner |
| hell_lava_basin | Hell | 208 × 209 × 88 | 1716 | corner |
| hell_fissure | Hell | 249 × 111 × 67 | 1704 | floor_patch |
| hell_drake_trophy | Hell | 145 × 172 × 184 | 6104 | wall |

## Files

- `Tools/build_regional_dressing.py`: deterministic Blender builder and preview renderer.
- `Tools/import_regional_dressing.py`: isolated Unreal import, fixed role mapping, Nanite, measured bounds, delayed editor exit.
- `Content/Game/Data/regional_dressing.json`: runtime asset catalog.
- `ArtSource/RegionalDressing/manifest.json`: editable source catalog.
- `ArtSource/RegionalDressing/CONTACT_SHEET.jpg`: preview overview.
- `/Game/RegionalDressing/Meshes/SM_RD_<id>`: new mesh namespace.
- `/Game/RegionalDressing/Materials/MI_RD_<Region>_<Role>`: 45 regional instances.
- `Saved/RegionalDressingPass/import_report.json`: written only when the Unreal importer runs.
