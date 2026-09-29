# `content/browser/` — DevTools protocol implementation

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium's CDP handler implementations live at
`content/browser/devtools/protocol/`, and the overlay reproduces that nesting so
the patch path matches the source path exactly (parent rule F2).

## Contents

```
browser/
└── devtools/
    └── protocol/
        └── target_handler.cc   ← the only patch
```

## Rules

**CTB1 — `content/browser/devtools/protocol/` is a very large upstream
directory** (dozens of handlers). Only `target_handler.cc` is patched here;
adding another means a new `features.yaml` entry.

**CTB2 — Handlers are generated-adjacent code.** Their shape is dictated by
the `.pdl` definitions. Change the protocol first, then the handler.

## Workflows

**Adding a CDP handler patch**
1. Define the domain/command in
   `third_party/blink/public/devtools_protocol/domains/*.pdl`.
2. Edit the matching `content/browser/devtools/protocol/<domain>_handler.cc`.
3. Extract both to their mirrored paths.
4. Register under the `cdp-api` feature.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `content/` subtree rules.
- [`devtools/AGENTS.md`](devtools/AGENTS.md) — next level down.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
