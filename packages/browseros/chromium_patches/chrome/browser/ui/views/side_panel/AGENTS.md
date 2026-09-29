# `chrome/browser/ui/views/side_panel/` — side-panel views and BrowserOS panels

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

The views half of Chromium's side panel, plus the three BrowserOS-owned
additions: the "Chat" (third-party LLM) panel, the "Council" (Clash of
GPTs) panel, the contextual extension-panel helpers, and an accessibility
page extractor shared by both BrowserOS panels.

## Contents

```
side_panel/
├── BUILD.gn                     ← adds browseros_simple_page_extractor.*,
│                                   clash_of_gpts/*, third_party_llm/* to
│                                   source_set("side_panel"), and
│                                   "//chrome/browser/browseros/metrics" to public_deps
├── browseros_simple_page_extractor.{cc,h}   ← NEW: turns an
│                                   ui::AXTreeUpdate into structured text for the
│                                   BrowserOS LLM panels
├── side_panel.{cc,h}            ← adds animations_disabled_browseros_ (default true)
│                                   to the ShouldRenderRichAnimation() condition
├── side_panel_coordinator.cc    ← content->RequestFocus() on entry switch
├── side_panel_helper.cc         ← registers the BrowserOS entries on the window
│                                   registry, each behind its FeatureList guard
├── clash_of_gpts/               ← "Council" panel (see its AGENTS.md)
├── extensions/                  ← contextual side-panel helpers + auto-pinning
└── third_party_llm/             ← "Chat" panel (see its AGENTS.md)
```

## Rules

**SPV1 — `animations_disabled_browseros_` defaults to `true` in the header
and is `const`-like in practice.** It is added to the existing
`animations_disabled_` term in `SidePanel::ShouldRenderRichAnimation()`.
BrowserOS panels do not animate; flipping this changes every side panel in
the browser, not just BrowserOS ones.

**SPV2 — `browseros_simple_page_extractor` is a new file, not a patch to
an existing extractor.** It converts `ui::AXTreeUpdate` /
`AXNodeData` into plain text for the LLM panels. It is the accessibility
bridge: any change to the Chromium AX tree shape lands here first.

**SPV3 — `side_panel_helper.cc` is the registration point.** It calls
`CreateAndRegisterEntry(window_registry)` on
`third_party_llm_panel_coordinator()` and
`clash_of_gpts_coordinator()`, each behind
`features::kThirdPartyLlmPanel` / `features::kClashOfGpts`. The
coordinators themselves are constructed in
`../../browser_window/internal/browser_window_features.cc`; a registration
without construction is a null dereference.

**SPV4 — `BUILD.gn` must list every BrowserOS source.** This target is
`source_set("side_panel")`; a file present on disk but absent from
`sources` compiles nowhere and fails at link time in `chrome/browser/ui`.

**SPV5 — The metrics dependency is deliberate.**
`"//chrome/browser/browseros/metrics"` is in `public_deps` because the
panels log usage; removing it breaks the link with an unresolved symbol
rather than a GN error.

## Workflows

**Adding a new BrowserOS side panel**
1. Create `side_panel/<name>/` with a coordinator, view, and window class.
2. Add the sources to this directory's `BUILD.gn`.
3. Add an entry ID in `../../side_panel/side_panel_entry_id.h` and an
   action in `../../actions/chrome_action_id.h`.
4. Declare a `base::Feature` in `../../ui_features.cc` (default on).
5. Construct the coordinator in
   `../../browser_window/internal/browser_window_features.cc` behind that
   feature.
6. Register the entry in `side_panel_helper.cc` behind the same feature.

**Changing what the panels send to an LLM**
Edit `browseros_simple_page_extractor.cc` (or its header). The panels call
it; they do not walk the AX tree themselves.

**Debugging a panel that does not appear**
1. Check the `base::Feature` is enabled.
2. Check the coordinator exists (`GetFeatures()->…_coordinator()` is
   non-null).
3. Check `side_panel_helper.cc` registered an entry with the right
   `SidePanelEntryId`.
4. Check the action is in the default pinned list
   (`../../toolbar/toolbar_pref_names.cc`) or the prefs
   (`browseros::prefs::kShowLLMChat` / `kShowLLMHub`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/`.
- [`../side_panel/AGENTS.md`](../side_panel/AGENTS.md) — entry IDs, prefs,
  action callback.
- [`../../browser_window/AGENTS.md`](../../browser_window/AGENTS.md) —
  coordinator construction.
- [`clash_of_gpts/AGENTS.md`](clash_of_gpts/AGENTS.md) and
  [`third_party_llm/AGENTS.md`](third_party_llm/AGENTS.md) — the panels.
- [`extensions/AGENTS.md`](extensions/AGENTS.md) — contextual panel helpers.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
