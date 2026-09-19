# Named journeys

- New Game offers all seven existing classes. Choose a class, enter a character name, choose Normal or Hardcore, then begin.
- The name field supports normal typing, arrow keys, Home/End, Backspace/Delete, Ctrl+A, copy/cut/paste, Enter to begin, and Escape to change class.
- Names accept up to 24 letters, numbers, spaces, apostrophes, hyphens or underscores. Windows reserved names, path characters, control characters and leading/trailing spaces are rejected. An existing name cannot be reused with different capitalization.
- Manual Save opens the current character's save list. **Create new manual save** makes another entry without replacing earlier manual saves.
- Load Game lists characters on the left and their saved moments on the right. Entries show class, level, location and an explicitly labeled UTC timestamp. Each list has its own page controls.
- Starting a named journey immediately creates its first autosave. If the introduction is interrupted, loading that save resumes safely in town.
- Each character has separate Autosave and Quicksave slots. Continue finds the most recently saved usable journey. Hardcore death records remain with that character and prevent resurrection through older saves.
- Choosing a class normally encountered as a companion substitutes a different companion at that location. Every starting class can form a party of five distinct classes.

Player storage is under the running build's `Saved/SaveGames/Characters/<character name>/`. Binary `.sav` files contain the journey; readable JSON files describe each save and the character. Missing metadata can be recovered from the actual save files. Existing flat saves are left in place; the new player-facing browser lists named journeys.

Explicit automated review/test campaigns use their unique `SavePrefix` namespace. Unnamed fixture campaigns are allowed only with `Testing` or `AllowReviewNames`; the player-facing Begin Journey command always validates a nonempty name.

The three profile suites passed in the final 35-suite run: `Dungeon.Profiles.NamesAndSevenClasses`, `Dungeon.Profiles.IsolatedSaveBrowserAndHardcore`, and `Dungeon.Profiles.InitialSaveAndCreationRollback`. The final packaged review passed native name entry, two-character isolation, multiple manual saves, exact selected-snapshot loading, and post-boss autosave/reload and town return. Evidence: `AutomationFinal/index.json` and the packaged candidate's `Saved/AdventurePolish/native_results.txt`. Clipboard shortcuts, more than six entries through native pagination, and power-loss recovery were not exercised in that rendered walkthrough.
