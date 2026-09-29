# `tests/` — Harness test suite

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/eval`; mirrors `src/`.

## What's here

`bun test` files that mirror the `src/` tree one-for-one, plus `e2e/` for
scripts that are not part of the fast suite. They are pure unit/integration
tests over pure logic: config parsing, CLI arg handling, suite adaptation,
grader registry and artifact layout, viewer manifest, R2 publishing (with a
fake client), metrics aggregation, and the pure helpers inside the Clado and
Claude Code agents. No test here launches a real BrowserOS stack — the
integration surface is covered by running the harness itself.

```
tests/
├── agents/       ← clado actions + driver, executor backend, claude-code evaluator/parser/runner
├── capture/      ← captcha-waiter
├── cli/          ← args, suite-command
├── dashboard/    ← shouldAutoOpenDashboard
├── e2e/          ← captcha-e2e.ts (not a *.test.ts; run manually)
├── grading/      ← grader registry, agisdk/infinity/performance artifacts, python evaluator + script layout
├── publishing/   ← r2-publisher, r2-viewer-compat
├── reporting/    ← run-summary, generate-report-script
├── runs/         ← artifact-paths, run-manifest, pipeline-compat
├── suites/       ← schema, config-adapter
├── utils/        ← resolve-provider-config
└── viewer/       ← viewer-manifest
```

## Rules

### T1 — Tests mirror `src/`, one directory per module
`tests/<area>/<module>.test.ts` for `src/<area>/<module>.ts`. A new source file
with real logic is expected to come with a test beside its mirror.

### T2 — `.test.ts` only; everything else is manual
`e2e/captcha-e2e.ts` is a runnable script, not a test. `bun test` will not
collect it, which is intentional: it needs a live server and a solved captcha.

### T3 — Dependency injection beats mocks of modules
The suite prefers the seams the code already exposes —
`SuiteCommandDeps.runEval/publishRun`,
`CreateClaudeCodeProcessRunnerDeps.spawn`, `R2PublisherOptions.client`, and
`resolveVariant({ env })` / `adaptEvalConfigFile({ env })`. When a test needs a
new seam, add it to the production type rather than stubbing a module.

### T4 — Env is injected, never mutated
Env-dependent helpers take an optional `env: Record<string, string|undefined>`
(`resolveVariant`, `shouldAutoOpenDashboard`, `loadR2ConfigFromEnv`). Pass a
literal object in tests instead of touching `process.env`.

### T5 — Artifacts are asserted by filename
The grading/publishing tests assert on the exact artifact names
(`evaluator-input.json`, `evaluator-output.json`, `finish-state.json`, …)
because those names are a contract with the viewer. Keep them in sync with
`src/viewer/viewer-manifest.ts`.

## Workflows

### Running the suite
```bash
# from packages/browseros-agent
bun run test:eval            # if the root script exists
# or from apps/eval
bun test ./tests
bun test ./tests/suites      # one area
```
`package.json` defines `test` as
`bun run ../../scripts/run-bun-test.ts ./apps/eval/tests` — use that so the
shared runner and env are applied.

### Adding a test for a new grader
1. Follow the shape of `tests/grading/agisdk-artifacts.test.ts`: a temp task dir
   (`taskArtifactDir`), a synthetic `GraderInput`, then assert on the files
   written under `grader-artifacts/<grader>/`.
2. If the grader shells out to Python, add the script-layout assertion in
   `tests/grading/python-script-layout.test.ts` instead of executing it.

## Cross-references

- [`../src/AGENTS.md`](../src/AGENTS.md) — the code under test.
- [`../AGENTS.md`](../AGENTS.md) — `apps/eval` overview.
- [`../package.json`](../package.json) — the `test` / `typecheck` scripts.
- [`e2e/AGENTS.md`](e2e/AGENTS.md) — the manual script.
- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent.
