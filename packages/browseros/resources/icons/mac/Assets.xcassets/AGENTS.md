# `resources/icons/mac/Assets.xcassets/` — asset catalog root

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

The root of an Xcode asset catalog — one JSON file and two image sets. The
catalog is compiled by `actool` into `../Assets.car` and exported by
`iconutil` into `../AppIcon.icns`; both of those live one level up and are
regenerated from this tree by
`build/scripts/icon_generation/generate_icons.py`.

## Contents

```
Assets.xcassets/
├── Contents.json                        ← { "info": { "author": "xcode", "version": 1 } }
├── AppIcon.appiconset/                  ← 7 PNGs + Contents.json
└── Icon.iconset/                        ← 2 PNGs, no Contents.json
```

## Rules

**XC1 — The root `Contents.json` is the Xcode-authored stub and must not be
edited.** It is a fixed one-line document. Changing `version` breaks
`actool` compatibility.

**XC2 — `actool` runs only on macOS.** This catalog is compiled to
`../Assets.car` by the generator's `ASSETS_CAR` operation. Regenerating on
Windows or Linux silently leaves the `.car` stale.

**XC3 — `Icon.iconset/` has no `Contents.json`.** It is a plain two-file
set (`icon_256x256.png`, `icon_256x256@2x.png`), not a spec'd appiconset.
Adding a `Contents.json` with the wrong idiom/scale mapping breaks the
`actool` compile for the whole catalog.

**XC4 — Adding an image requires a matching `Contents.json` entry.**
An unreferenced PNG inside a set is ignored by `actool`; a referenced
filename that is missing is a hard compile error.

**XC5 — Treat this as a build product.** Editing the PNGs by hand
desyncs them from `../Assets.car` and `../AppIcon.icns` until the next
macOS regeneration.

## Workflows

**Adding a new app-icon size**
1. Drop the PNG into `AppIcon.appiconset/` with the `appicon_<n>.png`
   naming the generator uses.
2. Add the matching `images` entry (`filename`, `idiom`, `scale`, `size`)
   to `AppIcon.appiconset/Contents.json`.
3. Run `generate_icons.py` on macOS to refresh `../Assets.car` and
   `../AppIcon.icns`.

**Debugging an `actool` failure**
1. Check every `filename` in each `Contents.json` exists on disk.
2. Check `idiom` / `scale` / `size` triples are consistent (see the existing
   entries for the pattern).
3. Confirm the root `Contents.json` is untouched (XC1).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `mac/` icon rules (MA1, MA5).
- [`AppIcon.appiconset/AGENTS.md`](AppIcon.appiconset/AGENTS.md) — the app icon set.
- [`Icon.iconset/AGENTS.md`](Icon.iconset/AGENTS.md) — the plain icon set.
- [`../../../../build/scripts/icon_generation/README.md`](../../../../build/scripts/icon_generation/README.md) —
  `XCASSETS` / `ASSETS_CAR` operations.
