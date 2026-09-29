# `chrome/browser/ui/views/frame/layout/` — vertical-tab layout tweak

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/views/frame/`)
> in [`packages/browseros/`](../../../../../../../AGENTS.md).

## What's here

One patch: `browser_view_tabbed_layout_impl.cc`. It shrinks the
minimum grab-handle area next to the window caption buttons from 40 px to
5 px so the vertical tab strip's drag region matches BrowserOS's compact
layout.

## Contents

```
layout/
└── browser_view_tabbed_layout_impl.cc   ←
      constexpr int kVerticalTabsGrabHandleSize = 40;   →   5;
```

## Rules

**FL1 — One constant, no logic.** The change is a single value at file
scope. Do not add conditional logic around it; the vertical tab strip's
handle is always narrow in BrowserOS.

**FL2 — 5 is a UX-tuned number, not a typo.** It is the minimum
width/height of the area adjacent to the caption buttons that still accepts
a window drag. Raising it re-creates the wide dead zone upstream ships;
lowering it makes the window hard to move.

**FL3 — The feature flag lives elsewhere.** `kVerticalTabs` is declared in
`chrome/browser/ui/tabs/features.cc`; this constant applies whenever the
vertical strip is active, with no separate guard.

**FL4 — Owned by the `vertical-tabs` feature** in
`packages/browseros/build/features.yaml`, together with
`chrome/browser/ui/tabs/BUILD.gn`, `features.cc`, and
`vertical_tab_strip_state_controller.cc`. Keep them in one commit.

## Workflows

**Tuning the vertical-tab drag behaviour**
1. Change `kVerticalTabsGrabHandleSize` here.
2. Verify on all three desktop platforms — the layout class is shared but
   the caption-button geometry differs.
3. Check the tab strip still has a minimum width.

**Enabling/disabling vertical tabs**
Flip `features::kVerticalTabs` in
`chrome/browser/ui/tabs/features.cc` or the BrowserOS pref
`browseros.vertical_tabs_enabled`. This constant does not change.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/views/frame/`.
- [`../../../tabs/AGENTS.md`](../../../tabs/AGENTS.md) — the vertical-tabs
  feature and pref sync.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) —
  `packages/browseros/`.
