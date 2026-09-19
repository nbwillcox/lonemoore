# Fine-tuning validation

- Native tests: 20 passed, zero failures (8 with unrelated connectivity-check timeout warnings).
- Packaged campaign: 18 floors, 92808 physical boundary sweeps, zero failures.
- Save slots and original artwork: preserved; 122 protected files unchanged.
- Display settings: VSync enabled, all other existing preferences retained.
- Both launchers have matching validated binaries and content containers.
- World enemies: 2x dimensions, capped at 520cm in expanded layouts and 440cm in legacy layouts to clear ceilings; combat artwork and controls retained.
- Timed save confirmation is emitted after the actual save API succeeds, never by dry-run or blocked combat saves.
- Initial route-test run had a randomized enemy-roster death. The fixture now pins RunId as well as the layout seed; its isolated rerun passed, followed by the full final suite. Original failure evidence is retained in `Automation/`.
- Evidence: `Saved/FineTunePass/` (build, import, tests, packaged reviews, before/after CSVs, preservation).
- Screenshots cannot establish the absence of physical monitor tearing; VSync is enabled and verified in the packaged runtime.
