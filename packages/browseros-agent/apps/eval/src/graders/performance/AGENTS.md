# `src/graders/performance/` — LLM-judge grader

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/graders/`; scores *how* the agent worked.

## What's here

`PerformanceGrader` (name `performance_grader`) does not check the app's state.
It reads the recorded trajectory, computes cheap metrics, and asks a Claude
Agent SDK sub-agent to score six weighted axes. `axes.ts` holds the axis
definitions and the system prompt (the rubric: what counts as a necessary
action, what is cosmetic, how to verify claims against the DOM).
`metadata-extractor.ts` turns `messages.jsonl` + `metadata.json` into the
`PreComputedMetrics` the judge reads. `types.ts` has the axis/response types and
`PERFORMANCE_EVAL_SCHEMA`.

```
performance/
├── performance-grader.ts  ← PerformanceGrader: prompt → SDK query → weighted score
├── axes.ts                ← DEFAULT_AXES, PERFORMANCE_SYSTEM_PROMPT, buildUserPrompt
├── metadata-extractor.ts  ← extractMetrics
└── types.ts               ← AxisDefinition, AxisScore, PreComputedMetrics, options
```

## Rules

### PF1 — Weights and threshold are the score contract
`DEFAULT_AXES`: `task_completion` 0.3, `reasoning_quality` 0.2,
`efficiency` 0.2, `speed` 0.1, `error_recovery` 0.1, `autonomy` 0.1 (sums to
1.0). `DEFAULT_PASS_THRESHOLD = 75` decides `pass`; the returned `score` is the
weighted 0–100 total. Changing a weight changes every historical number —
record it in the run's `axes.json`.

### PF2 — All axes or nothing
`parseResponse` rejects a response that does not carry every expected axis, and
the grader additionally diffs expected vs returned axis names and fails with a
"missing axes" reasoning string. A partial judgement is never silently averaged.

### PF3 — Defaults are explicit and overridable
`DEFAULT_MODEL = 'claude-opus-4-5-20251101'`, `DEFAULT_MAX_TURNS = 100`,
`DEFAULT_MAX_BUDGET_USD = 100`, `GRADER_TIMEOUT_MS = 300_000`. The
constructor takes `PerformanceGraderOptions`; the registry instantiates it with
no arguments, so any change here changes every run.

### PF4 — The judge is a tool-using agent with a read-only budget
`runAgent` streams the SDK `query()` and only accepts the `result` message's
`structured_output`; it logs which files the judge read. The judge is pointed at
the task artifact dir, so keep that dir's layout stable (see
`../../capture/AGENTS.md`).

### PF5 — Metrics come from the recorded run, not from the live browser
`extractMetrics` reads `metadata.json` (termination reason, duration) and the
message stream. A missing `metadata.json` is tolerated — the grader notes it and
continues with the stream-derived numbers.

### PF6 — Artifacts are `metrics.json` and `axes.json`
Both are written through `../../grading/artifacts.ts` under
`grader-artifacts/performance_grader/`. `axes.json` is what the run report
aggregates into score history.

## Workflows

### Tuning the rubric
1. Edit `DEFAULT_AXES` weights or `PERFORMANCE_SYSTEM_PROMPT` in `axes.ts`.
2. Re-grade an existing run: `bun run eval grade --run <outputDir>`.
3. Compare `axes.json` before/after on the same trajectory — the judge is
   stochastic, so compare distributions, not single tasks.

### Using it as the primary grader
`performance_grader` is the fallback in `PASS_FAIL_GRADER_ORDER` and is the
default for single-query smoke runs (`runner/task-loader.ts` fabricates
`graders: ['performance_grader']`). It is the only grader that does not need a
live browser.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — grader registry and precedence.
- [`../grading/AGENTS.md`](../../grading/AGENTS.md) — artifact writers used here.
- [`../../capture/AGENTS.md`](../../capture/AGENTS.md) — the trajectory this grader reads.
- [`../../reporting/AGENTS.md`](../../reporting/AGENTS.md) — how `score` becomes run history.
- [`../../../../tests/grading/AGENTS.md`](../../../tests/grading/AGENTS.md) — `performance-artifacts.test.ts`.
- [`../../../../../AGENTS.md`](../../../../../AGENTS.md) — Bun monorepo parent.
