# `content/browser/devtools/` — DevTools core patch location

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium's `content/browser/devtools/` also holds
`agent_host*.cc`, `devtools_agent_host_impl.cc`, and the browser-side
`devtools_manager`; BrowserOS patches only the protocol handlers one level
down.

## Contents

```
devtools/
└── protocol/
    └── target_handler.cc
```

## Rules

**CTD1 — The embedder-facing hook is in `content/public/browser/`, not here.**
The `GetTargetTabId` virtual lives in
`content/public/browser/devtools_manager_delegate.{h,cc}`. Keep the
public-API and implementation changes in their correct layers.

**CTD2 — Chrome's delegate override is under `chrome/`.**
`chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}` implements
the virtual. Do not add Chrome-specific behaviour to `content/`.

## Workflows

**Adding a patch under `content/browser/devtools/`**
1. Edit the real file in `<chromium_src>/content/browser/devtools/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `content/browser/` rules.
- [`protocol/AGENTS.md`](protocol/AGENTS.md) — the leaf folder.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview.
