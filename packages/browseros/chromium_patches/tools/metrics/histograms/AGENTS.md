# `tools/metrics/histograms/` — histogram metadata root

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`. Parent scope:
> [`../AGENTS.md`](../AGENTS.md).

## What's here

A path-only chain. Chromium's UMA histogram metadata lives at
`tools/metrics/histograms/metadata/<component>/`; BrowserOS patches four
component folders under it. The directory itself contains no patch file.

## Contents

```
histograms/
└── metadata/
    ├── browser/enums.xml
    ├── extensions/enums.xml
    ├── sql/histograms.xml
    └── sync/enums.xml
```

Upstream siblings BrowserOS does **not** patch include `histograms.xml`,
`histograms/*.xml` component lists, and the `*_histograms.py` generator
scripts. The generator scripts stay unpatched on purpose — they are upstream
tooling, and modifying them would break on the next Chromium bump.

## Rules

**HST1 — Only `metadata/` is patched.** Do not add a BrowserOS generator script
here. The upstream `tools/metrics/histograms/*.py` scripts already handle
arbitrary enums; BrowserOS only supplies the data.

**HST2 — Four component folders, four unrelated features.** `browser` is
`agent-v2-infobar`, `extensions` is `api`, `sql` is `metrics`, `sync` is
unregistered. Adding a fifth folder means a new patch and a new
`features.yaml` decision.

**HST3 — This is a metadata directory, not a data directory.** Nothing here is
read at runtime. The browser reads compiled-in enum constants; the XML exists so
PRESUBMIT and the metrics dashboards can decode them.

## Workflows

**Adding a new patched metadata component**
1. Create the folder only when a real enum change needs it.
2. Edit the file in `<chromium_src>/tools/metrics/histograms/metadata/<comp>/`.
3. Extract to the mirrored path; register in `features.yaml`.

**Checking a mirror is consistent**
- Run `python tools/metrics/histograms/update_extension_histograms.py` in a
  Chromium checkout, or rely on the upstream PRESUBMIT job.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` subtree rules.
- [`metadata/AGENTS.md`](metadata/AGENTS.md) — the component folders.
- [`../../../AGENTS.md`](../../../AGENTS.md) — package overview, rule F2.
