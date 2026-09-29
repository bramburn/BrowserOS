# `sync/` — syncable-pref table

> Part of [`../AGENTS.md`](../AGENTS.md) (`chrome/browser/`) in
> [`packages/browseros/`](../../../../AGENTS.md).

## What's here

A directory with no files of its own. The only patched file is
`prefs/chrome_syncable_prefs_database.cc`, which is documented in
`prefs/AGENTS.md`. This file exists so the syncable-pref table has a home in
the tree.

## Contents

```
sync/
└── prefs/
    └── chrome_syncable_prefs_database.cc   ← see prefs/AGENTS.md
```

## Rules

**SYNC0 — There are no sources directly in `sync/`.** Do not add a patch at
this level; a new syncable pref belongs in `sync/prefs/`, and any sync *data
type* change belongs in `../../../components/sync/`.

**SYNC1 — Syncable pref IDs are a persisted, append-only enum.**
`chrome_syncable_prefs_database.cc` carries an `enum` of
`syncable_prefs_ids::` values ending at `100382` and an explicit reviewer
warning ("IT IS YOUR RESPONSIBILITY to ensure that new syncable preferences…").
Any change there is a data-format change for every existing profile; read
`components/sync_preferences/README.md` before touching it.

**SYNC2 — The pref name and the ID must be added together.**
Each row in `kChromeSyncablePrefsAllowlist` maps a `prefs::kX` string to a
`syncable_prefs_ids::kX` ID, a `syncer::` type, a `PrefSensitivity` and a
`MergeBehavior`. A constant with no row is not synced; a row with a missing
constant does not compile.

## Workflows

**Adding a syncable BrowserOS pref**
1. Declare the pref in `../../../chrome/common/prefs/` (or
   `../browseros/core/browseros_prefs.h` for `browseros.*` keys).
2. Append a new `kX = <next id>` to the `syncable_prefs_ids::` enum in
   `prefs/chrome_syncable_prefs_database.cc` — never renumber.
3. Add the matching row to `kChromeSyncablePrefsAllowlist` with an explicit
   `PrefSensitivity` and `MergeBehavior`.
4. Extract the diff and add the path to the `features.yaml` block that already
   lists this file.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `chrome/browser/` overlay.
- [`prefs/AGENTS.md`](prefs/AGENTS.md) — the allowlist and enum in detail.
- [`../prefs/AGENTS.md`](../prefs/AGENTS.md) — BrowserOS pref registration.
- [`../../../../build/features.yaml`](../../../../build/features.yaml) — patch manifest.
