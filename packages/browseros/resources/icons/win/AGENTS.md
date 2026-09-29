# `resources/icons/win/` — Windows icons and Start tiles

> Part of [`../../AGENTS.md`](../../AGENTS.md) in
> [`resources/icons/`](../../AGENTS.md).

## What's here

The Windows icon set: five `.ico` bundles for the executable, document
associations and incognito, plus the `tiles/` subdirectory holding the Start
menu tile PNGs. Copied whole to `chrome/app/theme/chromium/win` by the
`Windows Icons` entry in `build/config/copy_resources.yaml` — no `os:
[windows]` filter, so the directory is copied into every build.

## Contents

| File | Notes |
|---|---|
| `chromium.ico` | 43,644 bytes — main executable icon |
| `app_list.ico` | 43,644 bytes — same byte size as `chromium.ico` |
| `incognito.ico` | 43,644 bytes — same byte size as `chromium.ico` |
| `chromium_doc.ico` | 26,039 bytes — document icon |
| `chromium_pdf.ico` | 26,039 bytes — same byte size as `chromium_doc.ico` |
| `tiles/Logo.png` | 10,559 bytes — Start tile, large |
| `tiles/SmallLogo.png` | 4,374 bytes — Start tile, small |

The identical byte sizes are expected: three icons share one artwork at
three sizes, two share another.

## Rules

**WN1 — `.ico` bundles are multi-size and generated.** The generator's `ICO`
operation takes a comma-separated size list. Hand-editing an `.ico` cannot
be validated by a build step; a malformed one fails resource compilation
in the ninja phase, hours in.

**WN2 — Icon *order* and *count* are contractual.** Chromium's
`chrome/installer/util/shell_util.cc` builds shortcuts by index into these
`.ico` files. Reordering or removing an image inside a bundle changes which
artwork a user sees on an existing pinned shortcut.

**WN3 — `chromium.ico` must stay the executable icon at index 0.** It is the
fallback in `chrome/app/chrome_exe.rc` and `chrome/app/chrome_dll.rc`, both
of which the `windows-disable-rcpy` series patch rewrites
(`series_patches/windows/ungoogled-chromium/`). A change to this file must
be consistent with those `.rc` edits.

**WN4 — `tiles/` is covered by the same directory copy.** There is no
separate `copy_operations` entry for it; `Logo.png` / `SmallLogo.png` land at
`chrome/app/theme/chromium/win/tiles/`.

**WN5 — No `os: [windows]` filter on the copy.** The directory is shipped
in macOS and Linux builds too. Adding a filter is a behaviour change, not a
cleanup.

## Workflows

**Changing the Windows icon**
1. Update `source/app_icon.png` under
   `build/scripts/icon_generation/source/`.
2. Run the generator; expect all five `.ico` files and both `tiles/*.png`
   to change together.
3. Run `browseros build -m resources`, then the `series_patches` module —
   the `.rc` files that consume these icons are patched there.

**Debugging a missing or wrong Windows icon**
1. Wrong artwork on an existing shortcut → the `.ico` image order changed
   (WN2).
2. `RC` / resource compile error late in the build → a hand-edited `.ico`
   (WN1).
3. Icon present but path-not-found in the build → check
   `chrome/app/chrome_exe.rc` still points at
   `theme\chromium\win\chromium.ico` after the series patches applied.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `icons/` rules and destination table.
- [`tiles/AGENTS.md`](tiles/AGENTS.md) — the Start tile set.
- [`../../../build/config/copy_resources.yaml`](../../../build/config/copy_resources.yaml) —
  the unconditional `Windows Icons` operation.
- [`../../../series_patches/windows/ungoogled-chromium/AGENTS.md`](../../../series_patches/windows/ungoogled-chromium/AGENTS.md) —
  the `.rc` patches that reference these icons.
- [`../../../build/scripts/icon_generation/README.md`](../../../build/scripts/icon_generation/README.md) —
  the `ICO` operation.
