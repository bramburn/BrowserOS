# `chrome/browser/ui/views/side_panel/clash_of_gpts/` — the "Council" panel

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/side_panel/`)
> in [`packages/browseros/`](../../../../../../../AGENTS.md).

## What's here

A complete, self-contained BrowserOS side panel: a coordinator that owns
pane/provider state, a view that renders the multi-model comparison, and a
separate frameless window for the full-screen comparison experience. All
eight files are `new file mode` patches — brand-new files added to the
Chromium tree.

## Contents

```
clash_of_gpts/
├── clash_of_gpts_coordinator.{cc,h}  ← pane/provider state, BrowserListObserver +
│                                       ProfileObserver, per-pane URL memory
├── clash_of_gpts_view.{cc,h}         ← the side-panel view (pane splitter,
│                                       per-pane WebView, timers)
├── clash_of_gpts_window.{cc,h}       ← a separate views::Widget window hosting
│                                       a ClashOfGptsView
└── (WebUI side: chrome/browser/ui/webui/clash_of_gpts/)
```

## Rules

**CG1 — All eight files are new files.** They are `new file mode` diffs
against `/dev/null`. Do not attempt to express them as edits to Chromium
files.

**CG2 — Ownership is by `FeatureList`.** The coordinator is constructed only
when `features::kClashOfGpts` is enabled
(`chrome/browser/ui/browser_window/internal/browser_window_features.cc`) and
its entry is registered only under the same guard in
`../side_panel_helper.cc`.

**CG3 — State lives in prefs, not memory.**
`browseros.clash_of_gpts.pane_providers`, `.last_urls`, and
`.pane_count` (default 3) are registered in
`chrome/browser/ui/side_panel/side_panel_prefs.cc`. The coordinator reads
and writes them; a restart must restore the same pane layout.

**CG4 — The coordinator observes `BrowserList` and `Profile`.** It uses
`base::ScopedObservation` / `ScopedMultiSourceObservation` and
`base::WeakPtr`. New observers must use the same RAII wrappers — a raw
observer registration outliving the coordinator is the classic crash here.

**CG5 — The coordinator reuses the Chat panel for single-pane content.**
`clash_of_gpts_coordinator.cc` includes
`../third_party_llm/third_party_llm_panel_coordinator.h` and the view
includes the third-party LLM view. Do not duplicate provider-fetching logic;
the Council panel delegates to the Chat coordinator.

**CG6 — The `chrome://clash-of-gpts/` WebUI is a separate concern.**
`clash_of_gpts_ui.{cc,h}` lives under
`chrome/browser/ui/webui/clash_of_gpts/`, is registered in
`chrome_web_ui_configs.cc`, and is declared by
`kChromeUIClashOfGptsHost` / `kChromeUIClashOfGptsURL` in
`chrome/common/webui_url_constants.h`. All three must be touched together.

**CG7 — Add every file to `../BUILD.gn`.** The target is
`source_set("side_panel")`; files on disk but not in `sources` fail at
link time.

## Workflows

**Changing the default number of panes**
1. Change `kDefaultClashOfGptsPaneCount` in
   `chrome/browser/ui/side_panel/side_panel_prefs.cc` (currently 3).
2. The coordinator reads `.pane_count` from prefs; existing profiles keep
   their stored value.

**Adding a new source to the comparison**
1. Add the provider to the pane-provider model in
   `chrome/browser/browseros/core/browseros_prefs.h`
   (`kProviders` / `kCustomProviders` / `kDefaultProviderId`).
2. The view and coordinator pick it up from the pref-backed provider list;
   no per-provider code is needed.

**Debugging the panel not opening**
1. Is `features::kClashOfGpts` enabled?
2. Is `kActionSidePanelShowClashOfGpts` in the default pinned list
   (`chrome/browser/ui/toolbar/toolbar_pref_names.cc`)?
3. Is the entry registered in `../side_panel_helper.cc`?
4. Is the `IDC_OPEN_CLASH_OF_GPTS` case present in
   `chrome/browser/ui/browser_command_controller.cc`?

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/side_panel/`.
- [`../third_party_llm/AGENTS.md`](../third_party_llm/AGENTS.md) — the Chat
  panel this one reuses.
- [`../../../side_panel/AGENTS.md`](../../../side_panel/AGENTS.md) — entry
  ID and prefs.
- [`../../../webui/clash_of_gpts/AGENTS.md`](../../../webui/clash_of_gpts/AGENTS.md)
  — the `chrome://clash-of-gpts/` WebUI.
- [`../../../../browseros/core/AGENTS.md`](../../../../browseros/core/AGENTS.md)
  — provider prefs.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) —
  `packages/browseros/`.
