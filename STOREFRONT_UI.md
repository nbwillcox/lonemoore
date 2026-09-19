# Storefront UI

The approved tavern layout is integrated into Tavern, Healer, Merchant, Blacksmith, Gambler, Back Alley, Hunt Board, and Dungeon Entrance.

- The original location images retain their complete framing. The central character region stays clear, including during transaction confirmation and action history.
- Left-side rows show names, short descriptions, icons where available, and prices. Hover previews details; click pins the selection. Purchases remain separate actions in the right panel.
- The right inspector shows effects, quality, slot/power, stack count/limit, selected hero's class compatibility, and sale ownership/equipped warnings. Blacksmith upgrades show before/after item power, hero attack, and armor. Longer descriptions scroll with the mouse wheel over the inspector.
- Buy/sell stock and equipped upgrades use five rows per page. Party portraits along the bottom select the hero to inspect or upgrade.
- Existing transaction rules, save schema, artwork, and balance are preserved. Unaffordable or unavailable actions are disabled; equipped sales retain confirmation.

## Validation

`Tools\Review-Storefront.ps1` runs the packaged native review in an isolated user directory. It checks hover, meal/healing, buy/cancel/confirm, paging, sales, upgrades, gambling, history, unaffordable purchases, maximum quality, and equipped sale cancellation/removal. Use `-Width 3440 -Height 1440` for ultrawide captures.

Build, automated regression, native captures, and save-preservation evidence are stored under `Saved\Storefront` and `Saved\Validation`. Final results are recorded in `Saved\Storefront\VALIDATION.md`.
