# `chrome/browser/ui/browser_window/public/` — window feature accessors

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/browser_window/`)
> in [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

`browser_window_features.h` — the public header of Chromium's per-window
feature bundle. BrowserOS adds forward declarations for
`ClashOfGptsCoordinator` and `ThirdPartyLlmPanelCoordinator`, the owning
`raw_ptr`/`unique_ptr` members, and inline accessors that return the raw
pointer.

## Contents

```
public/
└── browser_window_features.h   ← class ClashOfGptsCoordinator;   (fwd decl)
                                  class ThirdPartyLlmPanelCoordinator;
                                  ThirdPartyLlmPanelCoordinator* third_party_llm_panel_coordinator();
                                  ClashOfGptsCoordinator* clash_of_gpts_coordinator();
```

## Rules

**BWP1 — Accessors return raw pointers, not references.** The underlying
`unique_ptr` owns the coordinator. Returning a reference would hide the
nullable state, and these coordinators *are* nullable when their feature
flag is off.

**BWP2 — Callers must null-check.** `third_party_llm_panel_coordinator()`
and `clash_of_gpts_coordinator()` return null when
`kThirdPartyLlmPanel` / `kClashOfGpts` are disabled. Every current call
site checks the same `FeatureList` first.

**BWP3 — Forward-declare, do not include.** This header is on the include
path of `chrome/browser/extensions` and other low layers; pulling in
`ui/views/...` headers here would create a layering cycle. Use forward
declarations and include the real header at the call site.

**BWP4 — Members are `base::raw_ptr`, matching Chromium's
`BrowserWindowFeatures` convention.** Never a raw `T*`.

## Workflows

**Adding an accessor**
1. Forward-declare the class at the top of the namespace block.
2. Add the owning member (`base::raw_ptr<T>` with a `unique_ptr` deleter
   pattern already in place).
3. Add the inline accessor returning the raw pointer.
4. Construct it in
   [`../internal/AGENTS.md`](../internal/AGENTS.md).

**Using a coordinator from a view**
1. `GetFeatures()` on the `BrowserWindowInterface`.
2. Call the accessor.
3. Null-check, or guard on the same `base::Feature` the coordinator was
   constructed under.

## Cross-references

- [`../internal/AGENTS.md`](../internal/AGENTS.md) — where members are
  constructed.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/browser_window/`.
- [`../../views/side_panel/AGENTS.md`](../../views/side_panel/AGENTS.md) —
  a primary consumer.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
