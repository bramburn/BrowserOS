# `chrome/browser/ui/webui/side_panel/` — customize-chrome side panel

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/ui/webui/`) in
> [`packages/browseros/`](../../../../../../AGENTS.md).

## What's here

A container directory. There are **no patch files directly in
`chrome/browser/ui/webui/side_panel/`** — it holds a single chain:

- [`customize_chrome/`](customize_chrome/AGENTS.md) — container for the
  customize-toolbar surface.
- [`customize_chrome/customize_toolbar/`](customize_chrome/customize_toolbar/AGENTS.md)
  — `customize_toolbar.mojom`, the handler, and its unit test.

The `chrome://side-panel` host is Chromium's "customize your side panel"
UI; BrowserOS only adds its own actions to it.

## Rules

**SWP1 — Adding a BrowserOS action here is a three-file change.** The
action needs a `V(...)` row in
`chrome/browser/ui/side_panel/side_panel_entry_id.h`, an `E(...)` row in
`chrome/browser/ui/actions/chrome_action_id.h`, **and** a mojom enum value
plus a `case` pair in `customize_chrome/customize_toolbar/`.

**SWP2 — The mojom enum is a separate numbering space.**
`kShowThirdPartyLlm` and `kShowClashOfGpts` are appended to
`customize_toolbar.mojom`'s `ActionId` enum; they are not the same values
as the `actions::ActionId` constants. The handler's
`GetActionId`/`ToActionId` pair translates between them.

**SWP3 — Actions are filed under `kYourChrome`.** Both BrowserOS actions are
added with `CategoryId::kYourChrome`; a new action must pick a category
explicitly.

**SWP4 — Do not create files directly in this directory.** Everything lives
under `customize_chrome/`, matching Chromium's layout.

## Workflows

**Adding a BrowserOS action to the customize panel**
1. Add the mojom enum value in
   [`customize_chrome/customize_toolbar/customize_toolbar.mojom`](customize_chrome/customize_toolbar/customize_toolbar.mojom).
2. Add both `case` arms in `customize_toolbar_handler.cc`
   (`ActionId → mojom::ActionId` and back).
3. `add_action(..., CategoryId::kYourChrome)` in the available-actions list.
4. Add the action/entry IDs in the browser and UI layers.
5. Extend `customize_toolbar_handler_unittest.cc` if the mapping is
   non-trivial.

**Debugging a missing action in the customize panel**
1. Is it in the mojom enum?
2. Are both `case` arms present (a one-way mapping silently drops the
   action)?
3. Was it added to the `add_action` list?

## Cross-references

- [`customize_chrome/AGENTS.md`](customize_chrome/AGENTS.md) — container.
- [`customize_chrome/customize_toolbar/AGENTS.md`](customize_chrome/customize_toolbar/AGENTS.md)
  — the actual files.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/ui/webui/`.
- [`../../side_panel/AGENTS.md`](../../side_panel/AGENTS.md) — entry IDs.
- [`../../actions/AGENTS.md`](../../actions/AGENTS.md) — action IDs.
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) —
  `packages/browseros/`.
