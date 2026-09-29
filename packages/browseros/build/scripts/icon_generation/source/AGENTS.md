# `build/scripts/icon_generation/source/` — master icon input

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/icon_generation/`](../AGENTS.md).

## What's here

The single input to the entire icon pipeline: `app_icon.png`, the
master brand image. `../generate_icons.py` loads it, validates it, and
expands it into every platform icon under
`packages/browseros/resources/icons/`.

This directory is intentionally almost empty. One file is the whole
contract.

## Contents

| File | What it is |
|---|---|
| `app_icon.png` | The master source icon. Must be **at least 1024×1024** and is converted to RGBA on load. |

## Rules

**SRC1 — Minimum 1024×1024, enforced.** `validate_source()` in
`../generate_icons.py` compares `img.width` / `img.height` against
`MIN_SOURCE_SIZE = 1024` and exits 1 with a size-specific message if
either is smaller. This exists because `../generate_icons.txt` derives
outputs up to 1024×1024 (`product_logo_1024.png`) and 256 px entries for
`win/*.ico`; downscaling from a larger master avoids artefacts.

**SRC2 — Non-RGBA sources are converted, not rejected.** If the image
is not already RGBA, the script calls `img.convert("RGBA")` in memory
and never rewrites the file. Saving a converted copy back here is
unnecessary.

**SRC3 — This is the only generated-input directory.** The other inputs
(`product_logo.svg`, `product_logo.ai`,
`product_logo_animation.svg`) live in `../static/` and are copied
verbatim. Do not add generated PNGs here.

**SRC4 — One file only.** If a second brand variant is ever needed, add
a second `PNG`-addressed source and extend `../generate_icons.txt` —
do not silently swap which file `app_icon.png` is, because the file
name is hardcoded as the default source in the README workflow.

**SRC5 — The file is committed to git.** There is no CDN or
download step for the master; the "copy your icon over" instruction in
`../README.md` is a manual one.

## Workflows

**Replacing the brand icon**
1. Overwrite `app_icon.png` with a ≥1024×1024 master.
2. `python build/scripts/icon_generation/generate_icons.py`.
3. `git status` in `packages/browseros/resources/icons/` and commit the
   regenerated outputs alongside this file in the same change.

**Checking the source is valid before generating**
1. `python build/scripts/icon_generation/generate_icons.py` prints
   `✓ Source validated: app_icon.png (WxH)` on success.
2. A failure exits before any output is written, so nothing is left
   half-regenerated.

**Understanding where an icon came from**
1. `../generate_icons.txt` — the `source` column of each operation.
2. Operations reading `source/app_icon.png` are raster-derived;
   operations reading `static/*` are copies.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the icon generation system.
- [`../generate_icons.py`](../generate_icons.py) — the generator and the 1024×1024 gate.
- [`../generate_icons.txt`](../generate_icons.txt) — the operations that read this file.
- [`../static/AGENTS.md`](../static/AGENTS.md) — the manually maintained vector sources.
