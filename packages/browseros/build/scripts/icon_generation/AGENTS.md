# `build/scripts/icon_generation/` — all platform icons from one PNG

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The icon pipeline. One master image (`source/app_icon.png`, at least
1024×1024) plus a small set of manually maintained vector sources
(`static/`) are expanded into every icon Chromium wants on Windows,
macOS, Linux and ChromeOS, written into
`packages/browseros/resources/icons/`. A declarative config file
(`generate_icons.txt`) lists the operations; `generate_icons.py`
interprets it.

The generated output is picked up by
`../../../../config/copy_resources.yaml` on the next build — that file,
not this script, is what actually installs the icons.

## Contents

| File | What it does |
|---|---|
| `generate_icons.py` | The generator. `validate_source()`, `generate_png()`, and handlers for the `PNG` / `MONO` / `ICO` / `XPM` / `ICNS` / `XCASSETS` / `ASSETS_CAR` / `COPY` operations. `SCRIPT_DIR`, `DEFAULT_CONFIG`, `SOURCE_DIR`, `STATIC_DIR`, `OUTPUT_DIR`, `MIN_SOURCE_SIZE = 1024`. |
| `generate_icons.txt` | The operation list — one line per generated or copied artifact. |
| `README.md` | Full reference: quick start, requirements, config format, and a table of every generated output with its Chromium target. |
| `source/` | The master input. |
| `static/` | Manually maintained vector sources, copied verbatim. |

Requires Python 3.12+, Pillow, ImageMagick (XPM), and macOS for
`.icns` / `Assets.car` (`iconutil`, `actool`).

## Rules

**ICO1 — `source/app_icon.png` must be at least 1024×1024.** A smaller
source exits with "Source image is too small". Validate before editing
the config: the size gate is in `validate_source()`.

**ICO2 — `static/` files are copied, never transformed.** The `COPY`
operation passes them through untouched. `product_logo.ai` is an
Illustrator source for humans; the browser never reads it.

**ICO3 — `generate_icons.txt` is the manifest, not a config the script
infers.** Adding an output means adding a line, then confirming the
matching Chromium theme path is covered by
`../../../../config/copy_resources.yaml` — otherwise the icon is
generated and then never copied.

**ICO4 — Output path is `scripts/../../../resources/icons/`, computed as
`SCRIPT_DIR.parent.parent.parent / "resources" / "icons"`.** It is
hardcoded relative to the script, not to CWD. Do not run the script
from elsewhere and expect a different output directory.

**ICO5 — macOS-only operations fail off macOS.** `iconutil` and
`actool` do not exist on Windows or Linux. Generating the full set
requires macOS; generating only the `PNG` / `ICO` / `COPY` subset does
not.

**ICO6 — Pillow is a hard requirement of the script** and is imported
at module top with an `ImportError` guard that prints an install hint
and exits 1. Do not make it optional — every operation needs it.

**ICO7 — Update the README when you add an output.** It contains the
authoritative table of generated files and their Chromium targets;
`copy_resources.yaml` globs (`resources/icons/*.png`,
`*.ai`, `*.svg`) plus the `chromeos` / `linux` / `mac` / `win`
subdirectories.

## Workflows

**Regenerating all icons after a brand change**
1. Replace `source/app_icon.png` with the new ≥1024×1024 master.
2. Update any `static/*.svg` / `*.ai` sources that changed.
3. `python build/scripts/icon_generation/generate_icons.py` (on macOS
   for the complete set).
4. `git status` in `packages/browseros/resources/icons/` — everything
   under there is generated; commit the diff.
5. Run `browseros build -m resources` to copy them into Chromium.

**Adding a new icon size**
1. Add a `PNG <source> <size> <dest>` line to `generate_icons.txt`.
2. Confirm `copy_resources.yaml`'s `resources/icons/*.png` glob picks it
   up (it will, for anything directly in `icons/`).
3. Document it in `README.md`.

**Debugging a generation failure**
1. "Source image is too small" → enlarge `source/app_icon.png`.
2. "iconutil not found" / "actool not found" → you are not on macOS, or
   Xcode Command Line Tools are missing.
3. "ImageMagick not found" → `XPM` operations cannot run; install
   ImageMagick or drop those lines.

**Blurry Windows taskbar icons**
1. Check the `win/chromium.ico` entry includes 40×40 (125% DPI, the
   most common scaling on modern displays).
2. Regenerate and inspect the ICO.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — helper scripts overview.
- [`README.md`](README.md) — the generated-icon reference table.
- [`generate_icons.txt`](generate_icons.txt) — the operation manifest.
- [`../../../../resources/AGENTS.md`](../../../resources/AGENTS.md) — the output directory and `BROWSEROS_VERSION`.
- [`../../../../config/AGENTS.md`](../../config/AGENTS.md) — `copy_resources.yaml`, which installs the generated icons.
