# `tools/metrics/` — telemetry metadata root

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../../tools/AGENTS.md`](../../tools/AGENTS.md).

## What's here

A path-only directory. Chromium's metrics tooling lives at
`tools/metrics/<subsystem>/` (histograms, structured logging, trace events,
v8 …); BrowserOS patches only the histogram metadata subtree below it.

## Contents

```
metrics/
└── histograms/
    └── metadata/
        ├── browser/enums.xml
        ├── extensions/enums.xml
        ├── sql/histograms.xml
        └── sync/enums.xml
```

Upstream siblings under `tools/metrics/` that BrowserOS does not touch:
`histograms/`'s generator scripts (`update_extension_histograms.py`,
`update_extension_permissions.py`, …), `structured_logging/`, `trace_event/`,
`v8/`, and `protocol/`.

## Rules

**TMM1 — Only `histograms/metadata/` is patched; the generator scripts are
not.** The upstream `tools/metrics/histograms/*.py` scripts regenerate the XML
from the C++ enums. Patching them would break on the first Chromium bump
because they are rewritten whenever the enum format changes.

**TMM2 — Metadata is data, never behaviour.** Nothing under this tree is
compiled into the browser or read at runtime. Its only consumer is upstream
PRESUBMIT and the metrics dashboards.

**TMM3 — The C++ enum is the source of truth.** When the two disagree, re-run
the upstream generator and re-extract. Never edit the C++ to match the XML.

**TMM4 — Do not mirror Chromium's full `tools/metrics/` tree.** Four component
folders are patched because four enums gained BrowserOS values. Create a fifth
only when a real enum change needs a home.

**TMM5 — Adding a metric is a `chrome/` change first.** New histograms are
emitted from `chrome/browser/browseros/metrics/`. This folder only decodes them.

## Workflows

**Adding a BrowserOS metric**
1. Emit the histogram from `chrome/browser/browseros/metrics/`.
2. If it uses a custom enum, add the `<int>` to the matching
   `metadata/<component>/enums.xml`.
3. Register the XML path in `features.yaml` under the emitting feature.

**Checking the mirrors are consistent**
- Run the relevant `update_*.py` in a Chromium checkout with the overlay
  applied; whatever it changes is authoritative. Re-extract rather than
  hand-editing the diff.

**Deciding the component folder for a new enum**
1. Chromium already declares the enum in some `metadata/<component>/enums.xml`
   — use that folder.
2. If it does not, pick the folder matching the enum's owning subsystem
   (`browser`, `extensions`, `sync`, `sql`, …).
3. Add the BrowserOS entries in sorted value order, keeping the
   `LINT.ThenChange` / `LINT.IfChange` anchors intact.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` subtree rules and the per-file feature map.
- [`histograms/AGENTS.md`](histograms/AGENTS.md) — the histograms subtree.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
- [`../../../build/features.yaml`](../../../build/features.yaml) — feature ownership per file.
