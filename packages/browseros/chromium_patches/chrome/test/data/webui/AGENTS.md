# `chrome/test/data/webui/` — WebUI test data root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/test/data/`) in
> [`packages/browseros/`](../../../../../AGENTS.md).

## What's here

A container directory. There are **no patch files directly in
`chrome/test/data/webui/`** — its only child is
[`settings/`](settings/AGENTS.md), which holds the Chrome import dialog
test.

## Rules

**TW1 — One subdirectory per WebUI area.** Chromium's runner discovers
`chrome/test/data/webui/<area>/<name>_test.ts`. Creating a test file
directly in `webui/` will not be discovered.

**TW2 — The directory name is the WebUI area name** (`settings`,
`side_panel`, `help`, …) and must match the WebUI host, not the feature
name.

**TW3 — Do not duplicate production logic in tests.** The import dialog test
asserts on the dialog's *inputs* (`cookies`, `extensions`), which are
defined in
`chrome/browser/resources/settings/people_page/import_data_dialog.html` and
`chrome/browser/ui/webui/settings/import_data_handler.cc`. Update the
handler, not the test, when behaviour changes.

## Workflows

**Adding a test for a new WebUI page**
1. Create `chrome/test/data/webui/<host>/<component>_test.ts`.
2. Register the file in the upstream `chrome/test/data/webui/<host>/BUILD.gn`.
3. Add the path under the owning feature in `build/features.yaml`.

## Cross-references

- [`settings/AGENTS.md`](settings/AGENTS.md) — the only populated child.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/test/data/`.
- [`../../../browser/ui/webui/AGENTS.md`](../../../browser/ui/webui/AGENTS.md)
  — the WebUI code under test.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) —
  `packages/browseros/`.
