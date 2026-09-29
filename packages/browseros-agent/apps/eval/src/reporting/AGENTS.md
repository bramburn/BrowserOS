# `src/reporting/` — Metrics and score history

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; turns artifacts into numbers.

## What's here

`task-metrics.ts` reads a task's `messages.jsonl` and `metadata.json` and
produces per-task and per-run aggregates (duration, steps, screenshots, tool
calls, tool errors, per-tool counts, token usage, criteria pass/soften counts,
termination reason). `run-summary.ts` builds the score-history rows that
`scripts/weekly-report.ts` renders, deriving a config name from the run id.

```
reporting/
├── task-metrics.ts  ← normalizeToolName, countMessageMetrics, buildTaskMetrics,
│                      buildRunMetrics, readTaskMetrics, readRunMetricSummary
└── run-summary.ts   ← buildRunSummaries, extractConfigName, PASS_FAIL_GRADER_ORDER copy
```

## Rules

### RP1 — Metrics are derived from artifacts, never from live state
`readTaskMetrics` / `readRunMetricSummary` read what the run wrote. A
re-report must therefore be reproducible from the task dir alone, and must
work for runs produced by an older harness version.

### RP2 — Tool names are normalised before aggregation
`normalizeToolName` strips `mcp__browseros__` and, failing that, keeps only the
last `__`-separated segment of any `mcp__*` name, so the same logical tool
reported by the single-agent path and by the Claude Code path aggregates into
one bucket. New agent backends must go through this function.

### RP3 — Only the run-level averages are stored
`buildRunMetrics` computes totals plus averages (`avgSteps`, `avgToolCalls`,
…) and the per-tool stats, including `maxInputOutputTotal` for token usage.
Per-task numbers stay in the per-task summary; don't push per-task arrays into
the run metrics — the run manifest is uploaded publicly.

### RP4 — A second copy of the grader precedence lives here
`run-summary.ts` keeps its own `PASS_FAIL_GRADER_ORDER` with the comment that
report score must use the primary pass/fail grader. It is duplicated from
`../grading/grader-registry.ts`; **both must be updated together** or the CLI
history will disagree with the run summary.

### RP5 — Config name is parsed out of the run id
`extractConfigName` strips a trailing `-YYYY-MM-DD-HHMM`, matching the
timestamp format used by `../runs/eval-runner.ts` `formatTimestamp` and
`../runs/artifact-paths.ts`. Change the timestamp format and both must change.

### RP6 — Missing data degrades, it does not throw
A task with no `durationMs > 0` contributes nothing to `avgDurationMs`; a
manifest with no `tasks` yields zeros; `model`/`agentType` fall back to
`'unknown'`. Keep that behaviour — `scripts/generate-report.ts` runs over
partially-uploaded run sets.

## Workflows

### Regenerating a report from a local run
```bash
bun scripts/generate-report.ts <input-run-dir>   # → HTML report via claude-agent-sdk
```
It calls `readRunMetricSummary` for the numbers and
`claudeQuery` for the narrative. `scripts/weekly-report.ts` does the same across
every run in R2.

### Adding a metric
1. Add the field to `EvalTaskMetrics` in `task-metrics.ts` (and
   `EvalRunMetrics` if it aggregates).
2. Compute it in `buildTaskMetrics` / `buildRunMetrics`.
3. Surface it in `../viewer/viewer-manifest.ts` if the viewer should show it.
4. Add assertions in `tests/reporting/run-summary.test.ts` or
   `tests/reporting/generate-report-script.test.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness artifact rules.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — where `messages.jsonl` / `metadata.json` come from.
- [`../grading/AGENTS.md`](../grading/AGENTS.md) — the canonical grader precedence.
- [`../viewer/AGENTS.md`](../viewer/AGENTS.md) — metrics embedded in the viewer manifest.
- [`../../scripts/AGENTS.md`](../../scripts/AGENTS.md) — report generators.
- [`../../../tests/reporting/AGENTS.md`](../../tests/reporting/AGENTS.md) — report tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
