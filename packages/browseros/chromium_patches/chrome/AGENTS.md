# `chrome/` — Chromium `chrome/` root overlay

> Part of [`../AGENTS.md`](../AGENTS.md) (`chromium_patches/`) in
> [`packages/browseros/`](../../AGENTS.md).

## What's here

The overlay for the Chromium `chrome/` directory root. Exactly one file
lives here: `BUILD.gn`. The patch wires three BrowserOS concerns into the
Chrome bundle target — the `//chrome/browser/browseros/server` resource
group, the `//chrome/browser/browseros/bundled_extensions` CRX bundle, and
the macOS-only Sparkle framework — and imports
`//chrome/browser/sparkle_buildflags.gni` so `enable_sparkle` is visible at
this level.

Every file under `chromium_patches/` is a **unified diff**, not a full file
copy. `BUILD.gn` here is a `diff --git a/chrome/BUILD.gn …` patch whose
hunks are applied by the `patches/apply` build module.

## Contents

```
chrome/
└── BUILD.gn     ← unified diff: bundle_deps for server resources +
                   bundled_extensions, Sparkle framework on macOS
```

## Rules

**CH1 — These are diffs, never whole files.** Every file in this tree starts
with `diff --git a/<path> b/<path>`. If you add a file here it must be a
git-formatted patch; a raw `.gn` or `.cc` will not apply.

**CH2 — The `a/` and `b/` paths must both be `chrome/...`.** The apply step
resolves the target from the diff header, not the directory layout. A patch
sitting in the wrong directory with the right header still applies, which
means a mistake here is silent.

**CH3 — Every new bundled directory needs two entries on Windows.** The
patch adds the target to `data_deps` (under `if (!is_android && !is_mac)`)
**and** to `bundle_deps` (under `if (is_win)`). Missing the second one
produces a binary that builds but does not ship the resource.

**CH4 — macOS-only additions must be guarded by `if (enable_sparkle)`.**
`enable_sparkle` comes from `//chrome/browser/sparkle_buildflags.gni`, which
this patch imports. Un-guarded framework references break Windows and Linux
builds.

**CH5 — A file in `chromium_patches/chrome/BUILD.gn` must also be listed
under a feature block in `build/features.yaml`** (`mac-sparkle-updater`,
`server`, or `ota-updater`). The `browseros dev annotate` step derives one
commit per feature from that manifest.

## Workflows

**Adding a new bundled resource directory**
1. Create the GN target under `chrome/browser/browseros/…` (that tree has its
   own `AGENTS.md`).
2. Add the label to `data_deps` in this `BUILD.gn` patch, and to `bundle_deps`
   in the `is_win` block.
3. Add the directory to the matching feature block in
   [`../../build/features.yaml`](../../build/features.yaml).
4. Verify the target resolves in `<chromium_src>/out/Default/chrome-win/`.

**Turning Sparkle on or off**
1. Edit `chrome/browser/sparkle_buildflags.gni`
   (`enable_sparkle = is_mac` today).
2. Every consumer in this tree must already import that `.gni`; add the import
   in the same patch if not.
3. Re-check the `enable_sparkle` guards here and in `chrome/test/BUILD.gn`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chromium_patches/` overlay rules.
- [`../../AGENTS.md`](../../AGENTS.md) — `packages/browseros/` build CLI.
- [`../../build/features.yaml`](../../build/features.yaml) — patch
  manifest, source of truth for which feature owns each file.
- [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md) —
  repo-wide map.
- [`../../../../AGENTS-build.md`](../../../../AGENTS-build.md) — build
  pipeline detail.
