# `resources/icons/linux/` — Linux icon set

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

The non-DPI Linux icon set: five PNGs plus one XPM. It is copied whole to
`chrome/app/theme/chromium/linux` by the `Linux Icons` entry in
`build/config/copy_resources.yaml` — note there is no `os: [linux]` filter on
that operation, so the directory is copied into every build.

## Contents

| File | Notes |
|---|---|
| `product_logo_24.png` | 24px |
| `product_logo_32.xpm` | X11 pixmap, ImageMagick-generated |
| `product_logo_48.png` | 48px |
| `product_logo_64.png` | 64px |
| `product_logo_128.png` | 128px, same size as the flat master |
| `product_logo_256.png` | 256px |

## Rules

**LI1 — `product_logo_32.xpm` needs ImageMagick.** The `XPM` operation in
`generate_icons.txt` shells out to `convert`/`magick`. On a host without it
the generator fails on this one file while the PNGs succeed — a partial
regeneration, not a clean error.

**LI2 — The copy has no `os:` filter.** `copy_resources.yaml` copies
`resources/icons/linux` unconditionally. Do not assume removing the
directory is Windows/macOS-safe; the build will log a missing-source
warning and continue with a stale directory in the Chromium tree.

**LI3 — This is the non-DPI tree.** The DPI-specific Linux icons live at
`../default_100_percent/linux/`, copied to a *different* Chromium directory
(`chrome/app/theme/default_100_percent/chromium/linux`). Both are needed.

**LI4 — `.xpm` is text but still generated.** Editing it by hand desyncs it
from `product_logo_32.png` and the next regeneration overwrites it silently.

**LI5 — Filenames are Chromium's.** `product_logo_<n>.png` /
`product_logo_32.xpm` are resolved by name from
`chrome/app/theme/chromium/linux/`.

## Workflows

**Changing the Linux icon**
1. Update the source artwork under
   `build/scripts/icon_generation/source/`.
2. Run the generator with ImageMagick on PATH.
3. Confirm the `.xpm` and all five PNGs changed in the same run.
4. Run `browseros build -m resources` and check the `Linux Icons` line.

**Debugging a missing Linux icon in the build output**
1. `Source directory not found: resources/icons/linux` → the directory was
   renamed or deleted; `resources.py:106-107` warns and continues, so the
   Chromium tree keeps a stale copy.
2. `xpm` present but PNGs stale → ImageMagick failed and the generator
   aborted partway.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `icons/` rules and destination table.
- [`../default_100_percent/linux/AGENTS.md`](../default_100_percent/linux/AGENTS.md) —
  the DPI-scaled Linux set.
- [`../../../build/config/copy_resources.yaml`](../../../build/config/copy_resources.yaml) —
  the unconditional `Linux Icons` operation.
- [`../../../build/scripts/icon_generation/README.md`](../../../build/scripts/icon_generation/README.md) —
  `XPM` operation and ImageMagick requirement.
