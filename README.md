# Lonemoore

An old-school first-person dungeon crawler. Explore beneath Lonemoore, recruit a party, fight turn-based battles, and uncover the fate of Astra.

## Download the game

**[Download Lonemoore 0.2.0 for Windows (ZIP, 1.22 GB)](https://github.com/nbwillcox/lonemoore/releases/download/v0.2.0/Lonemoore-0.2.0-Windows-x64.zip)**

[Release page and checksum](https://github.com/nbwillcox/lonemoore/releases/tag/v0.2.0) · [All releases](https://github.com/nbwillcox/lonemoore/releases)

Use the compiled game ZIP listed under the release's **Assets**. GitHub's **Code → Download ZIP** and **Source code** archives contain the development project, not the playable game.

## Install and launch

1. Download `Lonemoore-0.2.0-Windows-x64.zip` using the link above.
2. Right-click the ZIP in Windows and choose **Extract All**. Extract it into a folder you can write to.
3. Open the extracted folder and double-click **play-lonemoore.exe**. This is Lonemoore's executable.
4. Keep all accompanying folders beside the executable. Do not run the game from inside the ZIP or move only the executable.

You do not need Unreal Engine, an editor, Git, or any build tools to play. The release is for 64-bit Windows and uses DirectX 12. Minimum hardware requirements have not yet been established.

If the game reports missing Visual C++ runtime components, open `Engine → Extras → Redist → en-us` inside the extracted game folder and run **vc_redist.x64.exe**, then try launching again. The first launch can take longer while resources initialize; let it reach the menu before starting another copy.

## Start playing

Choose **New Game**, select a class, enter a character name, and choose **Normal** or **Hardcore**. The seven classes are Warrior, Mage, Ranger, Cleric, Rogue, Paladin, and Warlock. Hardcore deaths are permanent for that journey.

Begin in town, then enter the dungeon. Recruit companions to build a party of up to five heroes. Explore for equipment, supplies, keys, and secrets. Return to town for healing, rest, supplies, and upgrades.

During combat, click an enemy to select a target, then choose an action for the highlighted hero. Wait for enemy turns to finish before acting again. Skills unlock at levels 1, 5, and 10; assign attribute points on the character sheet when you level up.

## Default controls

| Key or input | Action |
| --- | --- |
| W / S | Step forward / backward |
| A / D | Turn left / right by 90 degrees |
| E | Inspect, open, collect, activate a shrine, or use stairs |
| M | Open / close the automap |
| I | Inventory |
| C | Character sheet |
| J | Hunt journal |
| T | Return to town outside combat |
| Esc | Pause menu |
| F5 | Quicksave |
| F9 | Open quickload confirmation |
| Mouse | Select targets, combat actions, and menu options |

On the automap, middle-drag to pan, scroll to zoom, and click explored ground to add or remove a marker. Controls can be rebound in **Settings**. Settings also includes display, graphics, brightness, frame rate, and separate music/effects volume controls.

## Saves and progress

Each named character has a separate journey with an **Autosave**, a **Quicksave**, and multiple manual saves. To create a manual save, open the pause menu and choose **Save → Create new manual save**. Use **Load** to select a character and saved moment, or **Continue** to resume the most recent usable journey.

Save shrines beside descent stairs become available after the required key and guardian gates. Face a shrine and press **E** to autosave and add a town return point. Shrines do not restore health or mana. Face locked gates and press **E** after obtaining the key; step onto stairs before using them.

The Windows release normally stores saves in `%LOCALAPPDATA%\DungeonCrawler\Saved\SaveGames`. Paste that location into File Explorer's address bar to find and back up your saves before upgrading. This release is intended for fresh New Games; older development save formats are not supported by the character browser.

For more detail, read the [player guide](HOW_TO_PLAY.md).

## About this release

Version 0.2.0 is an early release with an 18-floor campaign, regional environments, secrets, traps, bosses, and post-ending Hell Hunts. Pacing, balance, and the final audio mix are still being refined. See [known issues](KNOWN_ISSUES.md) and [release validation](RELEASE_VALIDATION.md).

If you encounter a problem, [report it on GitHub](https://github.com/nbwillcox/lonemoore/issues) with your game version, what happened, and the steps that led to it.

## Artwork and audio

The creator, nbwillcox, permits free redistribution of Lonemoore's original artwork and audio. This permission does not apply to Unreal Engine or its runtime components. No separate open-source license for the game code is specified.
