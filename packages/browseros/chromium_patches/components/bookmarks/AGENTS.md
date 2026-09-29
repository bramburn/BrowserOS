# `components/bookmarks/` — bookmark bar default

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A single-branch intermediate directory. `components/bookmarks/browser/`
carries the only BrowserOS change to Chromium's bookmark layer.

## Contents

```
bookmarks/
└── browser/
    └── bookmark_utils.cc   ← bookmark bar shown by default
```

## Rules

**BM1 — One file, one hunk.** Anything else in `components/bookmarks/` means
you are adding a new patch; mirror the Chromium path and register it in
`features.yaml`.

**BM2 — `kShowBookmarkBar` default is a product decision.** See
[`browser/AGENTS.md`](browser/AGENTS.md) for the details.

## Workflows

**Adding a patch to Chromium's bookmark layer**
1. Edit `<chromium_src>/components/bookmarks/<file>`.
2. Extract to `chromium_patches/components/bookmarks/<file>`.
3. Keep it under the `chromium-ui-fixes` feature in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`browser/AGENTS.md`](browser/AGENTS.md) — the leaf folder.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
