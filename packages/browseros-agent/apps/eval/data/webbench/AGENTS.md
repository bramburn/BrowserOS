# `data/webbench/` — Raw benchmark exports (CSV)

> Part of [`../AGENTS.md`](../AGENTS.md) in `data/`; upstream task data, not eval input.

## What's here

Nine CSV exports of the WebBench / BrowseComp task pools that were used to
assemble the benchmark leaderboard, plus the BrowseComp list. These are the
**sources** for `../webbench-*.jsonl` and `../browsecomp-*.jsonl`; the harness
never reads them directly.

```
data/webbench/
├── webbenchfinal.csv              ← WebBench task pool
├── anthropicfinal.csv             ← competitor run results
├── browserusefinal.csv
├── convergencehitlfinal.csv
├── openaicuafinal.csv
├── operatorhitlfinal.csv
├── rtrvrfinal.csv
├── skyvern2.0browserbasefinal.csv
├── skyvern2.0final.csv
└── browsecomp.csv                 ← encrypted BrowseComp rows (canary key)
```

## Rules

### WB1 — Read-only inputs to the builders
`../scripts/build-webbench-sets.py` and `../scripts/build-browsecomp-sets.py`
read these and write the JSONL slices one level up. Editing a CSV changes a
future build only.

### WB2 — The `*final.csv` files are results, not tasks
The competitor files record other agents' outcomes on the same tasks. They are
kept for comparison, not executed. Do not convert them into tasks by accident —
check the header/column shape before feeding a file to a builder.

### WB3 — BrowseComp rows are encrypted
`../scripts/build-browsecomp-sets.py` decrypts with an XOR against the row's
canary field (the scheme from OpenAI's `simple-evals`). The CSVs here are
therefore unreadable without that step; the decryption happens in the builder,
not by hand.

### WB4 — Large and binary-adjacent: don't reformat
These files are ~0.2–2.7 MB each (~10 MB total). Line-ending normalisation,
BOM insertion, or a CSV round-trip will change the row parsing the builders
depend on. If a file must be touched, verify the builder still produces the
same slice sizes.

### WB5 — Published-adjacent data
A slice derived from these files is uploaded with every run. Check the derived
JSONL for anything that should not be public before publishing a run that
references it.

## Workflows

### Rebuilding the slices
```bash
cd packages/browseros-agent
python3 apps/eval/scripts/build-webbench-sets.py
```
`random.seed(42)` is fixed, so the 50-sample slices are reproducible. Verify
line counts afterwards: `webbench-{0,1,2}of4-50.jsonl` should stay 50 lines.

### Tracing a task back to its source
Take the `query_id` or `metadata.original_task_id` from the JSONL line and grep
the CSVs here. `scripts/build-consolidated-set.ts` is the exception — those
tasks are hand-written in the script, not derived from these files.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the generated `data/*.jsonl` datasets.
- [`../../scripts/AGENTS.md`](../../scripts/AGENTS.md) — the builders that read these files.
- [`../../src/types/AGENTS.md`](../../src/types/AGENTS.md) — the schema the derived output satisfies.
- [`../../src/runner/AGENTS.md`](../../src/runner/AGENTS.md) — the JSONL loader.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
