# `chrome/browser/ui/actions/` — action ID registry

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

One file: `chrome_action_id.h`. It is the X-macro list that generates the
`actions::ActionId` enum for the whole browser. BrowserOS adds four IDs —
two side-panel entries, one agent toggle, and one side-panel entry for
tabs-from-other-devices.

## Contents

```
actions/
└── chrome_action_id.h   ← E(kActionSidePanelShowTabsFromOtherDevices) \
                              E(kActionSidePanelShowThirdPartyLlm) \
                              E(kActionSidePanelShowClashOfGpts) \
                              E(kActionBrowserOSAgent)
```

## Rules

**AC1 — The `E(...)` macro list is the enum; order is ABI.** Insert new
`E(...)` rows at the end of the BrowserOS block. `actions::ActionId` values
are persisted in the pinned-toolbar pref, so reordering existing entries
rewrites users' pinned layouts.

**AC2 — An action ID must be reachable in three places.** The enum entry
alone does nothing. Every BrowserOS action also needs:
`side_panel/side_panel_entry_id.h` (if it opens a panel) and
`webui/side_panel/customize_chrome/customize_toolbar/customize_toolbar_handler.cc`
(if it appears in the customize-toolbar UI).

**AC3 — Action IDs and `IDC_*` command IDs are different namespaces.**
`kActionSidePanelShowThirdPartyLlm` (here) is dispatched by
`kActionBrowserOSAgent`'s command through
`browser_command_controller.cc`; the `IDC_*` constants live in
`chrome/app/chrome_command_ids.h`. Do not conflate them.

**AC4 — Pinned-toolbar membership is decided in
`chrome/browser/browseros/core/browseros_action_utils.h`.** Native
BrowserOS actions live in `kBrowserOSNativeActionIds`; the toolbar uses
`IsBrowserOSAction()` / `GetFeatureForBrowserOSAction()` to decide labels,
pinning, and flex priority.

## Workflows

**Adding a native BrowserOS action**
1. Add `E(kAction...)` to `chrome_action_id.h`.
2. Add it to `kBrowserOSNativeActionIds` in
   `browseros/core/browseros_action_utils.h`.
3. Add an `IDC_*` in `chrome/app/chrome_command_ids.h` and a `case` in
   `browser/ui/browser_command_controller.cc`.
4. Register a UI element in `browser/ui/browser_actions.cc`.
5. If it is a side panel, add a `V(...)` row in
   `browser/ui/side_panel/side_panel_entry_id.h`.

**Renaming an action**
Do not. The string form is persisted in the
`browser.ui.toolbar.pinned_actions` pref. Add a migration or a new ID.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../side_panel/AGENTS.md`](../side_panel/AGENTS.md) — entry IDs that pair
  with these actions.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) —
  `browseros_action_utils.h` and the native action set.
- [`../../../app/AGENTS.md`](../../../app/AGENTS.md) — the `IDC_*`
  counterpart.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
