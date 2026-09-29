# `ui/ozone/platform/` — Ozone platform backend root

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only directory. Chromium's Ozone backends live at
`ui/ozone/platform/<backend>/`; BrowserOS patches only the `x11/` backend. The
overlay reproduces the intermediate `platform/` level so patch lookup by
mirrored path succeeds.

## Contents

```
platform/
└── x11/
    ├── x11_window.h
    └── x11_window.cc
```

## Rules

**OZP1 — Four backends exist upstream (`x11`, `wayland`, `headless`,
`wayland/host`); only `x11` is patched here.** Do not create sibling folders
speculatively.

**OZP2 — `ui/ozone/platform/headless/` is a different thing entirely.** That is
Chromium's own `headless_display` platform backend, unrelated to the
`PlatformWindowInitProperties::headless` flag. Don't conflate them.

**OZP3 — Path fidelity is the only job of this directory.** Mirror Chromium
exactly; the tree above (`ui/ozone/`) has no files of its own.

## Workflows

**Adding a patch under `ui/ozone/platform/`**
1. Edit the file in `<chromium_src>/ui/ozone/platform/<backend>/`.
2. Extract to the mirrored path under `chromium_patches/`.
3. Register it in `features.yaml` and note the platform constraint.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `ui/ozone/` rules.
- [`x11/AGENTS.md`](x11/AGENTS.md) — the patched backend.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview, rule F2.
