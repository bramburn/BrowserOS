# `tools/metrics/histograms/metadata/browser/` — infobar identifier mirror

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../tools/AGENTS.md`](../../../../../tools/AGENTS.md).

## What's here

The UMA metadata mirror for Chromium's `InfoBarIdentifier` enum. One `<int>`
entry is added, and it exists only because
`components/infobars/core/infobar_delegate.h` was extended in the same feature.

Feature block: **`agent-v2-infobar`**.

## Contents

```
browser/
└── enums.xml   ← + <int value="135" label="BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE"/>
```

The enum block ends with the anchor:

```xml
<!-- LINT.ThenChange(//components/infobars/core/infobar_delegate.h:InfoBarIdentifier) -->
```

## Rules

**BRW1 — One `<int>`, one C++ enumerator, same commit.** The
`LINT.ThenChange` anchor is enforced by upstream PRESUBMIT; a mismatch fails
the build, not just the review.

**BRW2 — 135 is frozen.** The new entry is appended after
`WEB_APP_BLOCKED_MIGRATION_INFOBAR_DELEGATE = 134`. Upstream infobars get
removed and renumbered, so the *next* BrowserOS value must be re-checked
against a fresh Chromium, not assumed to be 136.

**BRW3 — The `label` is the telemetry key.** Renaming
`BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE` splits the metric. Prefer adding
a new value over renaming.

**BRW4 — Do not touch the neighbouring `<int>` entries.** The hunk context is
132/133/134; only the addition is ours.

**BRW5 — `browser/enums.xml` holds many enums.** The BrowserOS change is in the
`InfoBarIdentifier` block only. Other blocks in this file (feature lists, API
enums) are untouched upstream data.

## Workflows

**Adding a second BrowserOS infobar**
1. Pick the next free identifier after checking current upstream `enums.xml`.
2. Add the enumerator to `components/infobars/core/infobar_delegate.h`.
3. Add the `<int>` here in the same change.
4. Keep both paths under `agent-v2-infobar` in `features.yaml`.
5. Implement the delegate under `chrome/browser/ui/startup/`.

**A PRESUBMIT `LINT.IfChange` failure**
1. Read the reported file:line.
2. Compare the C++ enum and this XML line-for-line, including the value.
3. Re-extract from `<chromium_src>` rather than hand-editing the diff.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — metadata folder rules.
- [`../../../../components/infobars/core/AGENTS.md`](../../../../../components/infobars/core/AGENTS.md) — the C++ enum this mirrors.
- [`../../../../../build/features.yaml`](../../../../../../build/features.yaml) — the `agent-v2-infobar` block.
