# `tests/reporting/` — Reporting tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

`run-summary.test.ts` covers `buildRunSummaries` / `extractConfigName` in
`src/reporting/run-summary.ts`: config-name extraction from a timestamped run
id, pass rate from the primary grader, status counts, and the
average-duration filter. `generate-report-script.test.ts` covers the
`scripts/generate-report.ts` entry point (its input discovery and output
naming) without invoking the Claude Agent SDK.

```
tests/reporting/
├── run-summary.test.ts           ← score history rows from run manifests
└── generate-report-script.test.ts ← report script input/output behaviour
```

## Rules

### TR1 — Score precedence is asserted, and duplicated on purpose
`run-summary.ts` keeps its own `PASS_FAIL_GRADER_ORDER` copy (mirroring
`src/grading/grader-registry.ts`). `run-summary.test.ts` should fail if the
precedence used to compute `avgScore` changes silently — that is how a
mixed-grader run keeps matching the eval summary.

### TR2 — Fixture manifests, not real runs
The tests construct `ReportManifest` objects inline. They must keep parsing old
run shapes: `schemaVersion` is optional, `uploadedAt`/`agentConfig`/`dataset`
are optional, and an empty `tasks` array yields zeros rather than throwing
(`scripts/weekly-report.ts` runs over partially published sets).

### TR3 — `generate-report-script.test.ts` must not call an LLM
The script wraps `claudeQuery`. The test covers only the deterministic parts
(input-dir handling, output path, metric summary assembly). If you add a
network call to the script, keep it behind the same boundary.

## Workflows

### Changing the score shown in the weekly report
1. Edit `buildRunSummaries` / the precedence array in
   `src/reporting/run-summary.ts`.
2. Update `run-summary.test.ts` expectations.
3. Check `src/grading/grader-registry.ts` still agrees on precedence (rule RP4
   in `src/reporting/AGENTS.md`).

### Regenerating reports
```bash
bun scripts/generate-report.ts <input-run-dir>   # single run → HTML
bun scripts/weekly-report.ts [output-path]       # all runs in R2 → HTML dashboard
```
Both read the same metrics; the first is per-run, the second is the cumulative
history view.

## Cross-references

- [`../../src/reporting/AGENTS.md`](../../src/reporting/AGENTS.md) — the folder under test.
- [`../../src/grading/AGENTS.md`](../../src/grading/AGENTS.md) — canonical grader precedence.
- [`../../scripts/AGENTS.md`](../../scripts/AGENTS.md) — the report generators.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
