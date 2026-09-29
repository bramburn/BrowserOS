# `series_patches/ungoogle-chromium/` — cross-platform Ungoogled patches

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`series_patches/`](../AGENTS.md).

## What's here

One patch, `extensions-manifestv2.patch`, inherited from the Ungoogled
Chromium project. It is the only entry in the common `series` file, so it
runs on **every** platform on every build that invokes the `series_patches`
module.

The patch restores Manifest V2 extension support: it strips Chromium's MV2
deprecation experiments (`kExtensionManifestV2Disabled` /
`kExtensionManifestV2Unsupported`), the enterprise `manifest_v2_setting`
policy switch, the MV2-unpacked-extension install warning, and forces the
`extensions_ui.cc` Safety Hub three-dot-detail flag to `false`.

## Contents

`extensions-manifestv2.patch` — 160 lines, five files:

| Target file | Change |
|---|---|
| `chrome/browser/extensions/api/developer_private/extension_info_generator.cc` | drop the MV2 deprecation fields from `FillExtensionInfo` |
| `chrome/browser/extensions/extension_management.cc` | `IsAllowedManifestVersion` returns `true`; `IsExemptFromMV2DeprecationByPolicy` reduced to the enabled case |
| `chrome/browser/extensions/manifest_v2_experiment_manager.cc` | `CalculateCurrentExperimentStage` always returns `kWarning`; `ShouldDisableLegacyExtensions` always `false` |
| `chrome/browser/ui/webui/extensions/extensions_ui.cc` | `safetyHubThreeDotDetails` hard-coded to `false` |
| `extensions/common/extension.cc` | no `kManifestV2IsDeprecatedWarning` for unpacked MV2 extensions |

## Rules

**UC1 — It runs on every platform via the common `series` file.** Removing
the line from `series` disables MV2 support everywhere, not just on one OS.
`series.linux` and `series.macos` are empty; this is the only patch outside
the Windows set.

**UC2 — The patch is upstream-Ungoogled, not Chromium-structured.** It is a
plain `git diff` with `a/` `b/` prefixes, applied with `-p1` and a `--3way`
retry. It has no counterpart in `features.yaml`, so `browseros dev annotate`
does not group it and it gets no per-feature commit.

**UC3 — `--3way` is its safety net, not its design.** A Chromium release
that renames `manifest_v2_experiment_manager.cc` or changes
`IsAllowedManifestVersion`'s signature breaks this patch. When that
happens, do not widen the context lines — regenerate against the new
`BASE_COMMIT` or move the change into
`../../chromium_patches/chrome/browser/extensions/`.

**UC4 — The two policy-dependent behaviours are now constants.** After this
patch, `global_settings_->manifest_v2_setting` is effectively ignored and
`ManifestV2ExperimentManager` never advances past `kWarning`. A future
change that wants policy-controlled MV2 has to reverse this patch, not add a
flag next to it.

**UC5 — Keep the five files in one patch.** The MV2 behaviour is spread
across policy, experiment-manager, install-warning and WebUI surfaces; a
partial reapplication (e.g. one file moving to `chromium_patches/`) leaves
the browser in a state where the UI shows no warning but policy still
rejects the extension.

## Workflows

**Applying the patch to a fresh Chromium checkout**
1. Ensure the tree matches `../../CHROMIUM_VERSION` / `BASE_COMMIT`.
2. Run `browseros build -m series_patches`.
3. Expect `Applied 1 series patches`.

**Re-basing after a Chromium version bump**
1. Re-fetch and check out the new tag.
2. Run `browseros build -m series_patches --dry-run`-equivalent (the module
   supports `dry_run` in `apply_series_patches_impl`, but it is not exposed
   on the CLI — run the module and read the per-patch `✗ Failed` lines).
3. Regenerate the diff against the new tree and replace the file, keeping
   the same path so `series` needs no edit.

**Disabling MV2 support entirely**
1. Comment out the `extensions-manifestv2.patch` line in `../series`.
2. Do not delete the patch file — it stays as the record of the delta, and
   re-enabling is a one-line uncomment.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `series_patches/` rules (SP1-SP8).
- [`../series`](../series) — the `series` file that names this patch.
- [`../build/modules/patches/series_patches.py`](../../build/modules/patches/series_patches.py) —
  the applier and its `--3way` fallback.
- [`../../chromium_patches/chrome/app/AGENTS.md`](../../chromium_patches/chrome/app/AGENTS.md) —
  the diff-based counterpart tree.
- [`../../../AGENTS-build.md`](../../../../AGENTS-build.md) — patch-system
  overview.
