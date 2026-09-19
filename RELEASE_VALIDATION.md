# Release 0.2.0 validation

Release preparation: September 19, 2026.

- Authored content validation: 2,126 checks passed across 18 floors.
- Unreal native automation: 37 tests passed; zero failed or skipped. Nine tests included warnings from timed-out connectivity probes and deliberately missing save-file fixtures.
- Source configuration: removed the unused Android file-server token and disabled that file server.
- Distribution: Windows x64 Shipping configuration, compiled separately from the existing local Development package.
- Unity compilation is disabled for the game module because combining its translation units causes private helper-name collisions.

- Final extracted Shipping package: 31 walkthrough checks passed, zero failures. This covers seven-class selection, keyboard name entry, independent characters, multiple manual saves, cross-character loading, shrine gating, disk reload, and town return.
- All 33 registered combat sound assets were resident with valid duration. The walkthrough ran without audio output; this is asset-loading validation, not a listening review.
- Archive verification: 30 files, no saves, logs, crash dumps, or debug symbols. The Windows Visual C++ runtime installers and runtime notices are included.
- The 20 protected existing save files matched their recorded hashes. Packaged walkthroughs used separate user directories and unique test characters.
- Release ZIP: `Lonemoore-0.2.0-Windows-x64.zip`, 1,219,280,262 bytes.
- SHA-256: `5e9d41be8fae020174f36cd5b80e1da380b41d17e79bd251879ab00ae63ed179`.

The older walkthrough had a hard-coded expectation of 27 sounds. It now derives the expected count from the current registry; the rebuilt Shipping package passed the corrected check. Gameplay sound data was unchanged.

Automated checks and selected rendered screens do not establish sustained human playtesting or final balance.

Launcher naming update: the release now starts through `play-lonemoore.exe`. The ZIP was extracted and all 31 walkthrough checks passed when started through that launcher. Its embedded game runtime and cooked content match the previously validated build. The player guide, package instructions, and future packaging workflow use the same launcher name.
