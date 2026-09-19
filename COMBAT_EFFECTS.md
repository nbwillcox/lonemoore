# Combat sound and sprite pass — 2026-09-18

The combat pass adds original console-style sound and animated 2D effects inspired by the presentation of SNES-era JRPGs. No commercial game samples or artwork were used.

- 33 distinct audio cues, including metal slash, critical, miss, dodge, defend, blocked hit, all 21 class skills, health/mana potions, and successful/failed escape.
- 30 pixel-sprite sequences with eight frames each. Regular attacks sweep grey, white, and yellow across the enemy; critical attacks use red crossing slashes. Fireball, Ice Lance, Shadow Bolt, and the other spells use their own appropriate visual feedback.
- Offensive effects follow the selected enemy. Area attacks cover living targets and play one sound per action. Healing, revival, defend, and dodge appear on the affected party member.
- Short resolution pauses keep the full effect visible, including lethal hits. Rejected actions remain silent and produce no sprite.
- Audio uses original pulse/noise synthesis, stepped pitch and quantized timbres, stored as 16-bit PCM for playback. The 8-bit description refers to the authored sound character, not an 8-bit WAV container.

## Try it

Run **Play Lonemoore.cmd** for normal play, or **Review Combat Effects.cmd** for an automatic 31-case showcase with sound. The showcase uses temporary characters and a separate settings directory; it does not save a campaign.

Open **ArtReview/CombatFX/REVIEW.html** for animated previews, individual sound controls, and packaged-game captures. The full 33-cue listening page is **ArtReview/AdventurePolish/Audio/REVIEW.html**.

## Verification and preservation

Completed: 36 native automation tests passed; Windows packaging succeeded; all 31 packaged combat cases passed with audio dispatch and 31 captures at 1600 x 900. Representative single-target, area, defensive, revival, and lethal-hit captures were visually inspected. Preservation checks passed for five save files, two character metadata files, 117 original art files, and 14 unrelated audio assets. The two available project display-preference snapshots were restored and hash-checked.

Final machine-readable evidence is **Saved/CombatFXPass/validation-summary.json**. Source waveform/import reports are in **Saved/AdventurePolish/combat_audio_validation.json** and **combat_audio_import.json**. Native tests are logged in **Saved/Validation/tests.log**; packaged action/audio events are in **Saved/CombatFXPass/packaged-review.log**.

Original audio sources, imported audio assets, modified source files, and available project preference snapshots were backed up under **Saved/CombatFXPass/Backup**. Original `.art` files and campaign saves are checked against **preservation-before.json**. No save-schema changes were made.

Automated loading, routing, waveform checks, and rendered captures establish technical behavior. Subjective audio taste still needs a listening review.
