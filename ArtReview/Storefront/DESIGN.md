# Tavern storefront sample

Review sample only. No game code, art, saves, or packaged builds changed.

Keep the central character, hands, and held objects unobscured. At the 1600×900 design size, the left panel ends at x424 and the right panel starts at x1228. The status strip starts at y802, below the visible character. Preserve the full source image and its framing; contain the scene on ultrawide screens rather than cropping it.

Named service rows with prices sit left. Hover or keyboard focus previews full details in a fixed right panel; click pins the selection. A separate explicit purchase button avoids buying on inspection. No popup can cover the merchant. Prices shown use the screenshot's solo level-1 party. Purchases are simulated and reset on reload.

For eventual game integration, use the same side regions for paginated item stock, with complete ItemDetails output, price, quality, ownership/count, requirements and comparisons where applicable in the right inspector. Bind care availability and affordability to the existing game model. Keep all HUD, notices, and focus/hover content out of the character region. Validate each merchant image individually before rollout; widen the protected center if their silhouette requires it.

Review this sample before implementing the layout in DungeonUI.cpp. Game behavior and native resolution validation remain part of that later integration.
