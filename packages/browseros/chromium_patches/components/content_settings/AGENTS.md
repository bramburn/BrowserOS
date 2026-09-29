# `components/content_settings/` — third-party cookie default

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

An intermediate directory with no files of its own. It exists because
Chromium's cookie-controls code lives two levels down at
`components/content_settings/core/browser/`, and the patch path must mirror
the Chromium path exactly (parent rule F2).

## Contents

```
content_settings/
└── core/
    └── browser/
        └── cookie_settings.cc   ← kCookieControlsMode default → kBlockThirdParty
```

## Rules

**CS1 — Do not flatten this path.** The Chromium file is at
`components/content_settings/core/browser/cookie_settings.cc`; the patch must
be at the identical relative path under `chromium_patches/`. Creating a
`components/content_settings/cookie_settings.cc` patch would never be found by
`ctx.get_patch_path_for_file()`.

**CS2 — One hunk only.** See [`core/browser/AGENTS.md`](core/browser/AGENTS.md).

## Workflows

**Adding a content-settings default patch**
1. Edit `<chromium_src>/components/content_settings/core/browser/<file>`.
2. Extract to `chromium_patches/components/content_settings/core/browser/<file>`.
3. Keep it under `chromium-ui-fixes`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`core/AGENTS.md`](core/AGENTS.md) — next level down.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview, rule F2.
