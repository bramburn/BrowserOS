# `third_party/blink/renderer/core/` — Blink core path mirror

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium's Blink core is enormous; BrowserOS patches a
single file four levels down at
`third_party/blink/renderer/core/frame/navigator.cc`.

## Contents

```
core/
└── frame/
    └── navigator.cc
```

## Rules

**BLRC1 — Do not add sibling directories speculatively.** `core/` mirrors
Chromium's layout so a future patch at, say, `core/dom/element.cc` has a home.
Create the chain only when a patch needs it.

**BLRC2 — `frame/` is where `Navigator` lives.** If upstream moves `Navigator`
out of `frame/`, this patch path becomes wrong *and* silently stops applying —
verify with `browseros dev apply --dry-run` after any version bump.

## Workflows

**Adding a Blink core patch**
1. Edit the file in `<chromium_src>/third_party/blink/renderer/core/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml` and record the rebase-cost rationale.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `blink/renderer/` rules.
- [`frame/AGENTS.md`](frame/AGENTS.md) — the leaf folder.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — package overview, rule F2.
