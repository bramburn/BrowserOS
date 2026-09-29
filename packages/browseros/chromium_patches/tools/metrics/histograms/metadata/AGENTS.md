# `tools/metrics/histograms/metadata/` — telemetry metadata root

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

The root of Chromium's UMA metadata tree. Each child folder is a telemetry
*component* with its own `enums.xml` (or `histograms.xml`) declaring the numeric
values of C++ enums. BrowserOS patches four of them, each for a different
feature.

## Contents

```
metadata/
├── browser/enums.xml      ← InfoBarIdentifier +135
├── extensions/enums.xml   ← HistogramValue +1962..1986, ExtensionPermission3 +266
├── sql/histograms.xml     ← <variant name="ChromeImporter">
└── sync/enums.xml         ← ChromeSyncablePref +100381, +100382
```

Upstream component folders BrowserOS does not patch include `arc`, `blink`,
`browser_signin`, `component`, `dom`, `histograms`, `navigation`,
`networking`, `pdf`, `power`, `sys`, `tts`, `ukm`, `v8`, and more. They stay
untouched.

## Rules

**MDT1 — Metadata is data, not code.** Nothing here is compiled or read at
runtime. Its only job is to let Chromium PRESUBMIT and the metrics dashboards
decode numbers emitted by C++ enums.

**MDT2 — The C++ enum is always the source of truth.** When the two disagree,
the C++ wins and the XML is regenerated (by the upstream
`tools/metrics/histograms/*.py` scripts). Never "fix" the C++ to match the XML.

**MDT3 — One component folder per feature concern, not per feature.** The
mapping is by telemetry component, so two features can share a folder
(`agent-v2-infobar` → `browser`, `api` → `extensions`).

**MDT4 — Adding a component folder is a deliberate act.** Create it only when a
real enum change needs a home; do not mirror Chromium's full tree.

**MDT5 — Never add BrowserOS here for a runtime setting.** Prefs, flags, and
constants belong in `chrome/browser/browseros/core/`; this tree is telemetry
decoding only.

## Workflows

**Deciding where a new enum's metadata goes**
1. Find the enum's component in Chromium's tree (it maps to a `metadata/<component>/enums.xml`).
2. Confirm BrowserOS patches that folder; if not, add the folder and register it
   in `features.yaml`.
3. Add the entries in sorted value order.
4. Keep the file's `LINT.ThenChange` / `LINT.IfChange` anchors intact.

**Regenerating a mirror after a Chromium bump**
1. Apply the tree in a Chromium checkout.
2. Run the component's `update_*.py` script.
3. Copy the resulting diff into `chromium_patches/tools/metrics/...` via
   `browseros dev extract`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/metrics/histograms/` rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — package overview, rule F2.
- [`../../../../build/features.yaml`](../../../../../build/features.yaml) — feature ownership per file.
