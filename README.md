# Lonemoore

An old-school first-person dungeon crawler built with Unreal Engine 5.8.2. Explore beneath Lonemoore, recruit a party, fight turn-based battles, and uncover the fate of Astra.

## Download and play

Get the Windows x64 ZIP from [GitHub Releases](https://github.com/nbwillcox/lonemoore/releases). Extract the entire archive into a writable folder, then run `DungeonCrawler.exe` (the internal executable name for Lonemoore). Keep the accompanying folders together. Unreal Editor is not required to play.

If Windows reports missing runtime components, run `Engine/Extras/Redist/en-us/vc_redist.x64.exe` when included. A Windows PC with a DirectX 12-capable graphics card is recommended; minimum hardware requirements have not been established.

This is an early release, version 0.2.0. Begin a fresh New Game; retired save formats are not supported by the character browser. Back up your saves before replacing or removing an installation. Shipping builds normally store saves beneath `%LOCALAPPDATA%/DungeonCrawler/Saved/SaveGames`; development builds may use their packaged `DungeonCrawler/Saved/SaveGames` folder.

## Features

- Seven starting classes: Warrior, Mage, Ranger, Cleric, Rogue, Paladin, and Warlock.
- Parties of up to five heroes, turn-based combat, equipment, skills, and leveling.
- An 18-floor campaign with regional artwork, secrets, traps, shrines, and bosses.
- Named characters with autosaves, quicksaves, and multiple manual saves.
- Normal and Hardcore modes, town services, and post-ending Hell Hunts.

## Default controls

| Key | Action |
| --- | --- |
| W / S | Step forward / backward |
| A / D | Turn left / right |
| E | Interact |
| M | Automap |
| I / C / J | Inventory / character / journal |
| T | Return to town outside combat |
| Esc | Menu |
| F5 / F9 | Quicksave / confirm quickload |

Use the mouse to select targets and combat actions. Controls can be rebound in Settings. See [HOW_TO_PLAY.md](HOW_TO_PLAY.md) for the full guide and [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for current limitations. Pacing, balance, and the final audio mix still need sustained player testing.

## Build from source

Install Unreal Engine 5.8.2 and Visual Studio with the Unreal C++ toolchain and Windows SDK. Clone this repository and open `DungeonCrawler.uproject`. Assets and original art/audio are included in the repository; Git LFS is not required.

`Tools/Build.ps1` defaults to the engine installation at `D:\Epic Games\UE_5.8`; adjust `$engineRoot` for your machine. From PowerShell in the project directory:

```powershell
# Compile the editor, run Dungeon automation, and package Windows Shipping:
.\Tools\Build.ps1 -Test -Package -Configuration Shipping -ArchiveDirectory "$PWD\Builds\Distribution"

# Optional development package:
.\Tools\Build.ps1 -Test -Package
```

The Shipping output is `Builds/Distribution/Windows`. Build output, caches, local settings, logs, and player saves are excluded from Git. Playable packages are attached to GitHub Releases.

## Artwork and audio

The creator, nbwillcox, permits free redistribution of the original artwork and audio included with Lonemoore. The creator states that these assets were not sourced from external creators. This permission applies to the original game art and audio, and does not grant rights to Unreal Engine or its runtime components. No separate open-source license for the game code is specified.

See [release validation](RELEASE_VALIDATION.md) for the packaged walkthrough and checksum.

