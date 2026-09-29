# `chrome/browser/ui/tabs/` — vertical tab strip

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

The vertical-tabs feature: the GN dependency, the `kVerticalTabs`
`base::Feature` (enabled by default), and the state controller that keeps
Chromium's `prefs::kVerticalTabsEnabled` in sync with BrowserOS's
`browseros.vertical_tabs_enabled`.

## Contents

```
tabs/
├── BUILD.gn                       ← adds "//chrome/browser/browseros/core:prefs"
│                                    to the tab-strip target's deps
├── features.cc                    ← BASE_FEATURE(kVerticalTabs,
│                                     base::FEATURE_ENABLED_BY_DEFAULT)
└── vertical_tab_strip_state_controller.cc
                                   ← on construction: browseros::SyncVerticalTabsPref(...)
                                     observes browseros::prefs::kVerticalTabsEnabled and
                                     mirrors it into prefs::kVerticalTabsEnabled
```

## Rules

**VT1 — There are two vertical-tabs prefs and one direction of sync.**
BrowserOS owns `browseros.vertical_tabs_enabled`
(`browseros::prefs::kVerticalTabsEnabled`); Chromium owns
`prefs::kVerticalTabsEnabled`. `SyncVerticalTabsPref()` performs the initial
copy, and the `PrefChangeRegistrar` callback performs subsequent writes
from BrowserOS → Chromium. Do not add a Chromium → BrowserOS direction; it
would create a feedback loop between the two observers.

**VT2 — `BUILD.gn` must keep the `//chrome/browser/browseros/core:prefs`
dependency.** The controller includes `browseros_prefs.h`; dropping the dep
breaks only the tabs target, which is easy to miss.

**VT3 — The feature is on by default and is not in `chrome://flags` here.**
`kVerticalTabs` is a `base::Feature` in `features.cc`; exposing it in
`chrome://flags` would require the four-file edit described in
[`../AGENTS.md`](../AGENTS.md) (BR1).

**VT4 — Layout constants for the vertical strip are elsewhere.** The
grab-handle size lives in
`chrome/browser/ui/views/frame/layout/browser_view_tabbed_layout_impl.cc`
(`kVerticalTabsGrabHandleSize`). See
[`../views/frame/layout/AGENTS.md`](../views/frame/layout/AGENTS.md).

## Workflows

**Changing the default tab-strip mode**
1. Change `base::FEATURE_ENABLED_BY_DEFAULT` in `features.cc`, or
2. Change the default in `browseros::prefs::kVerticalTabsEnabled`
   (`chrome/browser/browseros/core/browseros_prefs.h`).

Both paths converge through `SyncVerticalTabsPref()` at controller
construction.

**Adding a pref that affects the tab strip**
1. Declare it in `browseros/core/browseros_prefs.h`.
2. Register it in the BrowserOS pref registry.
3. Mirror it into the Chromium pref in
   `vertical_tab_strip_state_controller.cc` — do not read it directly from
   the view layer.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`../views/frame/layout/AGENTS.md`](../views/frame/layout/AGENTS.md) — the
  vertical-tab grab handle.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) —
  `kVerticalTabsEnabled` and `SyncVerticalTabsPref()`.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
