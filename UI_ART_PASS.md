# Supplied UI artwork integration

Source: `.art/.ui-ux/main.png` (1448 × 1086). The original file is retained unchanged. The attached concept guides the composition; game text, portraits, inventory, gold, health and actions remain live.

Implemented in this first pass:

- Atlas-backed normal, hover and disabled buttons throughout the interface, with measured text fitting.
- Resizable metal frames that preserve their corner ornaments, plus framed parchment notices.
- A panoramic town illustration with matching hotspot coordinates, crimson side banners, and seven illustrated service buttons.
- Portrait frames and illustrated health/mana bars for one through five heroes.
- A bottom action-history panel, inventory/character/hunt controls, and a larger notice area when travelling alone.
- Framed service panels, confirmations and expanded history.

The atlas is imported by `Tools/import_ui_art.py` and loaded through the existing asset manifest. Runtime regions use source pixel coordinates; no destructive cropping or regeneration is required.

This is the first integration pass, not a pixel-for-pixel reproduction of the concept. Further refinement can address serif typography, the handbook's page artwork, and the separately supplied combat UI sheet. Dungeon 3D work remains Phase 3.

The approved artwork is now installed in the main game at `Builds/Windows`. Use `Play First Person Dungeon Crawler.cmd`; it uses your existing main-game saves. The separate preview launcher remains optional.

Validation: content checks passed (2,126 / 2,126). Editor/game compilation and the isolated Windows package succeeded. The packaged review runs with `-UIArtReview -RenderOffscreen` and never loads or writes player saves.

Final validation: all eight packaged review screens captured; merchant, character, confirmation cancellation and history pointer checks passed with no runtime errors. Evidence: `Saved/Validation/UIArt/` and `Saved/Validation/ui_art_packaged_review.log`. Final editor and game builds succeeded.

The earlier preview retains its separate save snapshot. Main-game promotion excluded the Saved folder, preserving the latest Auto, Manual and BeforeAstra saves. A pre-install backup is in `Saved/UIArtPromotionSafety/2026-09-12`.

Main-build verification: eight UI review screens completed with no runtime errors. Merchant, character, cancellation and history pointer checks passed. All three current save hashes match their pre-install backups; display preferences were restored after the offscreen check. Log: Saved/Validation/ui_art_main_build_review.log.

Main menu: supplied .art/.ui-ux/main-menu-parchment.png replaces the grey menu panel, with dark lettering and existing metal buttons. Installed in the main build. Packaged menu rendering and New Game pointer transition verified; save hashes unchanged and display preferences restored. Evidence: Saved/Validation/menu_parchment_review.log and Saved/Validation/UIArt/menu_parchment.png.

Transparent parchment revision reimported and repackaged into the main build. Transparent edges visually verified in the packaged menu; review completed without errors. Saves unchanged and display preferences restored. Log: Saved/Validation/menu_transparency_review.log.

UI proportions refinement: installed in the main build. Frame edges, button textures and vital bars now tile at a consistent scale; banners and menu parchment preserve their aspect ratios. The town title is smaller and higher. Action history displays two recent lines with padding and clipping inside its border; the full history remains accessible. Source artwork was unchanged. Editor review passed at 1600x900; all ten packaged UI review screens passed at 3440x1440, with no runtime errors. Packaged menu review and New Game pointer transition also passed. All three save hashes remained unchanged and display preferences were restored. Evidence: Saved/Validation/UIProportions3440, ui_proportions_3440.log and ui_proportions_menu.log.

Header alignment and Astra update: town title, subtitle and gold share a vertically centered header band. Side banners sit at the top of their frames. Dungeon floor/direction header and on-screen gold are removed; inventory retains the gold counter. FinalChoice now resolves Astra's tile when approached from the adjacent cell. Astra has triple her previous health, 1.8x attack, and rotating party-wide Dark/Fire/Ice/Holy Cataclysms, with stronger attacks below half health. Guard and relevant mitigation still apply. Dungeon.Phase2.Astra and EconomyPresentation passed; main Windows build succeeded. Difficulty is intentionally severe; long-term balance remains a playtesting task.
Widescreen packaged review completed successfully at 3440x1440. Town header and banner placement and removal of the dungeon header visually verified. Evidence: Saved/Validation/AstraAlignment and alignment_review.log. Display settings restored after review.

Follow-up UI and gameplay refinements are installed in the main build; see PHASE2_FOLLOWUP.md. Widescreen visual review and packaged input checks passed. Latest saves and display preferences preserved.
