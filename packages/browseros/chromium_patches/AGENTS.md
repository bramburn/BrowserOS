# `chromium_patches/` — the BrowserOS patch overlay

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Manifest of
> record is [`../build/features.yaml`](../build/features.yaml).

## What's here

366 files, each of which is a **unified diff** (`git apply -p1`) against one
file in a vanilla Chromium 148.0.7778.97 checkout. This directory is not
source code you compile — it is the overlay the build copies onto
`<chromium_src>/` after `gclient sync`. There are 48 files outside `chrome/`;
the remaining 318 are `chrome/**`.

The overlay does three things: new BrowserOS behaviour (`base/`, `components/`,
`content/`, `extensions/`), CDP surface for the agent (`third_party/blink/`,
`content/browser/devtools/`), and platform windowing for the agent's hidden
windows (`ui/`, `components/remote_cocoa/`).

## Contents

```
chromium_patches/
├── base/          ← version string plumbing + thread-restriction friends
├── chrome/        ← the bulk: browseros/, extensions API, devtools handlers
├── components/    ← pref defaults, vector icons, os_crypt, importer enums
├── content/       ← CDP target/tab identity plumbing
├── extensions/    ← permanent SW keepalives + permission id
├── third_party/   ← blink .pdl protocol defs, libxml visibility, sparkle
├── tools/         ← UMA histogram metadata (enum tables)
└── ui/            ← headless window init param, per-platform native impls
```

Each file's *path* mirrors the Chromium path (parent rule F2); each file's
*content* is a diff, so its `diff --git a/<path> b/<path>` header must match
its own location. Files with `new file mode 100644` create a file that does
not exist in stock Chromium (e.g. `third_party/blink/public/devtools_protocol/domains/Bookmarks.pdl`).

## Rules

**CP1 — These are diffs, not replacements.** Do not paste full file contents
here. `build/modules/apply/common.py:apply_single_patch` runs
`git apply --ignore-whitespace --whitespace=nowarn -p1` and retries with
`--3way`. A file that looks like plain source will fail to apply.

**CP2 — `diff --git` header must match the file's own path.** The path is how
`ctx.get_patch_path_for_file()` finds the patch. A moved file breaks lookup
silently.

**CP3 — Prefer editing in-tree, then `browseros dev extract`.** Edit the real
file under `<chromium_src>/`, then let
`build/modules/extract/extract_patch.py` regenerate the diff against
`BASE_COMMIT`. Hand-writing hunk headers (`@@ -450,7 +450,7 @@`) is how
off-by-N drift gets introduced.

**CP4 — Register every file in `features.yaml`.** The `files:` list is
per-feature and becomes the commit message. An unlisted patch is applied but
never annotated.

**CP5 — Don't renumber enums.** Every enum value added here (`InfoBarIdentifier`,
`APIPermissionID`, `HistogramValue`, `ChromeSyncablePref`) is persisted to
telemetry and must keep its number. Append only.

**CP6 — LINT.ThenChange pairs are atomic.** A C++ enum and its
`tools/metrics/histograms/metadata/**/enums.xml` mirror must land in the same
change, or `tools/metrics/histograms/PRESUBMIT.py` fails.

**CP7 — `ui/`, `components/remote_cocoa/`, and branding-adjacent files are
fragile.** They sit on per-platform code paths that are hard to compile on this
Windows host. Treat a conflict there as a stop-and-report, not a merge.

## Workflows

**Modifying an existing patched file**
1. Find its patch: `chromium_patches/<mirror-path>`.
2. Edit the file inside `<chromium_src>/<mirror-path>` (not the diff) and
   rebuild incrementally.
3. Run `browseros dev extract <mirror-path>` to regenerate the diff.
4. Re-add the path to the same `features.yaml` feature block.

**Adding a patch to a vanilla Chromium file**
1. Pick the Chromium path, mirror it exactly (F2).
2. Edit in `<chromium_src>/`, then extract.
3. Add the mirror path to a `features.yaml` feature.
4. Dry-run with `browseros dev apply --dry-run`.

**Adding a brand-new file to Chromium**
1. Create it at `<chromium_src>/<path>`; extraction emits a
   `new file mode 100644` diff under `chromium_patches/<path>`.
2. Note: new files still land in `chromium_patches/`, not `chromium_files/`
   (which holds only 4 files, all under `chrome/`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — package overview, rules F1–F7.
- [`../build/features.yaml`](../build/features.yaml) — patch manifest.
- [`../build/modules/apply/common.py`](../build/modules/apply/common.py) — how patches are applied.
- [`../build/modules/extract/extract_patch.py`](../build/modules/extract/extract_patch.py) — how patches are generated.
- [`../CHROMIUM_VERSION`](../CHROMIUM_VERSION) — pinned version the diffs were cut against.
