# `resources/icons/mac/Assets.xcassets/AppIcon.appiconset/` — app icon set

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`resources/icons/mac/Assets.xcassets/`](../../AGENTS.md).

## What's here

Seven PNGs covering the standard macOS app-icon slots, plus the
`Contents.json` that maps them to idioms, scales and sizes. `actool` compiles
this into `../../../Assets.car`; the icon generator also exports it to
`../../../AppIcon.icns`.

## Contents

| PNG | Slot |
|---|---|
| `appicon_16.png` | 16×16 1x |
| `appicon_32.png` | 16×16 2x **and** 32×32 1x |
| `appicon_64.png` | 32×32 2x |
| `appicon_128.png` | 128×128 1x |
| `appicon_256.png` | 128×128 2x **and** 256×256 1x |
| `appicon_512.png` | 256×256 2x **and** 512×512 1x |
| `appicon_1024.png` | 512×512 2x |

`Contents.json` holds ten `images` entries over those seven files — each
`2x` file is reused as the next size's `1x`, which is the normal Xcode
pattern and the reason the file count is lower than the entry count.

## Rules

**AI1 — Seven files, ten entries: that is correct.** A reviewer expecting
one PNG per `images` entry will find the 2x/1x reuse confusing. Do not
"fix" it by duplicating files.

**AI2 — `appicon_1024.png` is the master and must be 1024×1024.** It is the
only file not reused as a smaller slot. Downstream regeneration depends on
it; a smaller 1024 slot silently scales every other icon.

**AI3 — `Contents.json` is hand-maintained, not generated.** The generator
writes the PNGs; the mapping is the Xcode-authored file. Adding a size
requires both a new PNG *and* a new `images` entry (see XC4 in the parent).

**AI4 — Do not add non-mac idioms casually.** Every entry here is
`"idiom": "mac"`. Adding an `"idiom": "universal"` or `"ios"` entry without
supplying its renditions makes `actool` fail the whole catalog.

**AI5 — Regenerate the derived artefacts together.** Editing a PNG here
requires a macOS run of `generate_icons.py` to refresh `../../../Assets.car`
and `../../../AppIcon.icns`.

## Workflows

**Rebranding the app icon**
1. Replace `appicon_1024.png` with the new ≥1024×1024 artwork.
2. Run `generate_icons.py` on macOS — it re-derives 512/256/128/64/32/16
   from the master, so every PNG in this set changes.
3. Confirm `../../../Assets.car` and `../../../AppIcon.icns` were rewritten.
4. Run `browseros build -m resources` and then the macOS sign/package
   phases.

**Adding a new slot**
1. Generate the PNG with the `appicon_<n>.png` name.
2. Add the `images` entry with the correct `idiom` / `scale` / `size`.
3. Regenerate on macOS.

**Debugging an `actool` error**
1. Every `filename` referenced in `Contents.json` exists (AI3).
2. `appicon_1024.png` really is 1024×1024 (AI2).
3. The root `../Contents.json` is the unmodified Xcode stub.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `Assets.xcassets/` rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `mac/` icon bundle.
- [`../../../../build/scripts/icon_generation/README.md`](../../../../../build/scripts/icon_generation/README.md) —
  generator.
- [`../../../mac/app.icns`](../../app.icns) — the `.icns` exported from this set.
