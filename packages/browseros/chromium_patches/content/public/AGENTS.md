# `content/public/browser/` — the embedder-facing DevTools delegate

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory holding the two files that define the public hook
embedders implement to give a DevTools target a browser-tab identity. The
implementation side (`content/browser/devtools/protocol/target_handler.cc`)
lives in the sibling `browser/` branch.

## Contents

```
public/
└── browser/
    ├── devtools_manager_delegate.h   ← + virtual GetTargetTabId(WebContents*, int*, int*)
    └── devtools_manager_delegate.cc  ← default implementation returns false
```

## Rules

**CPB1 — `content/public/` is the stable embedder ABI.** Only add to it when
Chrome genuinely needs a hook that no existing API provides. A preference
change or a BrowserOS-only behaviour does not belong here.

**CPB2 — Defaults live in the `.cc`, not inline.** `content/public/browser` is
shipped as a shared library; out-of-line defaults keep header bloat down and
make the "unimplemented ⇒ false" contract explicit.

**CPB3 — Header and `.cc` must change together.** There is exactly one
declaration site and one definition site; neither is optional.

## Workflows

**Adding a new delegate hook**
1. Declare the `virtual` in `content/public/browser/devtools_manager_delegate.h`
   with a doc comment stating the default behaviour.
2. Add the out-of-line default in the `.cc`.
3. Call it from the relevant handler under
   `content/browser/devtools/protocol/`.
4. Override it in `chrome/browser/devtools/chrome_devtools_manager_delegate.{h,cc}`.
5. Register all four paths in `features.yaml`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `content/` subtree rules.
- [`browser/AGENTS.md`](browser/AGENTS.md) — the leaf folder.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
