# `extensions/common/` — extension permission enum

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium keeps the mojom-based extension API permission
enum at `extensions/common/mojom/api_permission_id.mojom`; the overlay mirrors
that nesting so the patch is found by path.

## Contents

```
common/
└── mojom/
    └── api_permission_id.mojom   ← + kBrowserOS = 266
```

## Rules

**EXC1 — `extensions/common/` is the layer-agnostic half of the extensions
system.** It is built for every embedder including non-Chrome ones. Only
declarations belong here; behaviour lives in `extensions/browser/` or
`chrome/`.

**EXC2 — The mojom path is `mojom/api_permission_id.mojom`; don't flatten it
to `common/api_permission_id.mojom`.**

## Workflows

**Adding a patch under `extensions/common/`**
1. Edit the file in `<chromium_src>/extensions/common/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `extensions/` subtree rules.
- [`mojom/AGENTS.md`](mojom/AGENTS.md) — the leaf folder.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview, rule F2.
