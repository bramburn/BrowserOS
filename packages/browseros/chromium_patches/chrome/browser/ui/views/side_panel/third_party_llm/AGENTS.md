# `chrome/browser/ui/views/side_panel/third_party_llm/` — the "Chat" panel

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/side_panel/`)
> in [`packages/browseros/`](../../../../../../../AGENTS.md).

## What's here

The BrowserOS "Chat" side panel: a coordinator that owns the provider list
and per-provider state, and a thin view that exists to guarantee the panel's
`WebContents` is torn down cleanly during browser shutdown. The Clash of
GPTs ("Council") panel reuses this coordinator.

## Contents

```
third_party_llm/
├── third_party_llm_panel_coordinator.{cc,h}  ← provider list, selection,
│                                   BrowserListObserver + ProfileObserver, per-tab
│                                   WebView lifecycle, menus for provider switching
└── third_party_llm_view.{cc,h}               ← a minimal views::View subclass whose
                                    only documented job is proper WebContents cleanup
                                    on browser shutdown
```

## Rules

**TL1 — All four files are new files** (`new file mode` patches against
`/dev/null`).

**TL2 — The coordinator is the shared provider layer.** The Clash of GPTs
coordinator (`../clash_of_gpts/`) includes this header and delegates provider
handling to it. Do not fork provider-fetching or provider-switching logic
into the Council panel.

**TL3 — The view is intentionally almost empty.** `third_party_llm_view.h`
documents its purpose: ensure proper cleanup of `WebContents` during browser
shutdown. Do not add rendering logic to it; the panel's content comes from
the coordinator's `WebView`.

**TL4 — Provider state is pref-backed.**
`browseros.third_party_llm.providers` (list) and
`browseros.third_party_llm.selected_provider` (int, default 0) are
registered in `chrome/browser/ui/side_panel/side_panel_prefs.cc` under
`features::kThirdPartyLlmPanel`. With the flag off, those prefs do not
exist.

**TL5 — Observers use RAII wrappers.** `ScopedObservation` /
`ScopedMultiSourceObservation` on `BrowserListObserver` and
`ProfileObserver`, plus `base::WeakPtr` for callbacks. A raw observer
registration here outlives the coordinator and crashes on profile close.

**TL6 — Construction and registration are feature-gated separately.**
Constructed in
`chrome/browser/ui/browser_window/internal/browser_window_features.cc`,
registered in `../side_panel_helper.cc`. Both check
`features::kThirdPartyLlmPanel`.

**TL7 — Add sources to `../BUILD.gn`** (`source_set("side_panel")`), or the
link fails with unresolved symbols.

## Workflows

**Adding a provider**
1. Add it to the provider model in
   `chrome/browser/browseros/core/browseros_prefs.h`
   (`kProviders`, `kCustomProviders`, `kDefaultProviderId`).
2. No view changes are needed — the coordinator builds the menu from prefs.

**Cycling providers from the keyboard**
`IDC_CYCLE_THIRD_PARTY_LLM_PROVIDER` (40305) is dispatched in
`chrome/browser/ui/browser_command_controller.cc`; bindings are in
`chrome/browser/ui/accelerator_table.cc` and
`chrome/browser/global_keyboard_shortcuts_mac.mm`, both behind
`features::kBrowserOsKeyboardShortcuts`.

**Hiding the Chat button from the toolbar**
Set `browseros::prefs::kShowLLMChat` to false. The
`PinnedToolbarActionsModel` observer in
`chrome/browser/ui/toolbar/pinned_toolbar/` re-evaluates automatically.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/side_panel/`.
- [`../clash_of_gpts/AGENTS.md`](../clash_of_gpts/AGENTS.md) — the consumer
  of this coordinator.
- [`../../../side_panel/AGENTS.md`](../../../side_panel/AGENTS.md) — entry ID
  and the provider prefs.
- [`../../../browser_window/AGENTS.md`](../../../browser_window/AGENTS.md) —
  coordinator construction.
- [`../../../../browseros/core/AGENTS.md`](../../../../browseros/core/AGENTS.md)
  — provider and visibility prefs.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) —
  `packages/browseros/`.
