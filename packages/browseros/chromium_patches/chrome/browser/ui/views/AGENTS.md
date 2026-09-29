# `chrome/browser/ui/views/` — views layer root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

A container directory for the views (widget) layer. There is exactly one
patch file directly in `chrome/browser/ui/views/`: `chrome_layout_provider.cc`.
Everything else in this tree lives in one of the subdirectories listed
below.

## Contents

```
views/
├── chrome_layout_provider.cc   ← non-refresh infobar padding 2*8 → 2*3
└── (subdirs, each with its own AGENTS.md)
    ├── extensions/             ← extension view side-panel auto-focus
    ├── frame/                  ← native widget headless for hidden Browsers
    │   └── layout/             ← vertical-tab grab handle size
    ├── infobars/               ← infobar shadow height
    ├── new_tab_footer/         ← footer suppression
    ├── relaunch_notification/  ← update annoyance level thresholds
    ├── side_panel/             ← the LLM panels + contextual panel helpers
    │   ├── clash_of_gpts/      ← "Council" panel
    │   ├── extensions/         ← extension side-panel utils + auto-pinning
    │   └── third_party_llm/    ← "Chat" panel
    └── toolbar/                ← pinned action buttons + label rendering
```

## Rules

**VW1 — `chrome_layout_provider.cc` is a distance-metric table.** The single
change is the non-refresh infobar total: `36 + 2 * 3` instead of
`36 + 2 * 8`, halving the vertical padding. If you change one metric, expect
the corresponding view to need a matching change.

**VW2 — The refresh/non-refresh fork is `features::kInfobarRefresh`.** Both
branches must stay in the same ternary; a change to one is a change to the
other's fallback.

**VW3 — Views files here are compiled by `chrome/browser/ui/BUILD.gn`,** not
by a `BUILD.gn` in this directory. A new `.cc` here must be added to that
target's `sources`.

**VW4 — macOS-only views live in `frame/*.mm`;** Windows/Linux variants are
separate `.cc` files in the same directory. A change to
`BrowserNativeWidget` headless behaviour must be made in all three
(`ash`, `aura`, `mac`).

## Workflows

**Adjusting infobar height**
1. Edit `DISTANCE_..._INFOBAR` in `chrome_layout_provider.cc`.
2. Check `../infobars/infobar_container_view.cc` — the shadow is hard-coded
   to 1 px and the container's preferred size is derived from the same
   metric.

**Adding a views file**
1. Place it in the correct subdirectory (see the tree above).
2. Add it to `chrome/browser/ui/BUILD.gn`.
3. If it is a BrowserOS-only file (a `new file mode` patch), also add it to
   the owning feature block in `packages/browseros/build/features.yaml`.

**Making a view headless-aware**
Set `params.headless = browser->is_hidden()` in all three native-widget
implementations under [`frame/`](frame/AGENTS.md).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/`.
- [`side_panel/AGENTS.md`](side_panel/AGENTS.md) — the largest BrowserOS
  views subtree.
- [`toolbar/AGENTS.md`](toolbar/AGENTS.md) — pinned buttons and labels.
- [`frame/AGENTS.md`](frame/AGENTS.md) — headless native widgets.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
