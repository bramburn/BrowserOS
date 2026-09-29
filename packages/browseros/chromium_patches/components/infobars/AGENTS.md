# `components/infobars/` — agent-install infobar delegate id

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A one-subdirectory branch carrying a single addition to Chromium's infobar
identifier enum. `InfoBarDelegate::Identifier` gains
`BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE = 135`, the infobar shown while the
BrowserOS agent extension is installing or updating.

Feature block: **`agent-v2-infobar`**.

## Contents

```
infobars/
└── core/
    └── infobar_delegate.h   ← + BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE = 135
```

## Rules

**IF1 — The XML mirror is not optional.** `infobar_delegate.h` ends the enum
with
`// LINT.ThenChange(//tools/metrics/histograms/metadata/browser/enums.xml:InfoBarIdentifier)`.
The matching `<int value="135" label="BROWSEROS_AGENT_INSTALLING_INFOBAR_DELEGATE"/>`
lives in
[`tools/metrics/histograms/metadata/browser/enums.xml`](../../tools/metrics/histograms/metadata/browser/enums.xml)
and must ship in the same change.

**IF2 — Append only; never renumber.** Values below 135 are already recorded in
telemetry. The new entry goes immediately before the closing `};`.

**IF3 — `components/infobars/` contains no `.cc`.** The delegate *behaviour*
(the class that draws the infobar) lives under `chrome/browser/ui/` (e.g.
`chrome/browser/ui/startup/infobar_utils.cc`), outside this folder. Don't copy
it here.

## Workflows

**Adding a new BrowserOS infobar**
1. Append the identifier to `InfoBarDelegate::Identifier` in
   `<chromium_src>/components/infobars/core/infobar_delegate.h`; take the next
   free number.
2. Add the matching `<int value="N" label="..."/>` to
   `tools/metrics/histograms/metadata/browser/enums.xml`.
3. Extract both files back into `chromium_patches/`.
4. Implement the delegate under `chrome/browser/ui/startup/`.
5. Keep `components/infobars/...` and the XML under `agent-v2-infobar`.

**Renaming the label**
- The `label` attribute is the telemetry key. Prefer adding a new value over
  renaming one; a rename silently splits the metric.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `components/` subtree rules.
- [`../../tools/metrics/histograms/metadata/browser/AGENTS.md`](../../tools/metrics/histograms/metadata/browser/AGENTS.md) — the XML mirror.
- [`../../AGENTS.md`](../../AGENTS.md) — package overview.
- [`../../../build/features.yaml`](../../../build/features.yaml) — the `agent-v2-infobar` block.
