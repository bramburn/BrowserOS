# `chrome/browser/ui/browser_window/internal/` — window feature construction

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/browser_window/`)
> in [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

`browser_window_features.cc` — the translation unit that constructs
per-window feature objects. BrowserOS adds two: the
`ThirdPartyLlmPanelCoordinator` and the `ClashOfGptsCoordinator`, each
created only when its `base::Feature` is enabled.

## Contents

```
internal/
└── browser_window_features.cc   ← includes both *_coordinator.h; constructs
                                   third_party_llm_panel_coordinator_ when
                                   features::kThirdPartyLlmPanel is enabled, and
                                   clash_of_gpts_coordinator_ when
                                   features::kClashOfGpts is enabled
```

## Rules

**BWI1 — Construct coordinators only under their `FeatureList` guard.**
With the flag off the `unique_ptr` stays null; consumers in
`side_panel_helper.cc` already check the same flag before dereferencing.
Adding an unguarded construction leaks a coordinator into a window that was
built with the feature off.

**BWI2 — The coordinator needs the window's `Profile` and `TabStripModel`
at construction time.** Both are available here; they are not available at
`CreateAndRegisterEntry` time, which is why registration is deferred.

**BWI3 — Construction order matters.** The coordinators register their
side-panel entries later, from `side_panel_helper.cc`. Do not call
`CreateAndRegisterEntry` from this file — the window registry does not
exist yet.

**BWI4 — Include what you use.** Each new coordinator needs its own
`#include ".../coordinator.h"`; the file does not include the
`browser_window_features.h` transitively for you.

## Workflows

**Adding a per-window BrowserOS feature**
1. Add the `unique_ptr` + accessor to
   [`../public/AGENTS.md`](../public/AGENTS.md) (as
   `raw_ptr`, not a raw pointer — see that file's rules).
2. Construct it here from `profile` and `browser->GetTabStripModel()`.
3. Gate it on a `base::Feature` if it is a panel.
4. Register its side-panel entry from
   `chrome/browser/ui/views/side_panel/side_panel_helper.cc`.

**Debugging a null coordinator**
Almost always a feature-flag mismatch: the coordinator is created under
`FeatureList` but the consumer dereferences it unguarded (or the reverse).
Check both sites before assuming a lifecycle bug.

## Cross-references

- [`../public/AGENTS.md`](../public/AGENTS.md) — declarations.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/browser_window/`.
- [`../../views/side_panel/third_party_llm/AGENTS.md`](../../views/side_panel/third_party_llm/AGENTS.md)
  and [`../../views/side_panel/clash_of_gpts/AGENTS.md`](../../views/side_panel/clash_of_gpts/AGENTS.md)
  — the coordinators built here.
- [`../../ui_features.h`](../../ui_features.h) — `kThirdPartyLlmPanel`,
  `kClashOfGpts`.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
