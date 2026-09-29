# `resources/icons/default_100_percent/linux/` — Linux 1x overrides

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

Two Linux-specific 100 % DPI icons: `product_logo_16.png` (728 bytes) and
`product_logo_32.png` (1702 bytes). They sit at
`chrome/app/theme/default_100_percent/chromium/linux/product_logo_*.png`
after the copy, because the parent directory is copied recursively.

## Contents

| File | Size |
|---|---|
| `product_logo_16.png` | 16px |
| `product_logo_32.png` | 1702-byte, 32px |

Both are byte-identical in size to their non-Linux counterparts in
`../../product_logo_16.png` and `../../product_logo_32.png`, i.e. they are
present so Linux finds the DPI-scaled tree with the same filenames rather
than falling back to the flat set.

## Rules

**L1 — The copy is recursive; there is no separate Linux entry.**
`copy_resources.yaml` has one `DPI 100% Icons` operation of
`type: directory` for `resources/icons/default_100_percent`. Anything added
here is copied automatically, on every platform.

**L2 — Only the 100 % tree has a `linux/` subdirectory.**
`../../../default_200_percent/` has none. Do not create one by hand — the
generator emits no such rule, and a 2x Linux override would need a new
`generate_icons.txt` operation plus a decision about the Chromium lookup
path.

**L3 — Filenames must match the non-Linux parent exactly.** Chromium
resolves `product_logo_32.png` under the DPI directory; a Linux-only rename
(`product_logo_32_x11.png`) yields a missing icon at runtime.

**L4 — Generated, not hand-edited.** Both files come from
`build/scripts/icon_generation/generate_icons.py`.

## Workflows

**Adding a Linux-only 1x icon**
1. Add a `PNG <source> <size> linux/product_logo_<n>.png` line to
   `build/scripts/icon_generation/generate_icons.txt`.
2. Regenerate and confirm the file appears under this directory.
3. Run `browseros build -m resources` to copy the parent directory.

**Verifying placement**
1. After phase 2, check
   `<chromium_src>/chrome/app/theme/default_100_percent/chromium/linux/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the `default_100_percent` tree and its
  copy destination.
- [`../../AGENTS.md`](../../AGENTS.md) — `icons/` rules.
- [`../../../build/config/copy_resources.yaml`](../../../../build/config/copy_resources.yaml) —
  the single recursive `DPI 100% Icons` operation.
- [`../../linux/AGENTS.md`](../../linux/AGENTS.md) — the non-DPI Linux icon
  set.
