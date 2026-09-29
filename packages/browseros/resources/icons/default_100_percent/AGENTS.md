# `resources/icons/default_100_percent/` — 100 % DPI icon set

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

The 1x DPI variants of the product logo, plus a `linux/` subdirectory with
the two sizes Linux needs. The whole tree is copied to
`chrome/app/theme/default_100_percent/chromium` by the `DPI 100% Icons`
entry in `build/config/copy_resources.yaml` — note that is a *sibling* of
`chrome/app/theme/chromium/`, not a subdirectory of it.

## Contents

| File | Notes |
|---|---|
| `product_logo_16.png` | 16px, 728 bytes |
| `product_logo_32.png` | 32px, 1702 bytes |
| `product_logo_name_22.png` | titlebar lockup, colour, 1081 bytes |
| `product_logo_name_22_white.png` | titlebar lockup, white silhouette |
| `linux/product_logo_16.png` | Linux-specific 16px |
| `linux/product_logo_32.png` | Linux-specific 32px |

## Rules

**D1 — Destination is a different directory tree from the flat icons.**
`copy_resources.yaml` sends this folder to
`chrome/app/theme/default_100_percent/chromium`, while `resources/icons/*.png`
goes to `chrome/app/theme/chromium`. Editing one does not affect the other;
both must be regenerated together.

**D2 — Sizes are 1x pixel dimensions, not scaled points.**
`product_logo_name_22.png` here is 1081 bytes; the 2x file of the same nominal
size in `../default_200_percent/` is 2432 bytes. Do not double the size in this
directory.

**D3 — `product_logo_name_22_white.png` is a monochrome silhouette, not a
recoloured copy.** It is produced by the generator's `MONO` operation and
has the same byte size as the colour variant by coincidence, not by
construction.

**D4 — `linux/` is covered by the same directory copy.** There is no separate
`copy_operations` entry for it; adding one would copy the files twice to
the same destination.

**D5 — Do not add files without a matching generator rule.** Every file here
is emitted by `build/scripts/icon_generation/generate_icons.txt`; a manual
addition is deleted on the next run.

## Workflows

**Changing the 1x product logo**
1. Update the source artwork under
   `build/scripts/icon_generation/source/`.
2. Run the generator; check all four top-level files changed.
3. If only the Linux variants should change, the generator emits both — do
   not edit `linux/product_logo_16.png` by hand.

**Adding a 1x size Chromium expects but this tree lacks**
1. Add a `PNG <source> <size> product_logo_<n>.png` line to
   `generate_icons.txt`.
2. Regenerate, then confirm the file appears here and in
   `../default_200_percent/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `icons/` rules (IC3, IC4) and the
  destination table.
- [`../default_200_percent/AGENTS.md`](../default_200_percent/AGENTS.md) —
  the 2x counterpart.
- [`../../../build/config/copy_resources.yaml`](../../../build/config/copy_resources.yaml) —
  the `DPI 100% Icons` operation.
- [`../../../build/scripts/icon_generation/README.md`](../../../build/scripts/icon_generation/README.md) —
  generator and `MONO` operation.
