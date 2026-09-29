# `build/scripts/icon_generation/static/` — hand-maintained vector logos

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/icon_generation/`](../AGENTS.md).

## What's here

The icon files that are **not** generated from the master PNG but
maintained by hand and copied through verbatim. `../generate_icons.py`
implements them as `COPY` operations in `../generate_icons.txt` — a
byte-for-byte copy into `packages/browseros/resources/icons/`, with no
resizing, recolouring or format conversion.

Unlike `../source/app_icon.png`, nothing here is validated. Correctness
is editorial: these are the brand's source-of-truth vector files.

## Contents

| File | What it is |
|---|---|
| `product_logo.svg` | Vector logo. Copied to `resources/icons/` and then to `chrome/app/theme/chromium/`. |
| `product_logo_animation.svg` | Animated vector logo. Copied the same way. |
| `product_logo.ai` | Adobe Illustrator source, kept for designers. Copied, but not consumed by Chromium. |

## Rules

**STA1 — Files here are copied byte-for-byte, never processed.** If an
output looks wrong, the source is wrong; do not add a transformation
to the script to compensate.

**STA2 — `.ai` is a human artefact, not a runtime one.** It is copied
because `../../../../config/copy_resources.yaml` has a
`resources/icons/*.ai` glob that mirrors the `*.png` and `*.svg` globs.
Nothing in Chromium reads it; do not add logic that does.

**STA3 — Keep names in sync with the `COPY` lines in
`../generate_icons.txt`.** Renaming a file here without updating the
manifest silently stops it shipping — there is no scan.

**STA4 — The `icons/` destination is shared with generated PNGs.**
`copy_resources.yaml` globs the whole directory, so anything dropped
here that the manifest names will land in `chrome/app/theme/chromium/`.

**STA5 — Validate SVG changes before shipping.** The pipeline does no
syntax checking on these files; a malformed SVG reaches the browser
untested.

## Workflows

**Updating the vector logo**
1. Replace `product_logo.svg` (or the animation variant).
2. Confirm the filename is still referenced by a `COPY` line in
   `../generate_icons.txt`.
3. `python build/scripts/icon_generation/generate_icons.py`.
4. Run `browseros build -m resources` and inspect the icon in Chromium.

**Adding a new static asset**
1. Drop it in this directory.
2. Add a `COPY <source> <dest>` line to `../generate_icons.txt`.
3. Document it in `../README.md` under "Static Files (Copied)".

**Checking whether an icon is generated or copied**
1. `../generate_icons.txt` — `PNG` / `ICO` / `ICNS` / `XPM` operations
   read `../source/`; `COPY` operations read this directory.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the icon generation system.
- [`../generate_icons.txt`](../generate_icons.txt) — the `COPY` operations naming these files.
- [`../generate_icons.py`](../generate_icons.py) — the copy implementation.
- [`../source/AGENTS.md`](../source/AGENTS.md) — the generated-from master PNG.
- [`../../../../resources/AGENTS.md`](../../../../resources/AGENTS.md) — the destination directory.
