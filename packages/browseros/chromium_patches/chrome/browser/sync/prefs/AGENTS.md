# `sync/prefs/` — syncable-pref enum and allowlist

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Chromium's master table of which Chrome preferences sync and how. The file
holds two things that must move together: an append-only
`syncable_prefs_ids::` enum of numeric pref IDs, and
`kChromeSyncablePrefsAllowlist`, a `base::MakeFixedFlatMap` mapping each
`prefs::kX` string to `{id, syncer type, PrefSensitivity, MergeBehavior}`.
BrowserOS appends two entries at the end of the enum —
`kPinnedThirdPartyLlmMigrationComplete = 100381` and
`kPinnedClashOfGptsMigrationComplete = 100382`, both under a
`// BrowserOS: sync pref IDs` comment — and adds their two allowlist rows
inside the same `#if BUILDFLAG(IS_ANDROID)` block as the neighbouring
`kProjectsPanelEntrypointEnabled`.

## Contents

```
prefs/
└── chrome_syncable_prefs_database.cc   ← + kPinnedThirdPartyLlmMigrationComplete = 100381
                                           + kPinnedClashOfGptsMigrationComplete = 100382
                                           + 2 rows in kChromeSyncablePrefsAllowlist
                                           (kNone sensitivity, kNone merge behavior)
```

## Rules

**SYNCP1 — Never renumber or remove an ID.** The enum comment is explicit:
"Never remove any existing values, as this enum is used to bucket a UMA
histogram, and removing values breaks that." The IDs are also written into
every user's sync state as the on-the-wire key. Append only; the next free
value is `100383`.

**SYNCP2 — The enum and the allowlist row are one change.** A constant with no
map row does not sync. A map row referencing a non-existent constant does not
compile. They are in the same file precisely so they stay together.

**SYNCP3 — Sensitivity and merge behaviour must be chosen deliberately.**
Both BrowserOS rows use `PrefSensitivity::kNone` and `MergeBehavior::kNone`
because they are one-shot migration flags, not user settings. A row that
carries real user state needs a considered sensitivity, not a copy-paste of
`kNone`.

**SYNCP4 — The new rows live inside `#if BUILDFLAG(IS_ANDROID)`.**
They sit in the same guarded block as `kProjectsPanelEntrypointEnabled`. Moving
them out of the guard changes which platforms sync these flags; that is a
behaviour change, not a cleanup.

**SYNCP5 — `components/sync_preferences/README.md` is the upstream process
document.** It is referenced in the file itself. Read it before adding an ID;
Chromium's presubmit enforces the ordering and the "note to the reviewer"
requirements.

**SYNCP6 — "Migration complete" prefs are write-once flags.** They are set by a
one-time migration in the UI code and never cleared. Treat them as durable
state: renaming the string constant orphans the synced value.

## Workflows

**Adding a new syncable BrowserOS pref**
1. Read `components/sync_preferences/README.md` in the Chromium tree.
2. Append the next free ID to the `syncable_prefs_ids::` enum under a
   `// BrowserOS:` comment (SYNCP1).
3. Add the matching `kChromeSyncablePrefsAllowlist` row with explicit
   `PrefSensitivity` and `MergeBehavior` (SYNCP2, SYNCP3).
4. Decide the platform guard deliberately (SYNCP4).
5. Re-extract; the patch should stay three small hunks.

**Finding the ID of an existing pref**
1. Grep the `syncable_prefs_ids::` enum for the constant name.
2. Grep `kChromeSyncablePrefsAllowlist` for the row to see the wire type,
   sensitivity and merge behaviour.
3. Both must exist; neither is authoritative alone.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `sync/` parent.
- [`../../prefs/AGENTS.md`](../../prefs/AGENTS.md) — where the underlying
  BrowserOS prefs are registered.
- [`../../browseros/core/AGENTS.md`](../../browseros/core/AGENTS.md) — the
  `browseros.*` pref constants.
- [`../../../../../build/features.yaml`](../../../../../build/features.yaml) —
  patch manifest.
