# `chrome/test/data/` — test data root

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/test/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A pure container directory. There are **no patch files directly in
`chrome/test/data/`** — it holds one chain of subdirectories:

- [`webui/`](webui/AGENTS.md) — container for WebUI test data.
- [`webui/settings/`](webui/settings/AGENTS.md) —
  `import_data_dialog_test.ts`, the only BrowserOS WebUI test in the tree.

## Rules

**TD1 — Test data mirrors the Chromium path exactly.** The directory chain
here is a copy of `chrome/test/data/webui/settings/` in the Chromium
checkout. Renaming a directory here changes where Chromium's test runner
looks for the test.

**TD2 — Do not add a `BUILD.gn` in this directory.** WebUI test data is
picked up by the mirrored Chromium `BUILD.gn` files and the mojom test
lists, not by a BrowserOS-local target.

**TD3 — One feature, one test file.** Today a single feature
(`chrome-importer`) owns the only file here. If you add a WebUI test for
another feature, it goes in its own `chrome/test/data/webui/<area>/`
directory, and the corresponding feature block in
`packages/browseros/build/features.yaml`.

## Workflows

**Adding WebUI test data for a feature**
1. Create `chrome/test/data/webui/<area>/<component>_test.ts`.
2. Write Lit `describe`/`it` blocks against the shipped templates.
3. Add the path to the feature block in `build/features.yaml`.
4. Register it in the upstream `chrome/test/data/webui/**/BUILD.gn`.

## Cross-references

- [`webui/settings/AGENTS.md`](webui/settings/AGENTS.md) — the only
  populated child.
- [`../AGENTS.md`](../AGENTS.md) — `chrome/test/`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `packages/browseros/`.
