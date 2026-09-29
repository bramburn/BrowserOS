# `chrome/app/theme/chromium/` — the `BRANDING` payload

> Part of [`../../../../AGENTS.md`](../../../../AGENTS.md) in
> [`chromium_files/`](../../../../AGENTS.md).

## What's here

The two build-type variants of Chromium's `BRANDING` file. Chromium parses
`KEY=value` lines out of this file at build time to derive the company and
product names, the installer titles, the copyright string and the macOS
bundle identity. `BRANDING.release` is what ships; `BRANDING.debug` is the
"BrowserOS Dev" variant used for `--build-type debug` builds.

Both files are copied wholesale over `<chromium_src>/chrome/app/theme/chromium/BRANDING`
by `build/modules/resources/chromium_replace.py`; the `.debug` / `.release`
suffix is stripped to compute the destination.

## Contents

| File | Lines | What it sets |
|---|---|---|
| `BRANDING.debug` | 10 | `BrowserOS Development` / `BrowserOS Dev`, bundle id `com.browseros.dev.BrowserOS`, `COPYRIGHT` with literal `@2025@` |
| `BRANDING.release` | 10 | `BrowserOS` / `BrowserOS Installer`, bundle id `com.browseros.BrowserOS`, `COPYRIGHT` with `@LASTCHANGE_YEAR@` |

Keys in both: `COMPANY_FULLNAME`, `COMPANY_SHORTNAME`, `PRODUCT_FULLNAME`,
`PRODUCT_SHORTNAME`, `PRODUCT_INSTALLER_FULLNAME`,
`PRODUCT_INSTALLER_SHORTNAME`, `COPYRIGHT`, `MAC_BUNDLE_ID`,
`MAC_CREATOR_CODE`, `MAC_TEAM_ID`.

## Rules

**BT1 — Keep the key set identical across the two files.** Chromium reads a
fixed key list; a key present in one variant and missing in the other fails
the build for that build type only, which is a slow, confusing failure.

**BT2 — `@2025@` and `@LASTCHANGE_YEAR@` are placeholders, not text.**
`BRANDING.debug` pins the literal token `@2025@` while `BRANDING.release`
uses `@LASTCHANGE_YEAR@`, which Chromium substitutes at build time. Do not
substitute a year by hand in the release file.

**BT3 — The only safe edit format is `KEY=value` on one line.** No quoting,
no comments, no trailing whitespace. The file is parsed as a simple
`key=value` list.

**BT4 — `MAC_TEAM_ID` is intentionally empty in both files.** macOS signing
and notarization are driven by the signing module, not by this file. Filling
it in without a matching Developer ID will fail `codesign` in phase 4.

**BT5 — `MAC_BUNDLE_ID` must stay distinct from the updater's.**
`BRANDING.*` uses `com.browseros.BrowserOS`; the updater bundle id lives in
`../../../updater/branding.gni` as `mac_updater_bundle_identifier`. They
must not collide.

**BT6 — These two files have no `features.yaml` entry of their own.** The
`branding` feature block lists `chrome/app/theme/` (a directory entry, not a
path to a patch — there is no diff for these files, they are whole-file
replacements) plus `chrome/updater/branding.gni` and
`chrome/enterprise_companion/branding.gni`. `browseros dev annotate` therefore
does not attribute these two files to a feature commit. Add an explicit entry
if you need them grouped.

## Workflows

**Changing the shipping product name**
1. Edit `BRANDING.release` (`COMPANY_*`, `PRODUCT_*`,
   `PRODUCT_INSTALLER_*`).
2. Mirror the change into `BRANDING.debug` with the "Dev" suffix kept.
3. Update `browser_name` / `browser_product_name` /
   `mac_browser_bundle_identifier` in `../../../updater/branding.gni` —
   see the sibling AGENTS.md.
4. Rebuild the string resources; the About page and installer titles read
   these values, not `chrome/app/app-Info.plist`.

**Debugging "Destination file not found in chromium_src"**
1. The build is running with `--build-type` unset or a value other than
   `debug` / `release`, so the variant branch in `chromium_replace.py:45-51`
   is not doing what you expect.
2. Confirm `<chromium_src>/chrome/app/theme/chromium/BRANDING` exists in the
   pinned tree before blaming this folder.

## Cross-references

- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `chromium_files/` rules.
- [`../../../updater/AGENTS.md`](../../../updater/AGENTS.md) — updater identity.
- [`../../../../build/modules/resources/chromium_replace.py`](../../../../../build/modules/resources/chromium_replace.py) —
  the copy + build-type variant logic.
- [`../../../../resources/icons/AGENTS.md`](../../../../../resources/icons/AGENTS.md) —
  icons copied into the *same* Chromium directory by a different module.
- [`../../../../chromium_patches/chrome/app/AGENTS.md`](../../../../../chromium_patches/chrome/app/AGENTS.md) —
  macOS bundle plist and command IDs (diff-based).
