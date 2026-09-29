# `third_party/blink/public/` — public Blink headers and protocol

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium keeps the DevTools Protocol schema definitions
at `third_party/blink/public/devtools_protocol/`; the overlay mirrors that path
so patch lookup works. BrowserOS patches nothing else in `blink/public/`.

## Contents

```
public/
└── devtools_protocol/
    ├── BUILD.gn
    ├── browser_protocol.pdl
    └── domains/
        ├── Bookmarks.pdl
        ├── Browser.pdl
        ├── History.pdl
        └── Target.pdl
```

## Rules

**BLP1 — `blink/public/` is a public API surface consumed by embedders and
by out-of-tree consumers.** A change here is a change to Blink's contract, not
just to Chromium's build.

**BLP2 — `devtools_protocol/` is the only patched child.** Adding a sibling
folder means a new patch, mirrored at the same relative path.

## Workflows

**Adding a patch under `blink/public/`**
1. Edit the file in `<chromium_src>/third_party/blink/public/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `third_party/blink/` rules.
- [`devtools_protocol/AGENTS.md`](devtools_protocol/AGENTS.md) — the protocol tree.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
