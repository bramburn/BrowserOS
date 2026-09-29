# `tools/` — UMA histogram metadata

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../chromium_patches/AGENTS.md`](../../chromium_patches/AGENTS.md).

## What's here

Chromium's telemetry metadata tree. Every C++ enum that BrowserOS extends has a
mirrored XML table under
`tools/metrics/histograms/metadata/<component>/`, and Chromium PRESUBMITs that
the two stay identical. This directory holds four patch files across four
component folders: the BrowserOS infobar enum addition, the `browserOs` API
histogram values and `kBrowserOS` permission id, the Chrome-importer SQL
variant, and two pinned-panel migration sync prefs.

## Contents

```
tools/
└── metrics/
    └── histograms/
        └── metadata/
            ├── browser/enums.xml      ← + BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE = 135
            ├── extensions/enums.xml   ← + BROWSER_OS_* = 1962..1986, + kBrowserOS = 266
            ├── sql/histograms.xml     ← + <variant name="ChromeImporter" .../>
            └── sync/enums.xml         ← + PinnedThirdPartyLlmMigrationComplete, PinnedClashOfGptsMigrationComplete
```

The single authoritative list of what belongs to which feature is
[`../../../../build/features.yaml`](../../build/features.yaml):

| File | Feature |
|---|---|
| `browser/enums.xml` | `agent-v2-infobar` |
| `extensions/enums.xml` | `api` |
| `sql/histograms.xml` | `metrics` |
| `sync/enums.xml` | *(not listed)* |

## Rules

**TME1 — Every enum change in this overlay has a mirror here.** Chromium's
`LINT.ThenChange` comments point at specific files. If you add an enumerator
anywhere in `components/infobars/`, `extensions/`, or `chrome/`, the
corresponding `enums.xml` changes in the same commit or PRESUBMIT fails.

**TME2 — Values are append-only and never compacted.** 135, 1962–1986, 266,
100381–100382 are all recorded in shipped telemetry. Removing a
`DELETED_*` entry splits the metric; renumbering corrupts it.

**TME3 — `sync/enums.xml` entries carry a migration, not a runtime value.**
`PinnedThirdPartyLlmMigrationComplete` and `PinnedClashOfGptsMigrationComplete`
are one-shot `ChromeSyncablePref` migration markers recorded in
`chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc` (under `chrome/`).
They exist so a one-time data migration can be observed exactly once.

**TME4 — `sql/histograms.xml` `<variant>` entries name the source database.**
`ChromeImporter` matches the `SqliteDatabaseType` variant used by
`chrome/utility/importer/browseros/chrome_*_importer.cc`. Adding an importer
without the variant yields an untyped telemetry record.

**TME5 — These are diffs, same as everything else in the overlay.** Edit the
real XML in `<chromium_src>/tools/metrics/histograms/metadata/`, then extract.

**TME6 — Do not reformat these XML files.** They are generated-adjacent and
large; the patches are surgical. A reformat turns a 3-line diff into a
thousand-line conflict generator.

**TME7 — The generator scripts must still run.** After changing an enum,
`tools/metrics/histograms/update_extension_histograms.py` and
`update_extension_permissions.py` are the upstream contract named in the
trailing comments of these files. Keep those comment anchors intact.

## Workflows

**Adding a `browserOs` extension API function**
1. Append the next `HistogramValue` in
   `chromium_patches/extensions/browser/extension_function_histogram_value.h`.
2. Add the matching `<int value="N" label="..."/>` to
   `metadata/extensions/enums.xml`.
3. Keep the `LINT.ThenChange(...extension_function_histogram_value.h:HistogramValue)`
   anchor in place.

**Adding a new infobar**
1. Append the identifier in `chromium_patches/components/infobars/core/infobar_delegate.h`.
2. Add the `<int>` to `metadata/browser/enums.xml`.

**Adding an extension permission**
1. Append `kFoo = <next>` to `chromium_patches/extensions/common/mojom/api_permission_id.mojom`.
2. Add `<int value="<next>" label="kFoo"/>` to the `ExtensionPermission3` enum
   in `metadata/extensions/enums.xml`.

**Adding a new importer source**
1. Add `<variant name="<Name>Importer" summary="<Name>Importer"/>` to
   `metadata/sql/histograms.xml`, in sorted position.

**Adding a one-shot syncable-pref migration**
1. Add the `ChromeSyncablePref` value in
   `chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc`.
2. Append `<int value="<next>" label="<PrefName>MigrationComplete"/>` to
   `metadata/sync/enums.xml` above the
   `<!-- LINT.ThenChange(/chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc:ChromeSyncablePref) -->`
   anchor.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — overlay conventions.
- [`metrics/histograms/AGENTS.md`](metrics/histograms/AGENTS.md) — the
  histograms subtree.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../components/infobars/core/AGENTS.md`](../components/infobars/core/AGENTS.md) — the infobar enum this mirrors.
- [`../../../extensions/browser/AGENTS.md`](../extensions/browser/AGENTS.md) — the histogram values this mirrors.
