# `src/cli/commands/` — Command implementations

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/cli/`.

## What's here

Four small modules, one per CLI verb. `suite.ts` is the substantial one: it
turns either a legacy config path or a suite path into an `EvalConfig` and runs
it, optionally publishing afterwards. `run.ts` is `suite` without the publish
step. `grade.ts` re-scores existing task artifacts. `publish.ts` uploads a run
to R2. Every command takes an optional `deps` object so tests can substitute
the runner and the publisher.

```
commands/
├── suite.ts   ← resolveSuiteCommand, runSuiteCommand, suiteToEvalConfig, SuiteCommandDeps
├── run.ts     ← runRunCommand
├── grade.ts   ← runGradeCommand (find task dirs, re-run graders, write back)
└── publish.ts ← publishRun, runPublishCommand
```

## Rules

### CMD1 — Dependency injection is the test seam
`SuiteCommandDeps` accepts `runEval` and `publishRun`;
`CreateClaudeCodeProcessRunnerDeps` does the same for spawning. `runSuiteCommand`
throws `publish requested before the publisher is configured` when
`--publish` is passed without `deps.publishRun` — a deliberate guard, not an
oversight.

### CMD2 — Suite → EvalConfig mapping lives in one function
`suiteToEvalConfig` is the only place that turns the suite vocabulary into the
runner vocabulary: `single`/`tool-loop` → `type: 'single'`,
`orchestrated`/`orchestrator-executor` → `type: 'orchestrator-executor'` with an
`executor` block, `claude-code` → `type: 'claude-code'`. It calls
`ensureRunnableSuite` first, so a suite without a `browseros` block fails with
`suite browseros config is required to run suite commands` before any browser
work.

### CMD3 — Clado executor settings come from env, not the variant
For `executorBackend: 'clado'`, the executor model/key/baseUrl are read from
`EVAL_EXECUTOR_*` with `CLADO_ACTION_*` fallbacks. The variant only
configures the orchestrator.

### CMD4 — `grade` reuses the grader names already recorded
`runGradeCommand` reads `Object.keys(metadata.grader_results)` and skips a task
with no graders (warning, not failure). It re-derives the MCP URL from
`BROWSEROS_SERVER_URL` (default `http://127.0.0.1:9110`), so a re-grade that
needs a live browser requires a server on that port.

### CMD5 — Task discovery is "directory containing metadata.json"
Both `grade.ts` and `publishing/r2-publisher.ts` enumerate subdirectories of a
run dir and treat a directory as a task only when `metadata.json` exists.
Keep that predicate identical in both.

## Workflows

### Re-grading a run after a grader change
1. Change the grader (and bump `PASS_FAIL_GRADER_ORDER` if its precedence
   changed).
2. `bun run eval grade --run <outputDir>` — no browser is launched unless the
   grader itself needs one (agisdk and infinity both do).
3. Inspect `grades.json` + `metadata.json` per task, then
   `bun run eval publish --run <outputDir> --target r2` to refresh the viewer.

### Debugging a suite that refuses to start
`resolveSuiteCommand` throws in this order: neither/both path flags →
`loadSuite` zod failure (message names the offending field) →
`resolveVariant` (`EVAL_AGENT_MODEL is required`) → `suite browseros config is
required`. Read the first error only; each masks the next.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — CLI surface and arg rules.
- [`../../suites/AGENTS.md`](../../suites/AGENTS.md) — suite schema and variant resolution.
- [`../../publishing/AGENTS.md`](../../publishing/AGENTS.md) — what `publish` calls.
- [`../../../tests/cli/AGENTS.md`](../../../tests/cli/AGENTS.md) — `suite-command.test.ts`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
