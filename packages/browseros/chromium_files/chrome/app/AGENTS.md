# `chrome/app/` — namespace for app identity files

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`chromium_files/`](../../AGENTS.md).

## What's here

A single empty directory. It is the `chrome/app/` path segment of the
Chromium tree, and it exists so the branding payload underneath it can
address `<chromium_src>/chrome/app/...`. No build step reads this folder
directly; `build/modules/resources/chromium_replace.py` only ever touches
files found by `rglob`.

## Contents

```
app/
└── theme/
    └── chromium/
        ├── BRANDING.debug
        └── BRANDING.release
```

## Rules

**CFN1 — No files directly in `app/`.** `BRANDING` belongs in
`app/theme/chromium/`, not in `app/`. A file placed here would be copied to
`<chromium_src>/chrome/app/<filename>`, which does not exist upstream and
fails the destination check in `chromium_replace.py:81-87`.

**CFN2 — `app/` here is not `chromium_patches/chrome/app/`.** The patch
tree holds unified diffs against existing Chromium files (including
`chrome/app/app-Info.plist` and `chrome_command_ids.h`); this tree holds
whole-file copies. Mixing the two conventions in one directory breaks the
`annotate` step, which only understands the diff tree.

## Workflows

**Adding the app's Info.plist branding**
1. It already exists as a diff at
   `../../../../chromium_patches/chrome/app/app-Info.plist` — edit it there.
2. Do not add a second copy under this directory.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — `chromium_files/` overview and rules.
- [`../theme/AGENTS.md`](theme/AGENTS.md) — `chrome/app/theme/`.
- [`../../../../chromium_patches/chrome/app/AGENTS.md`](../../../chromium_patches/chrome/app/AGENTS.md) —
  the diff-based `chrome/app/`.
