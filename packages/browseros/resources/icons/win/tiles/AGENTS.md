# `resources/icons/win/tiles/` — Windows Start tiles

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`resources/icons/win/`](../../AGENTS.md).

## What's here

Two PNGs consumed by the Windows Start menu / tile layout: the large tile
and the small tile. They are copied to
`chrome/app/theme/chromium/win/tiles/` as part of the single `Windows Icons`
directory operation in `build/config/copy_resources.yaml`.

## Contents

| File | Notes |
|---|---|
| `Logo.png` | 10,559 bytes — large Start tile |
| `SmallLogo.png` | 4,374 bytes — small Start tile |

## Rules

**TL1 — Filenames are fixed by Chromium's Windows installer resource
compiler.** `Logo.png` and `SmallLogo.png` are referenced by name in the
`.rc` files; renaming either yields a resource that resolves to nothing.

**TL2 — No separate copy operation.** Adding a `tiles` entry to
`copy_resources.yaml` would double-copy the same files; the parent
`resources/icons/win` directory copy already recurses.

**TL3 — No `os: [windows]` filter.** These tiles are copied into every
build, including macOS and Linux, because the copy operation is
unconditional (see WN5 in the parent).

**TL4 — Regenerate, do not hand-edit.** Both files come from
`build/scripts/icon_generation/generate_icons.py`; a manual edit is
overwritten on the next run and desyncs the Windows tiles from the `.ico`
set in the parent directory.

## Workflows

**Updating the Start tiles**
1. Update the source artwork under
   `build/scripts/icon_generation/source/`.
2. Run the generator on any host (these are plain PNGs — no Apple tooling
   needed for this subdirectory).
3. Run `browseros build -m resources` and check the `Windows Icons` line.

**Debugging a missing tile**
1. Confirm the file is at
   `<chromium_src>/chrome/app/theme/chromium/win/tiles/` after phase 2.
2. The directory copy uses `copytree(..., dirs_exist_ok=True)`, so a rename
   leaves the old file behind and the `.rc` still resolves to the stale one
   — remove it from the checkout.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `win/` rules (WN4, WN5).
- [`../../AGENTS.md`](../../AGENTS.md) — `icons/` destination table.
- [`../../../../build/config/copy_resources.yaml`](../../../../build/config/copy_resources.yaml) —
  the `Windows Icons` operation.
