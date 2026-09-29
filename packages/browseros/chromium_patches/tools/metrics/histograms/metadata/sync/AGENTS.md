# `tools/metrics/histograms/metadata/sync/` — syncable-pref migration markers

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../tools/AGENTS.md`](../../../../../tools/AGENTS.md).

## What's here

The UMA metadata mirror for Chromium's `ChromeSyncablePref` enum, which labels
syncable pref migrations for telemetry. BrowserOS adds two entries recording
one-shot data migrations for the pinned third-party-LLM and Clash-of-GPTs
toolbar panels.

This file is **not listed in any `features.yaml` feature block** — see SY3.

## Contents

```
sync/
└── enums.xml   ← + <int value="100381" label="PinnedThirdPartyLlmMigrationComplete"/>
                 + <int value="100382" label="PinnedClashOfGptsMigrationComplete"/>
```

The additions sit immediately above the anchor:

```xml
<!-- LINT.ThenChange(/chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc:ChromeSyncablePref) -->
```

## Rules

**SYN1 — These are one-shot migration markers, not runtime prefs.** The
`ChromeSyncablePref` enum labels the *migration* that moved a user's pinned-panel
configuration, so the migration can be counted exactly once. Adding an entry
here without a corresponding migration in
`chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc` produces a metric
that is never emitted.

**SYN2 — The anchor path is absolute, not `//`-prefixed.** It reads
`/chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc:ChromeSyncablePref`,
unlike the `//`-prefixed anchors elsewhere. Preserve it exactly — the PRESUBMIT
parser is literal.

**SYN3 — Unregistered in `features.yaml`.** The patch applies by path, but it
will not appear in an annotated commit. When you touch this file, add it to the
`pin-chat-and-hub` feature (which owns the pinned-panel pref work).

**SYN4 — 100381/100382 are the current high-water mark.** The next value is
100383. Verify against current upstream before allocating.

**SYN5 — Do not reorder the `IosSyncablePref` block that follows.** The
`<!-- LINT.IfChange(IosSyncablePref) -->` marker separates a second enum in the
same file; BrowserOS only touches the `ChromeSyncablePref` block above it.

**SYN6 — Keep the additions above the `LINT.ThenChange` anchor.** Inserting
below it puts the entries outside the guarded block and PRESUBMIT will not see
them.

## Workflows

**Adding a migration for a new BrowserOS pref**
1. Add the migration to
   `chrome/browser/sync/prefs/chrome_syncable_prefs_database.cc` under the
   `ChromeSyncablePref` enum.
2. Append `<int value="<next>" label="<Name>MigrationComplete"/>` here, above
   the `LINT.ThenChange` anchor.
3. Add both paths to `features.yaml` (suggest `pin-chat-and-hub`).
4. Verify the migration actually runs for an upgrading profile.

**Debugging a PRESUBMIT enum mismatch**
1. Compare the C++ enumerator order and values with the `<int>` list, in order.
2. The order matters, not just the set — the PRESUBMIT script compares
   positionally for some enums.
3. Re-extract from `<chromium_src>` rather than editing the diff.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — metadata folder rules.
- [`../../../../../build/features.yaml`](../../../../../../build/features.yaml) — feature manifest (`pin-chat-and-hub` is the natural home).
- [`../../../../../../AGENTS.md`](../../../../../../AGENTS.md) — package overview.
