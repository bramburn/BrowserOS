# `src/graders/` — Grader implementations

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the code that decides pass/fail.

## What's here

The `Grader` interface itself lives one level up in
`../grading/types.ts`; this folder holds the implementations. `benchmark/`
contains the two state-diff graders that shell out to Python;
`performance/` contains the LLM-judge grader; `python/` holds the two helper
scripts those graders execute. `registry.ts` and `types.ts` at this level are
re-exports of `../grading/grader-registry` and `../grading/grader-runner` so
older import paths keep working.

```
graders/
├── registry.ts                       ← re-export of grading/grader-registry + grader-runner
├── types.ts                          ← re-export of Grader / GraderInput
├── benchmark/
│   ├── agisdk-state-diff.ts          ← AgisdkStateDiffGrader ('agisdk_state_diff')
│   └── infinity-state.ts             ← InfinityStateGrader ('infinity_state')
├── performance/
│   ├── performance-grader.ts         ← PerformanceGrader ('performance_grader')
│   ├── axes.ts                       ← DEFAULT_AXES + PERFORMANCE_SYSTEM_PROMPT
│   ├── metadata-extractor.ts         ← extractMetrics(messages, metadata)
│   └── types.ts                      ← AxisDefinition, PERFORMANCE_EVAL_SCHEMA, …
└── python/
    ├── agisdk-evaluate.py            ← WebCloneEvaluator wrapper (stdin JSON → stdout JSON)
    └── infinity-evaluate.py          ← loads a verifier module and calls verify(url)
```

## Rules

### GR1 — Three graders exist; the registry is exhaustive
`agisdk_state_diff`, `infinity_state`, `performance_grader`. `createGrader`
warns `Unknown grader: <name>` and returns `null` for anything else, and
`runConfiguredGraders` skips null graders silently. A misspelled grader name in
a config produces a task with no grades — not an error.

### GR2 — Benchmark graders navigate the real browser
`AgisdkStateDiffGrader` drives the page itself: it derives the site from the
task id (`dashdish-10` → `https://evals-dashdish.vercel.app`), calls
`navigate_page` to `<origin>/finish`, then polls `evaluate_script` for a
`<pre>` block that starts with `{` (20 attempts) and hands that state diff to
Python. It needs `input.mcpUrl`.

### GR3 — Query ids carry the dataset meaning
`agisdk-<site>-<n>` → strip the `agisdk-` prefix; `infinity-<app>-task_<id>` →
regex `^infinity-(.+)-(task_.+)$`. A query id that does not match yields
`score: 0, pass: false` with a reasoning string — it does not throw.

### GR4 — Every grader dumps its inputs and outputs
Use `grading/artifacts.ts` `writeGraderJsonArtifact` /
`writeGraderTextArtifact`, which write to
`grader-artifacts/<grader-name>/` under the task dir. The names the viewer
expects are `context.json`, `evaluator-input.json`, `evaluator-output.json`,
`verifier.json`, `metrics.json`, `axes.json`, `finish-state.json`. The viewer
manifest hard-codes the last one.

### GR5 — Python scripts speak JSON on stdin/stdout, always
`runPythonJsonEvaluator` (in `../grading/python-evaluator.ts`) spawns
`python3`/`python` with the script path, writes one JSON object to stdin, and
parses one JSON object from stdout; a non-zero exit throws with stdout/stderr
attached. A Python helper that logs to stdout corrupts the result — the agisdk
script explicitly redirects `sys.stdout` to stderr while evaluating.

### GR6 — `performance_grader` is a weighted LLM judge
`DEFAULT_AXES` weights are `task_completion` 0.3, `reasoning_quality` 0.2,
`efficiency` 0.2, `speed` 0.1, `error_recovery` 0.1, `autonomy` 0.1.
`DEFAULT_PASS_THRESHOLD = 75`, `DEFAULT_MAX_TURNS = 100`,
`DEFAULT_MAX_BUDGET_USD = 100`, `GRADER_TIMEOUT_MS = 300_000`, default model
`claude-opus-4-5-20251101`. A response missing any expected axis is rejected
and scored 0 — the judge must answer for every axis.

### GR7 — A grader must never crash the run
Every `grade()` wraps its body in try/catch and returns
`{ score: 0, pass: false, reasoning: '...' }` on failure. `runConfiguredGraders`
also catches per grader so one broken grader doesn't hide the others.

## Workflows

### Adding a benchmark grader for a new benchmark
1. Implement `Grader` in `benchmark/`, deriving everything it needs from
   `input.task.query_id` / `input.task.dataset` (and `mcpUrl` /
   `infinityAppUrl` where a live server is required).
2. Add its Python helper under `python/` and call it through
   `runPythonJsonEvaluator`.
3. Register it in `../grading/grader-registry.ts` **and** add it to
   `PASS_FAIL_GRADER_ORDER` in both that file and `../reporting/run-summary.ts`.
4. Add it to the suite's `graders` array and to the task lines in `data/`.

### Debugging a bad grade
1. Open `grader-artifacts/<grader>/` in the task dir — inputs and outputs are
   both there.
2. `grades.json` in the same dir is the record the runner used.
3. Re-score without re-running: `bun run eval grade --run <outputDir>`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness map and the single-registry rule.
- [`../grading/AGENTS.md`](../grading/AGENTS.md) — registry, runner, python bridge, artifact writers.
- [`benchmark/AGENTS.md`](benchmark/AGENTS.md) — the two state-diff graders.
- [`performance/AGENTS.md`](performance/AGENTS.md) — the LLM judge and its axes.
- [`python/AGENTS.md`](python/AGENTS.md) — the two Python helpers.
- [`../../../tests/grading/AGENTS.md`](../../tests/grading/AGENTS.md) — grader tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
