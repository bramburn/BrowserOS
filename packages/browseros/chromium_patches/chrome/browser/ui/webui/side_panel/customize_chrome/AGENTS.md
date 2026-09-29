# `chrome/browser/ui/webui/side_panel/customize_chrome/` — customize-chrome root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/side_panel/`)
> in [`packages/browseros/](../../../../../../../AGENTS.md).

## What's here

A container directory. There are **no patch files directly in
`chrome/browser/ui/webui/side_panel/customize_chrome/`** — its only child is
[`customize_toolbar/`](customize_toolbar/AGENTS.md), which holds the mojom
and handler for BrowserOS's entries in Chromium's customize-your-side-panel
UI.

## Rules

**CCC1 — The mojom lives in `customize_toolbar/`, matching Chromium's
layout.** Do not hoist `customize_toolbar.mojom` into this directory; the
`//chrome/browser/ui/webui/side_panel/customize_chrome` GN target expects it
at that path.

**CCC2 — Adding a mojom value means regenerating bindings.** The `.mojom`
enum `ActionId` is compiled by Chromium's mojom generator; a value added
without regenerating produces a stale
`customize_toolbar.mojom.h` and a mismatch between the handler and the
generated enum.

**CCC3 — A new action is a browser-layer change too.** The mojom value
here is meaningless on its own; it must be backed by an
`actions::ActionId` in `chrome/browser/ui/actions/chrome_action_id.h` and a
`SidePanelEntryId` in
`chrome/browser/ui/side_panel/side_panel_entry_id.h`.

## Workflows

**Adding a customize-panel action**
See
[`customize_toolbar/AGENTS.md`](customize_toolbar/AGENTS.md) for the
file-level recipe, and the sibling
[`../AGENTS.md`](../AGENTS.md) for the three-file rule.

**Listing what lives under this directory**
Only `customize_toolbar/`. If you are adding a second surface, mirror
Chromium's directory name exactly.

## Cross-references

- [`customize_toolbar/AGENTS.md`](customize_toolbar/AGENTS.md) — the only
  populated child.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/side_panel/`.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../../../../../../AGENTS.md`](../../../../../../../AGENTS.md) —
  `packages/browseros/`.
