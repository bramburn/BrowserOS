# `chrome/app/theme/` — namespace for theme assets

> Part of [`../../../AGENTS.md`](../../../AGENTS.md) in
> [`chromium_files/`](../../../AGENTS.md).

## What's here

A single empty directory — the `chrome/app/theme/` path segment. Its only
child, `chromium/`, holds the two `BRANDING` payload files. Icons are *not*
here: they are copied separately at build time from
`../../../resources/icons/` by the `resources` module, which lands them in
`<chromium_src>/chrome/app/theme/chromium/` and
`.../theme/default_{100,200}_percent/chromium/` via
`../../../build/config/copy_resources.yaml`.

## Contents

```
theme/
└── chromium/
    ├── BRANDING.debug
    └── BRANDING.release
```

## Rules

**CFN1 — This folder is a namespace, not a payload root.** Files placed
directly here are copied to `<chromium_src>/chrome/app/theme/<file>`, which
the pinned Chromium tree does not contain.

**CFN2 — Do not put icons here.** Icon copying is declarative: add a
`copy_operations` entry in `../../../build/config/copy_resources.yaml`
pointing at `resources/icons/...`, and let the `resources` module handle it.
Dropping a `.png` here makes `chromium_replace.py` fail the destination
check.

**CFN3 — The 100 %/200 % DPI destinations are siblings, not children.**
`copy_resources.yaml` sends `resources/icons/default_100_percent` to
`chrome/app/theme/default_100_percent/chromium` — a *different* directory
from `chrome/app/theme/chromium/`. Icons in this namespace and DPI-scaled
icons in that one are maintained separately.

## Workflows

**Adding a new icon size**
1. Add the file under `../../../resources/icons/` (flat, or in the platform
   subdirectory that matches the target OS).
2. Confirm a matching glob in `../../../build/config/copy_resources.yaml`
   (`resources/icons/*.png`, or the per-platform `type: directory` entry).
3. Run `browseros build -m resources` and check the copy log.

## Cross-references

- [`../../../AGENTS.md`](../../../AGENTS.md) — `chromium_files/` rules.
- [`chromium/AGENTS.md`](chromium/AGENTS.md) — the `BRANDING` payload.
- [`../../../resources/icons/AGENTS.md`](../../../../resources/icons/AGENTS.md) —
  the source of the copied icon assets.
- [`../../../build/config/copy_resources.yaml`](../../../../build/config/copy_resources.yaml) —
  declarative copy operations.
