# `resources/` — branding assets, version file, macOS entitlements

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/`](../AGENTS.md).

## What's here

Everything the build *copies* into the Chromium tree without diffing it.
Three consumers, all declared in code rather than by convention:

1. **Copy operations** — `build/config/copy_resources.yaml` lists every
   directory/glob that lands in `<chromium_src>/`. Read by
   `build/modules/resources/resources.py` (the `resources` module, phase 2).
2. **Version** — `BROWSEROS_VERSION` (`MAJOR`/`MINOR`/`BUILD`/`PATCH`) is
   parsed by `build/common/context.py` into `ctx.semantic_version` and
   copied to `<chromium_src>/chrome/BROWSEROS_VERSION`.
3. **Entitlements** — `entitlements/*.plist` is read directly by
   `build/modules/sign/macos.py` via `ctx.get_entitlements_dir()`; it is
   *not* covered by `copy_resources.yaml`.

A fourth consumer exists but is populated at build time, not from git: the
per-platform BrowserOS Server bundles that `download_resources` fetches from R2
into a temp root, laid out as `<binaries_dir>/<target>/resources/`. Nothing in
this tree is checked in for them.

## Contents

```
resources/
├── BROWSEROS_VERSION          ← BROWSEROS_MAJOR/MINOR/BUILD/PATCH (0.46.3.0)
├── logo.png                   ← 28 KB brand logo; NOT in copy_resources.yaml
├── entitlements/              ← macOS codesign entitlements + Sparkle plist fragment
└── icons/                     ← every product/platform icon, generated
```

Current `BROWSEROS_VERSION`: `0.46.3.0` → `semantic_version` `0.46.3`.

## Rules

**RS1 — A file here is dead until `copy_resources.yaml` names it.** The
`resources` module has no directory scan; it iterates `copy_operations`
only. Adding `resources/foo/bar.png` without a `source:` entry copies
nothing (and the module logs a warning, not an error, for a missing source
directory).

**RS2 — Use `os:` / `arch:` filters rather than creating platform
conditionals.** `copy_resources.yaml` supports `os: [windows|macos|linux]`
and `arch: [x64|arm64]`, checked by `resources.py:64-86`. A `build_type:`
filter also exists but nothing in the current file uses it.

**RS3 — Do not rename assets already referenced by a Chromium patch.**
`updater/branding.gni` (in `chromium_files/`) hard-codes
`//chrome/app/theme/chromium/mac/app.icns`; renaming
`icons/mac/app.icns` breaks the updater bundle at build time.

**RS4 — `BROWSEROS_VERSION` is four `KEY=value` lines, and the build
version is not the app version.** `context.py:359` composes
`semantic_version` from the four fields. Bump all four together; the file is
also the value copied to `chrome/BROWSEROS_VERSION`, which the
`branding` feature block in `build/features.yaml` covers.

**RS5 — `logo.png` is unreferenced today.** It is not matched by any
`copy_resources.yaml` glob (only `resources/icons/*` is). Treat it as a
source-of-truth master for external use, not as a shipped asset, until you
add an entry for it.

**RS6 — Regenerate icons with the generator, do not hand-edit binaries.**
`build/scripts/icon_generation/generate_icons.py` drives Pillow /
ImageMagick / `iconutil` and writes into this tree. Hand-editing a `.icns`
or `Assets.car` produces drift that the next regeneration silently
overwrites.

**RS7 — `chromium.ai` and `product_logo.ai` are 0-byte placeholders.** They
exist so the `resources/icons/*.ai` copy operation in `copy_resources.yaml`
matches something. Do not treat them as real Illustrator sources.

## Workflows

**Adding a new branding asset**
1. Put the file under `icons/` (flat) or the platform subdirectory that
   matches the destination.
2. Add a `copy_operations` entry in
   [`../build/config/copy_resources.yaml`](../build/config/copy_resources.yaml)
   with `source`, `destination`, `type`, and `os:` / `arch:` filters.
3. Run `browseros build -m resources` and confirm the "✓ Copied" line names
   your entry.

**Bumping the BrowserOS version**
1. Edit `BROWSEROS_VERSION` (all four keys).
2. Re-run phase 2 so `chrome/BROWSEROS_VERSION` is refreshed in the
   Chromium tree.
3. Check that `ctx.semantic_version` in the build log matches.

**Adding a macOS entitlement**
1. Write the `.plist` into `entitlements/`.
2. Reference it by filename from
   `build/common/server_binaries.py` (`SignSpec.entitlements`) for server
   binaries, or from `build/modules/sign/macos.py` for helpers.
3. Do not add a `copy_resources.yaml` entry — entitlements are not copied.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `packages/browseros/` (branding-asset
  workflow, F3).
- [`../build/config/copy_resources.yaml`](../build/config/copy_resources.yaml) —
  the declarative copy catalogue.
- [`../build/modules/resources/resources.py`](../build/modules/resources/resources.py) —
  the `resources` module that executes it.
- [`../build/common/context.py`](../build/common/context.py) —
  `get_entitlements_dir()`, `BROWSEROS_VERSION` parsing.
- [`entitlements/AGENTS.md`](entitlements/AGENTS.md) — signing plists.
- [`icons/AGENTS.md`](icons/AGENTS.md) — the icon tree.
- [`../build/scripts/icon_generation/README.md`](../build/scripts/icon_generation/README.md) —
  the icon generator.
