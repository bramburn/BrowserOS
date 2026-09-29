# `data/` — Task datasets (JSONL)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/eval`; the task inputs the harness executes.

## What's here

One JSONL file per benchmark slice. Each line is a single task matching
`TaskSchema` in `src/types/task.ts`: `query_id`, `dataset`, `query`, optional
`graders` / `start_url` / `setup_script`, and a `metadata` object
(`original_task_id`, optional `website` / `category`, and an `additional` bag
that carries benchmark-specific fields). `webbench/` holds the raw CSV sources
these slices are generated from. `data/raw/` is git-ignored (upstream downloads).

```
data/
├── agisdk-*.jsonl                  ← agisdk-real, agisdk-real-smoke, agisdk-daily-10
├── webvoyager.jsonl                ← + webvoyager_e2e_test.jsonl
├── mind2web.jsonl, mind2web_e2e_test.jsonl
├── webbench-*.jsonl                ← 0of4/1of4/2of4 full and 50-sample slices
├── browsecomp-medium-hard-50.jsonl, browsecomp-very-hard-50.jsonl
├── consolidated-eval-set.jsonl     ← hand-written multi-step tasks
├── test-set.jsonl                  ← 1-line smoke set
├── webarena-infinity-hard-50.jsonl ← WebArena-Infinity tasks
└── webbench/*.csv                  ← raw benchmark exports (9 files, ~10 MB)
```

## Rules

### DT1 — One JSON object per line, and a bad line fails the whole file
`src/runner/task-loader.ts` parses every line through `TaskSchema` and throws
`TaskLoadError` listing up to five failing lines if *any* line is invalid. It
also rejects duplicate `query_id`s. Validate a new file before committing:
`bun run eval run --suite …` will refuse to start on a malformed line.

### DT2 — `query_id` is a path and a grader input
It becomes the task output directory name and is parsed by graders
(`agisdk-<site>-<n>`, `infinity-<app>-task_<id>`). Keep it lowercase, stable,
and free of characters that need escaping.

### DT3 — Per-task `graders` wins over the config's `graders`
`runs/task-run-pipeline.ts` runs the graders named on the line. A suite's
`graders` array is the default for the run; changing it does not re-grade tasks
that already name their own graders.

### DT4 — Benchmark-specific data goes in `metadata.additional`
`src/utils/dataset-metadata.ts` lifts `website`, `difficulty`,
`challenge_type` and `similar_to` out of `additional` into the viewer.
Infinity tasks additionally carry `app_name`, `verifier_path` and
`app_base_port`, which the app manager and grader read directly. Adding a field
here means nothing consumes it until a grader or the viewer asks for it.

### DT5 — Regenerate, don't hand-edit, the generated slices
`webbench-*.jsonl`, `browsecomp-*.jsonl` and the agisdk/infinity files are
produced by `../scripts/build-*.py` from the CSVs in `webbench/` or from an
upstream package. Hand edits are lost on the next build. Hand-written content
lives in `consolidated-eval-set.jsonl`, produced by
`../scripts/build-consolidated-set.ts`.

### DT6 — These files are public once published
Datasets are uploaded with the run (as `dataset` metadata and grader artifacts).
Do not add credentials, internal hostnames, or customer data to a task.

## Workflows

### Adding a small hand-written set
1. Write one JSON object per line following `TaskSchema`.
2. Include `metadata.original_task_id` and a `category`.
3. Point a suite at it (`"dataset": "../../data/<name>.jsonl"`) — see
   [`../configs/suites/AGENTS.md`](../configs/suites/AGENTS.md).
4. Run `bun run eval run --suite …` and confirm the task count in the log.

### Regenerating the WebBench slices
```bash
cd packages/browseros-agent
python3 apps/eval/scripts/build-webbench-sets.py
```
It reads `apps/eval/data/webbench/*.csv` (seeded, so output is stable) and
rewrites the `webbench-*.jsonl` and `browsecomp-*.jsonl` files.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/eval` overview.
- [`../src/types/AGENTS.md`](../src/types/AGENTS.md) — the `TaskSchema` these lines satisfy.
- [`../src/runner/AGENTS.md`](../src/runner/AGENTS.md) — the loader and its all-or-nothing rule.
- [`webbench/AGENTS.md`](webbench/AGENTS.md) — the raw CSV sources.
- [`../scripts/AGENTS.md`](../scripts/AGENTS.md) — the dataset builders.
- [`../configs/AGENTS.md`](../configs/AGENTS.md) — the configs that reference these files.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
