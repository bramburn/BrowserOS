# `chrome/browser/ui/browser_window/` — window feature aggregation

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

A container directory. There are **no patch files directly in
`chrome/browser/ui/browser_window/`** — Chromium splits this directory into
a public header half and an internal implementation half, and BrowserOS
adds one file to each:

- [`public/`](public/AGENTS.md) — `browser_window_features.h`, the accessor
  declarations consumers include.
- [`internal/`](internal/AGENTS.md) — `browser_window_features.cc`, where
  the per-window feature objects are constructed.

## Rules

**BW1 — Public and internal must change together.** A member added to
`browser_window_features.h` must be constructed (or default-initialised) in
`browser_window_features.cc`, or the browser fails to link.

**BW2 — The split is a Chromium include-boundary contract, not
style.** `public/` headers are includable from `chrome/browser/extensions`
and other lower layers; `internal/` is not. Do not move a declaration from
`internal/` into `public/` to make a call site compile.

**BW3 — Anything added here is per-`BrowserWindow` state.** If your feature
is per-`Browser` or per-`Profile`, it belongs elsewhere.

## Workflows

**Exposing a new per-window BrowserOS feature**
1. Add the forward declaration and raw_ptr member +
   accessor to [`public/AGENTS.md`](public/AGENTS.md).
2. Construct it in
   [`internal/AGENTS.md`](internal/AGENTS.md) from the
   window's `Profile` and `TabStripModel`, behind a `FeatureList` guard if
   it is panel-specific.
3. Consume it via
   `browser->GetFeatures()` / `browser_window->GetFeatures()` from the view
   layer.

**Adding a source file to this directory**
Both halves are covered by `chrome/browser/ui/BUILD.gn`. A new file in
either half must be added to that target's `sources`.

## Cross-references

- [`public/AGENTS.md`](public/AGENTS.md) — the accessor surface.
- [`internal/AGENTS.md`](internal/AGENTS.md) — construction.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
