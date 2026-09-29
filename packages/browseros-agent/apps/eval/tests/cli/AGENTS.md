# `tests/cli/` — CLI tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

Two tests guarding the CLI contract: `args.test.ts` pins the argv → `EvalCliArgs`
mapping (including the implicit legacy fallback and the mutually-exclusive
`--config` / `--suite` guard), and `suite-command.test.ts` drives
`resolveSuiteCommand` / `runSuiteCommand` with an injected `runEval` and
`publishRun` so the whole command can be exercised without a browser.

```
tests/cli/
├── args.test.ts           ← parseEvalCliArgs for suite|run|grade|publish|legacy
└── suite-command.test.ts  ← resolveSuiteCommand, runSuiteCommand (+ publish guard)
```

## Rules

### TCL1 — `args.ts` must stay side-effect free
These tests import `parseEvalCliArgs` directly. If parsing ever touches the
filesystem, the network, or the dashboard, these tests become integration tests.

### TCL2 — Command behaviour is tested through `deps`
`SuiteCommandDeps` supplies `runEval` (returns `{ outputDir, summary }`) and
`publishRun`. Assert that `--publish` is only forwarded when the target is set,
and that a missing `deps.publishRun` produces
`publish requested before the publisher is configured`.

### TCL3 — Error strings are assertions
The tests pin messages such as `suite accepts either --config or --suite, not
both`, `grade requires --run`, `publish requires --target` and
`Unsupported publish target: …`. These are user-facing text emitted by
`src/index.ts`; changing them is a UX change, not a refactor.

## Workflows

### Adding a CLI flag
1. Add it to `parseSuiteLikeArgs` (or the relevant `parseArgs` block) in
   `src/cli/args.ts` and to the args interface.
2. Add a case to `args.test.ts` for the flag present, absent, and empty.
3. If it changes what the command does, extend `suite-command.test.ts`.

### Verifying a config maps to the right backend
`suite-command.test.ts` is where a suite's `agent.type` /
`executorBackend` is checked against the `EvalConfig` handed to the runner. A
mis-mapping shows up here as a wrong `agent.type` or a missing `executor`
block, long before a browser is launched.

## Cross-references

- [`../../src/cli/AGENTS.md`](../../src/cli/AGENTS.md) — the folder under test.
- [`../../src/cli/commands/AGENTS.md`](../../src/cli/commands/AGENTS.md) — command implementations.
- [`../../src/suites/AGENTS.md`](../../src/suites/AGENTS.md) — the suite model being resolved.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
