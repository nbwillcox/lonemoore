# Startup artwork

The normal `Play First Person Dungeon Crawler.cmd` launch uses the supplied Lonemoore artwork:

1. `.art/splashscreen.png` appears as Unreal's native Windows startup splash. `Content/Splash/Splash.bmp` is an aspect-preserving 1000-pixel-wide runtime copy, staged outside the package by Unreal's Windows packager.
2. `.art/loadingscreen.png` appears while the game's artwork loads. It is imported as `/Game/Game/ImportedArt/T_loadingscreen`, loaded before the first UI frame, and displayed in full with black letterboxing where necessary. Its existing title and loading text are retained. A thin gold line below the artwork's ornament shows asset-loading progress.
3. The existing main menu appears when startup loading finishes.

Original PNGs are untouched. Gameplay and save formats are unchanged.

To refresh the art, run `Tools/Prepare-Splash.ps1`, then run `Tools/import_startup_art.py` through Unreal's Python commandlet, and rebuild/package. The imported texture is included by the existing `/Game/Game` cook rule.

Validation artifacts are under `Saved/Validation/StartupArt`. The existing `-MenuArtReview` captures `00_loading.png` before continuing through the menu and character-selection screen, then exits without starting a campaign.

## Verification — September 16, 2026

- Editor build and clean game build succeeded. Final packaging succeeded (`repackage.log`).
- Packaged launcher review reached the menu and character selection and reported `MENU_ART_REVIEW_COMPLETE` (`packaged_review.log`). The captured loading screen was visually inspected; see `packaged_loading.png`.
- The packaged native splash matches `Content/Splash/Splash.bmp` by SHA-256. Live native-window inspection was unavailable because computer-use approval was denied; the native splash itself was not visually verified on desktop.

Revalidated with the 16 September dungeon update: the normal packaged launcher completed standard and ultrawide integrity reviews, and the current loading screen was rendered and inspected at `Saved/IntegrityUpdate/Packaged/loading.png`. The native splash file still matches its staged copy. Original `.art` files remain unchanged.
- Both original PNGs and all four existing campaign saves have identical before/after SHA-256 hashes. Packaged display preferences were restored after the review.
