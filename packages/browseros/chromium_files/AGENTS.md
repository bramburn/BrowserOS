# `chromium_files/` — verbatim files copied over the Chromium tree

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/`](../AGENTS.md).

## What's here

Exactly four payload files, in four leaf directories — the non-Python,
non-diff half of the fork. `chrome/app/theme/chromium/BRANDING.debug` and
`BRANDING.release` are the product-identity strings for debug and release
builds; `chrome/enterprise_companion/branding.gni` and
`chrome/updater/branding.gni` are the ungoogled (non-`is_chrome_branded`)
branding variable sets for the enterprise companion app and the macOS/Windows
updater. Everything else in this tree is an empty path segment that exists
only so the Chromium-relative layout is preserved.

The consumer is `build/modules/resources/chromium_replace.py`, run by the
`chromium_replace` module in build phase 2. It `rglob`s this directory and
`shutil.copy2`s each file onto `ctx.chromium_src / <same relative path>` —
verbatim, unmerged, with no conflict resolution.

## Contents

```
chromium_files/
└── chrome/
    ├── app/theme/chromium/
    │   ├── BRANDING.debug          ← company/product/bundle-id for debug
    │   └── BRANDING.release        ← company/product/bundle-id for release
    ├── enterprise_companion/
    │   └── branding.gni            ← ungoogled enterprise-companion identity
    └── updater/
        └── branding.gni            ← ungoogled updater identity + COM GUIDs
```

## Rules

**CF1 — This is a copy, not a merge.** `chromium_replace.py` uses
`shutil.copy2` and never diffs. Every byte of a file here replaces the
Chromium original wholesale, so the file must be a *complete, buildable*
copy of the upstream file plus the BrowserOS delta — never a fragment.

**CF2 — The destination must already exist upstream.** The same module
raises `FileNotFoundError` when `dst_file` is absent in `chromium_src`
(`chromium_replace.py:81-87`). This tree cannot create a path that Chromium
does not already ship; use `chromium_patches/` with a `new file mode` hunk
for genuinely new files.

**CF3 — `.debug` / `.release` suffix is a build-type selector, not an
extension.** `BRANDING.debug` lands at `chrome/app/theme/chromium/BRANDING`
in a debug build; `BRANDING.release` is skipped. `--build-type` only accepts
`debug` or `release` (`build/cli/build.py:301-306`) — the variant comparison
in `chromium_replace.py:45-51` has no branch for any other value, so an
unset `build_type` walks both files and the last one copied wins.

**CF4 — A generic file is suppressed by a matching variant.** If
`X` and `X.debug` both exist, a debug build copies only `X.debug` and logs
"using debug variant instead" (`chromium_replace.py:63-75`). Do not add an
unsuffixed twin to a suffixed pair unless you want the unsuffixed one
dead for one build type.

**CF5 — The two `branding.gni` files must keep their `is_chrome_branded`
branches.** Both files gate the ungoogled values behind
`if (is_chrome_branded)`. Chromium selects the `else` branch for this fork,
but deleting the `if` branch breaks any Chrome-branded overlay and any
`--import` comparison build.

**CF6 — Changing identity means editing at least two files.** Product name
lives in `BRANDING.*` (browser identity) *and* in `updater/branding.gni`
(`browser_name`, `browser_appid`, `mac_browser_bundle_identifier`). Changing
one alone ships a browser whose updater still announces the old name.

**CF7 — Do not add files here without a `features.yaml` entry.**
`chrome/enterprise_companion/branding.gni` and `chrome/updater/branding.gni`
are listed as individual paths under the `branding` feature block in
`build/features.yaml`, which is what `browseros dev annotate` uses to group
commits. `chrome/app/theme/chromium/BRANDING.*` is only covered indirectly, by
the `chrome/app/theme/` directory entry — there is no per-file entry for it.

## Workflows

**Rebranding the product**
1. Edit `chrome/app/theme/chromium/BRANDING.release` (`COMPANY_FULLNAME`,
   `PRODUCT_FULLNAME`, `COPYRIGHT`, `MAC_BUNDLE_ID`) and the matching keys
   in `BRANDING.debug`.
2. Edit `chrome/updater/branding.gni`: `browser_name`,
   `browser_product_name`, `browser_appid`, `updater_appid`,
   `mac_browser_bundle_identifier`, `mac_updater_bundle_identifier`.
3. Edit `chrome/enterprise_companion/branding.gni`:
   `enterprise_companion_appid`, `enterprise_companion_crash_product_name`,
   `mac_enterprise_companion_bundle_identifier`.
4. Verify with `browseros build -m chromium_replace` and read the
   "Replaced N files" line.

**Replacing one of these files from a live Chromium checkout**
1. Edit the file in `<chromium_src>/` as usual.
2. Copy it over the same relative path under `chromium_files/`.
3. Re-run `browseros build -m chromium_replace` to confirm it is still copied.

**Debugging "Destination file not found in chromium_src"**
1. The relative path under `chromium_files/` does not exist at that path in
   the pinned Chromium tree — check `CHROMIUM_VERSION` / `BASE_COMMIT`.
2. Confirm you are not pointing at `chromium_patches/` (full replacement
   diffs) by mistake.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `packages/browseros/` (F3: new files).
- [`../build/features.yaml`](../build/features.yaml) — patch/feature manifest.
- [`../build/modules/resources/chromium_replace.py`](../build/modules/resources/chromium_replace.py) —
  the module that performs the copy.
- [`../build/cli/build.py`](../build/cli/build.py) — `--build-type` option.
- [`../../chromium_patches/chrome/app/AGENTS.md`](../chromium_patches/chrome/app/AGENTS.md) —
  the diff-based counterpart for `chrome/app/`.
