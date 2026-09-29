# `resources/icons/mac/` — macOS icon bundle

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

The macOS icon payload: three `.icns` files, the compiled `Assets.car`, and
the `Assets.xcassets` source bundle. Copied whole to
`chrome/app/theme/chromium/mac` by the `Mac Icons` entry in
`build/config/copy_resources.yaml`.

Everything here except the `Contents.json` files is produced by Apple
tooling (`iconutil`, `actool`) and therefore only regenerates on a macOS
host.

## Contents

| File | Notes |
|---|---|
| `app.icns` | 406 KB — the main application icon |
| `AppIcon.icns` | 38 KB — the Xcode asset-catalog export |
| `document.icns` | 406 KB — document-type icon, same size as `app.icns` |
| `Assets.car` | 362 KB — compiled asset catalog (opaque binary) |
| `Assets.xcassets/` | source bundle — see its own AGENTS.md |

`app.icns` is referenced by path from
`chromium_files/chrome/updater/branding.gni` as
`updater_app_icon_path = "//chrome/app/theme/chromium/mac/app.icns"`.

## Rules

**MA1 — This directory is macOS-generated.** `iconutil` and `actool` do not
exist on Windows or Linux. Regenerating the icon tree from a non-macOS host
updates the PNGs and leaves `app.icns`, `AppIcon.icns`, `document.icns` and
`Assets.car` stale — a silent partial update.

**MA2 — Do not hand-edit `.icns` or `Assets.car`.** They are build products
of `generate_icons.py`'s `ICNS` / `XCASSETS` / `ASSETS_CAR` operations.
Regenerate from the source PNGs instead.

**MA3 — `app.icns` is referenced by an absolute GN path.** Renaming it
breaks the macOS updater bundle at build time, with a GN error in
`chrome/updater/` rather than an icon error.

**MA4 — `app.icns` and `document.icns` are the same size (415884 bytes
each).** That is not a copy error; both are full multi-resolution `.icns`
builds from the generator. Do not "deduplicate" one into the other.

**MA5 — `Assets.xcassets/` must stay consistent with `AppIcon.icns` and
`Assets.car`.** All three are derived from the same asset catalog; updating
one without the others is what produces a stale icon in a specific macOS
release path.

**MA6 — The copy has no `os: [macos]` filter.** `copy_resources.yaml` copies
`resources/icons/mac` into every build, including Windows. That is
intentional (it is how the updater asset path resolves on all platforms);
do not add a filter without re-checking `updater/branding.gni`.

## Workflows

**Changing the macOS app icon**
1. Update `source/app_icon.png` under
   `build/scripts/icon_generation/source/`.
2. Run `generate_icons.py` on a macOS host.
3. Confirm `app.icns`, `AppIcon.icns`, `Assets.car` and
   `Assets.xcassets/AppIcon.appiconset/` all changed.
4. Run `browseros build -m resources` on macOS.

**Debugging a blank macOS icon**
1. `Assets.car` older than the `AppIcon.appiconset` PNGs → the host was not
   macOS during regeneration (MA1).
2. `app.icns` missing after a rename → check
   `chromium_files/chrome/updater/branding.gni` (MA3).
3. Stale extra files in `chrome/app/theme/chromium/mac/` → the copy is
   `copytree(dirs_exist_ok=True)` and never deletes; remove them manually.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `icons/` rules (IC6, IC7).
- [`Assets.xcassets/AGENTS.md`](Assets.xcassets/AGENTS.md) — the asset catalog.
- [`../../../build/config/copy_resources.yaml`](../../../build/config/copy_resources.yaml) —
  the `Mac Icons` operation.
- [`../../../build/scripts/icon_generation/README.md`](../../../build/scripts/icon_generation/README.md) —
  `ICNS` / `XCASSETS` / `ASSETS_CAR` operations.
- [`../../../chromium_files/chrome/updater/AGENTS.md`](../../../chromium_files/chrome/updater/AGENTS.md) —
  consumes `mac/app.icns` by path.
