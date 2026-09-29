# `chrome/` — Chromium-relative namespace root

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`chromium_files/`](../../AGENTS.md).

## What's here

A single empty directory. It exists so the paths under it can mirror the
Chromium source tree (`chrome/app/`, `chrome/updater/`,
`chrome/enterprise_companion/`) — `chromium_replace.py` computes
`dst_file = ctx.chromium_src / <path relative to chromium_files/>` and
refuses to create the destination. Nothing in this folder is copied, and no
build step reads it directly.

## Contents

```
chrome/
├── app/                      ← theme/BRANDING.{debug,release}
├── enterprise_companion/     ← branding.gni
└── updater/                  ← branding.gni
```

## Rules

**CFN1 — Never put a file directly in `chrome/`.** Any file here is copied
to `<chromium_src>/chrome/<filename>`, and the pinned Chromium tree has no
file at that path — `chromium_replace.py` raises `FileNotFoundError`.

**CFN2 — Depth must match Chromium exactly.** The path under
`chromium_files/chrome/` is the destination path under `<chromium_src>/chrome/`.
`app/theme/chromium/` lands in `<chromium_src>/chrome/app/theme/chromium/`.

**CFN3 — This tree is not a patch surface.** If you need a diff against an
existing Chromium file, use `../../../chromium_patches/` with the mirrored
path instead. See the parent file's CF1/CF2.

## Workflows

**Checking where a file in this tree will land**
1. Take its path relative to `chromium_files/`.
2. Join it onto `ctx.chromium_src` (`../build/common/context.py`,
   `ctx.chromium_src`).
3. Confirm that path exists in the pinned Chromium tree before committing.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — `chromium_files/` overview and rules.
- [`../app/AGENTS.md`](app/AGENTS.md) — `chrome/app/`.
