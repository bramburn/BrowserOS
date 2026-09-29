# `entrypoints/app/customization/` — toolbar customization (`/settings/customization`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

A small page with one card. It toggles the BrowserOS toolbar buttons — LLM chat,
LLM Hub, toolbar labels — plus vertical tabs when the browser reports support.
Three files total; the page itself is a 10-line composition of a header and a card.

The card talks to the browser through `getBrowserOSAdapter().getPref(...)` /
`setPref(...)` with keys from `@/lib/browseros/prefs` — **not** `chrome.storage`.

## Contents

```
customization/
├── CustomizationPage.tsx  ← header + ToolbarSettingsCard
├── CustomizationHeader.tsx
└── ToolbarSettingsCard.tsx← the switches; capability-gates vertical tabs
```

## Rules

- **CU1 — Browser prefs go through the BrowserOS adapter, not `chrome.storage`.**
  `getBrowserOSAdapter()` + `BROWSEROS_PREFS.*` is the only sanctioned path; these
  prefs are mirrored into the browser process.
- **CU2 — Gate every switch on a capability.** `Capabilities.supports(Feature.VERTICAL_TABS_SUPPORT)`
  decides whether the vertical-tabs row is shown and read at all. A switch that
  renders before its capability resolves will fire a write the browser ignores.
- **CU3 — Defaults are "on".** The card reads `pref?.value !== false`, so a missing
  pref means enabled. Keep that polarity when adding a toggle; flipping it silently
  turns a feature off for existing users.
- **CU4 — Failures are non-fatal.** The `loadPrefs` effect swallows errors and falls
  back to defaults, because the adapter is absent in plain Chrome. Do not surface a
  toast for a missing adapter.

## Workflows

**Adding a toolbar toggle**
1. Add the pref key to `@/lib/browseros/prefs` (`BROWSEROS_PREFS`).
2. Add local state, a `useEffect` read in `loadPrefs`, and a write handler in
   `ToolbarSettingsCard.tsx`.
3. Add the `Label` + `Switch` pair in the same card.
4. If the underlying toolbar item is browser-side, wire it in the Chromium patches
   under `packages/browseros/chromium_patches/` — the extension only writes the pref.

**Verifying a toggle end to end**
1. Run the browser with dev mode and reload the unpacked extension.
2. `bun scripts/dev/inspect-ui.ts open-sidepanel` then
   `bun scripts/dev/inspect-ui.ts eval app "..."` to read back the pref.
3. Check the toolbar visually — the extension write is not enough on its own.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../../../../lib/browseros/prefs.ts`](../../../lib/browseros/prefs.ts) — `BROWSEROS_PREFS` keys.
- [`../../../../lib/browseros/capabilities.ts`](../../../lib/browseros/capabilities.ts) — `Feature` / `Capabilities`.
- [`../../../../lib/browseros/adapter.ts`](../../../lib/browseros/adapter.ts) — the pref read/write adapter.
