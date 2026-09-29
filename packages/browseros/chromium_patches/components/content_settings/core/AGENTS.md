# `components/content_settings/core/` — core content-settings intermediates

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. It carries no files itself; it exists because Chromium
nests the cookie controls implementation at
`components/content_settings/core/browser/cookie_settings.cc` and the patch
overlay reproduces that nesting.

## Contents

```
core/
└── browser/
    └── cookie_settings.cc   ← the only patch
```

## Rules

**CSC1 — Path fidelity is the whole job of this directory.** Adding, renaming,
or flattening anything here breaks lookup. Mirror Chromium exactly.

**CSC2 — Chromium has other `core/` subdirs** (e.g. `core/common/`) that
BrowserOS does not patch. Only add a child directory when a new patch demands
it.

## Workflows

**Adding a patch under `components/content_settings/core/`**
1. Edit the real file in `<chromium_src>/components/content_settings/core/...`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml` (`chromium-ui-fixes` unless branding).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/content_settings/` rules.
- [`browser/AGENTS.md`](browser/AGENTS.md) — the leaf folder.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview.
