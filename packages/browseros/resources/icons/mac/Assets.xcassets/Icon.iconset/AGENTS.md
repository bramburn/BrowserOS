# `resources/icons/mac/Assets.xcassets/Icon.iconset/` — plain icon set

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`resources/icons/mac/Assets.xcassets/`](../../AGENTS.md).

## What's here

The smallest set in the catalog: two PNGs and nothing else. Unlike
`../AppIcon.appiconset/`, there is no `Contents.json` — these are a plain
`@1x` / `@2x` pair named by Xcode's `@2x` convention, so `actool` infers
the slots from the filenames.

## Contents

| File | Notes |
|---|---|
| `icon_256x256.png` | 20017 bytes — 1x |
| `icon_256x256@2x.png` | 50209 bytes — 2x (512×512) |

Both are byte-identical in size to `appicon_256.png` and `appicon_512.png`
in the sibling appiconset.

## Rules

**IX1 — The `@2x` suffix is the entire specification.** There is no
`Contents.json`; renaming either file removes it from the compiled
`Assets.car` without any error.

**IX2 — Both files are shared with the appiconset.** The same pixel data
appears as `appicon_256.png` / `appicon_512.png`. A rebrand that updates
only this set produces a catalog with two different logos at 256px.

**IX3 — Do not add a `Contents.json` speculatively.** A spec'd set with a
half-filled `images` array fails `actool` for the whole catalog; the
inferred pair is intentional.

**IX4 — Regenerate on macOS.** `actool` compiles the parent catalog; from
Windows or Linux these two PNGs update but `../../../Assets.car` does not.

## Workflows

**Checking this set is current**
1. Compare `icon_256x256.png` with `../AppIcon.appiconset/appicon_256.png`
   — they should be the same artwork.
2. If they differ, re-run the generator rather than editing one side.

**Removing or renaming**
1. Expect a silent result — no `Contents.json` means no validation.
2. Verify by compiling on macOS and inspecting the resulting
   `../../../Assets.car`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `Assets.xcassets/` rules (XC3, XC5).
- [`../AppIcon.appiconset/AGENTS.md`](../AppIcon.appiconset/AGENTS.md) — the
  spec'd sibling set.
- [`../../AGENTS.md`](../../AGENTS.md) — `mac/` icon bundle.
