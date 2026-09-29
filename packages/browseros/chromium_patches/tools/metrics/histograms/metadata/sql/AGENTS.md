# `tools/metrics/histograms/metadata/sql/` — SQL database telemetry types

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../tools/AGENTS.md`](../../../../../tools/AGENTS.md).

## What's here

The UMA metadata for Chromium's `SqliteDatabaseType` enum — the list of on-disk
SQLite databases that can be opened for telemetry inspection. BrowserOS adds one
variant, `ChromeImporter`, because the Chrome data importer opens the source
Chrome profile's databases.

Feature block: **`metrics`**.

## Contents

```
sql/
└── histograms.xml   ← + <variant name="ChromeImporter" summary="ChromeImporter"/>
```

The entry sits between `FirefoxImporter` and `FirstPartySets` — next to the other
importer variants rather than in the alphabetical slot it would get on a fresh
sort.

## Rules

**SQL1 — One `<variant>` per SQLite database type BrowserOS can open.** Each
corresponds to a `SqliteDatabaseType` value declared under
`chrome/browser/metrics/chrome_metrics_service_client.cc` (unpatched upstream
file, in `chrome/`). Adding a new importer database without a variant produces
telemetry records with no type.

**SQL2 — `name` and `summary` are separate attributes.** Both are `ChromeImporter`
today, but they are distinct fields: `name` is the identifier used by the
metrics tooling, `summary` is the dashboard label. Do not collapse them.

**SQL3 — Match the existing importer grouping, or re-sort the whole list.** The
`ChromeImporter` variant was added after `FirefoxImporter`, so the list is
*mostly* sorted with one exception. Do not append at the end — that guarantees a
future conflict with upstream CLs.

**SQL4 — The `<variants>` block is a flat list of `<variant>` children.** Don't
wrap entries in a group or add attributes Chromium's schema does not define.

**SQL5 — This is the only file under `metadata/sql/`.** Chromium keeps other SQL
telemetry files in that directory; BrowserOS patches none of them.

## Workflows

**Adding telemetry for a new imported database**
1. Add the `SqliteDatabaseType` enumerator in
   `chrome/browser/metrics/chrome_metrics_service_client.cc`.
2. Add `<variant name="<Name>" summary="<Name>"/>` here in sorted position.
3. Extract the XML; keep it under the `metrics` feature.
4. Confirm the importing code declares the database type when opening it
   (`chrome/utility/importer/browseros/chrome_*.cc`).

**Debugging "telemetry has no database type"**
1. Check the `<variant>` entry exists here.
2. Check the enumerator exists in `chrome_metrics_service_client.cc`.
3. Check the importer passes the right type when opening the database.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — metadata folder rules.
- [`../../../../components/user_data_importer/AGENTS.md`](../../../../../components/user_data_importer/AGENTS.md) — the importer this covers.
- [`../../../../../build/features.yaml`](../../../../../../build/features.yaml) — the `metrics` block.
