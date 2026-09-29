# `scripts/` — Dataset builders and report generators

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/eval`; operator tooling outside the CLI.

## What's here

Standalone runnable scripts, not part of the `bun run eval` command surface.
Ten scripts: five build `data/*.jsonl` from upstream sources, one converts
legacy benchmark formats, three publish/report, and one smoke-tests the Clado
endpoint. They import from `../src/` where useful (`upload-run.ts` →
`../src/publishing`, `generate-report.ts` → `../src/reporting`), so they are Bun
scripts; the dataset builders are Python 3.

```
scripts/
├── build-agisdk-dataset.py      ← agisdk package → data/agisdk-real.jsonl
├── build-infinity-dataset.py    ← WebArena-Infinity real-tasks.json → data/webarena-infinity-*.jsonl
├── build-webbench-sets.py       ← data/webbench/*.csv → data/webbench-*.jsonl + browsecomp-*.jsonl
├── build-browsecomp-sets.py     ← BrowseComp CSV → the two browsecomp-*.jsonl
├── build-consolidated-set.ts    ← hand-written tasks → data/consolidated-eval-set.jsonl
├── converter.py                 ← WebVoyager / Mind2Web → unified JSONL
├── upload-run.ts                ← R2Publisher wrapper (single run or all runs of a config)
├── generate-report.ts           ← per-run HTML report via claude-agent-sdk
├── weekly-report.ts             ← all R2 runs → cumulative HTML dashboard
└── test-clado-api.ts            ← Clado endpoint smoke test
```

## Rules

### SC1 — Builders write `data/`, they never run the eval
Every `build-*` script emits JSONL and exits. The path is either hard-coded
relative to the repo (`apps/eval/data`) or passed as an argument. Keep them
runnable from `packages/browseros-agent` without arguments where they already
are — that is how the CSVs and committed slices stay reproducible.

### SC2 — Seeding is part of the output
`build-webbench-sets.py` and `build-browsecomp-sets.py` call `random.seed(42)`.
Removing it silently changes every 50-sample slice. If a slice must change, do
it deliberately and update `../data/AGENTS.md`.

### SC3 — Python output must satisfy `TaskSchema`
The builders are the reason `data/*.jsonl` loads at all. Any field rename there
breaks `src/runner/task-loader.ts` for every task in the file, so validate by
running a suite against the output before committing it.

### SC4 — Each report script has its own credential set
`upload-run.ts` and `weekly-report.ts` need R2 credentials (`EVAL_R2_ACCOUNT_ID`,
`EVAL_R2_ACCESS_KEY_ID`, `EVAL_R2_SECRET_ACCESS_KEY`, optionally `EVAL_R2_BUCKET` /
`EVAL_R2_CDN_BASE_URL`); `upload-run.ts` gets them indirectly through
`loadR2ConfigFromEnv()` in `../src/publishing/r2-publisher.ts`. `generate-report.ts`
needs no R2 access at all — it reads a local run dir and drives the Claude Agent
SDK with `CLAUDE_CODE_OAUTH_TOKEN` or `ANTHROPIC_API_KEY`. Conversely
`weekly-report.ts` reads no Anthropic credential. See `../.env.example`. Dataset
builders need only Python 3 — plus, for agisdk, the `agisdk` package installed.

### SC5 — `test-clado-api.ts` is a diagnostic, not a gate
It health-checks the Clado endpoint, optionally captures a screenshot over MCP
from a running server (`BROWSEROS_URL`, default `http://127.0.0.1:9110`), and
prints every field of the action contract. Cold start can take ~5 minutes, so it
waits 6. It is the fastest way to tell a model problem from a wiring problem in
`../src/agents/orchestrated/backends/clado/`.

### SC6 — Paths in scripts are repo-root relative
Several builders use `OUT_DIR = "apps/eval/data"` and are documented to be run
from `packages/browseros-agent`. Keep that convention; a relative path change
silently writes to a different directory.

## Workflows

### Rebuilding every dataset slice
```bash
cd packages/browseros-agent
python3 apps/eval/scripts/build-webbench-sets.py
python3 apps/eval/scripts/build-browsecomp-sets.py
python3 apps/eval/scripts/build-agisdk-dataset.py > apps/eval/data/agisdk-real.jsonl
python3 apps/eval/scripts/build-infinity-dataset.py --apps-dir <webarena-infinity>/apps
bun    apps/eval/scripts/build-consolidated-set.ts
```

### Publishing a run and reporting on it
```bash
bun apps/eval/scripts/upload-run.ts results/<config>/<timestamp>   # one run
bun apps/eval/scripts/upload-run.ts results/<config>                # all runs of a config
bun apps/eval/scripts/generate-report.ts <input-run-dir>           # HTML report
bun apps/eval/scripts/weekly-report.ts [output-path]               # history dashboard
```

### Debugging "the run produced no grades"
1. `metadata.json` in the task dir — `grader_results` empty means the grader
   name did not resolve (`Unknown grader: <name>` is logged by
   `src/grading/grader-registry.ts`).
2. The dataset line's `graders` array is what runs — check it, not the config.
3. For `infinity_state`, check `WEBARENA_INFINITY_DIR` and that the app server
   was up at grading time.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/eval` overview.
- [`../data/AGENTS.md`](../data/AGENTS.md) — the files these builders write.
- [`../src/AGENTS.md`](../src/AGENTS.md) — the modules the Bun scripts import.
- [`../.env.example`](../.env.example) — required environment variables.
- [`../tests/AGENTS.md`](../tests/AGENTS.md) — where `generate-report.ts` is covered.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
