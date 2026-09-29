# `src/grading/` — Grader plumbing

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the contract and execution layer for graders.

## What's here

Not the graders themselves (those are in `../graders/`) but everything that
hosts them: the `Grader`/`GraderInput` types, the name → class registry, the
loop that runs a configured set of graders, the bridge to Python, and the
artifact writers. `runs/task-run-pipeline.ts` imports `runGraders` from
`../graders/registry`, which re-exports this folder.

```
grading/
├── types.ts            ← Grader, GraderInput
├── grader-registry.ts  ← createGrader(name), PASS_FAIL_GRADER_ORDER
├── grader-runner.ts    ← runConfiguredGraders / runGraders
├── python-evaluator.ts ← runPythonJsonEvaluator (spawn python, JSON in/out, timeout)
└── artifacts.ts        ← writeGraderJsonArtifact / writeGraderTextArtifact
```

## Rules

### GD1 — `PASS_FAIL_GRADER_ORDER` is the precedence contract
`['agisdk_state_diff', 'infinity_state', 'performance_grader']`. The first
grader present in a task's results decides pass rate, via
`getPrimaryGraderResult` in `../runner/types.ts`; if none match, the first entry
of the object wins. `../reporting/run-summary.ts` keeps a **duplicate copy** of
this array — a known duplication, not an accident; change both or the CLI
summary will disagree with the run summary.

### GD2 — Graders run independently and never abort each other
`runConfiguredGraders` loops over names, wraps each `grade()` in try/catch, and
records `{ score: 0, pass: false, reasoning: 'Error running grader: …' }` on
throw. A `null` from `createGrader` is skipped without a record. Graders run
sequentially (`await` in a `for` loop), not in parallel.

### GD3 — `GraderInput` carries everything a grader may need
`task` (`query_id`, `query`, `dataset`), `messages`, `screenshotCount`,
`finalAnswer`, optional `expectedAnswer`, `taskArtifactDir`, `outputDir`, and
optional `mcpUrl` / `infinityAppUrl`. Adding a capability (say, the task's
`start_url`) means extending this type and every construction site — the
pipeline and `cli/commands/grade.ts` both build it by hand.

### GD4 — Artifact paths are derived, never passed in
`artifactDir()` is always
`join(input.taskArtifactDir || input.outputDir, 'grader-artifacts', graderName)`.
The viewer manifest hard-codes the same shape, including
`grader-artifacts/agisdk_state_diff/finish-state.json`.

### GD5 — Python invocation is bounded
`runPythonJsonEvaluator` races the process against a `timeoutMs` rejection and
kills the child in `finally`. Its `pythonPath` is resolved from env/availability
inside the function, so the TypeScript side never hardcodes an interpreter.

## Workflows

### Re-scoring with a modified grader
1. Edit the grader in `../graders/`.
2. Leave `createGrader` alone unless the **name** changed.
3. `bun run eval grade --run <outputDir>` — the runner reuses the grader names
   already recorded in `metadata.json`.

### Renaming a grader (breaking)
Rename in the class, in `createGrader`, in `PASS_FAIL_GRADER_ORDER` (both
copies), in every `data/*.jsonl` `graders` array, and in every
`configs/**.json` `graders` array. Existing artifacts keep the old key, so
`grade` will report "no existing grader names" for old runs.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness-wide artifact and registry rules.
- [`../graders/AGENTS.md`](../graders/AGENTS.md) — the grader implementations.
- [`../runner/AGENTS.md`](../runner/AGENTS.md) — `getPrimaryGraderResult`.
- [`../reporting/AGENTS.md`](../reporting/AGENTS.md) — the duplicate precedence list.
- [`../../tests/grading/AGENTS.md`](../../tests/grading/AGENTS.md) — registry and artifact tests.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
