# `src/` — Eval harness source

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/eval`; the harness implementation.

## What's here

The whole eval engine. `index.ts` is a `bun` entry that hands `Bun.argv` to
`cli/runCli`; the CLI resolves a suite or legacy config into an `EvalConfig` and
calls `runs/eval-runner.ts` `runEval`, which boots one BrowserOS stack
(Chrome + Bun server) per worker and drives every task through
`runs/task-run-pipeline.ts`. Each task writes a trajectory directory
(`metadata.json`, `messages.jsonl`, `grades.json`, `screenshots/`,
`grader-artifacts/`) and is then scored by the graders in `graders/`.
Nothing here talks to a mocked browser: the harness opens a real
BrowserOS.app binary over CDP and calls the real MCP endpoint.

```
src/
├── index.ts                 ← #!/usr/bin/env bun entry → runCli(Bun.argv.slice(2))
├── constants.ts             ← DEFAULT_TIMEOUT_MS 30m, SCREENSHOT_TIMEOUT_MS 65s,
│                              MAX_ACTIONS_PER_DELEGATION 15, CLADO_REQUEST_TIMEOUT_MS 6m
├── agents/                  ← AgentEvaluator implementations (single, orchestrator-executor, claude-code)
├── capture/                 ← screenshots, message log, error/warning buffer, artifact writes
├── cli/                     ← argv parsing + command dispatch (suite|run|grade|publish|legacy)
├── dashboard/               ← local Hono dashboard (127.0.0.1:9900) + index.html/viewer.html
├── graders/                 ← concrete Grader classes + python/*.py helpers
├── grading/                 ← Grader registry, runner, python bridge, artifact writers
├── publishing/              ← Cloudflare R2 (S3) upload of a finished run
├── reporting/               ← per-task/per-run metrics + score history summaries
├── runner/                  ← Chrome/server process + port management, task loading
├── runs/                    ← run orchestration: runEval, worker pool, per-task pipeline
├── suites/                  ← suite JSON schema, variant resolution, legacy-config adapter
├── types/                   ← zod schemas shared app-wide (config, task, message, result, errors)
├── utils/                   ← MCP client, provider config, timeout wrapper, env + token helpers
└── viewer/                  ← viewer-manifest.json builder consumed by the static viewer
```

## Rules

### EV1 — Task output layout is fixed
Every task writes into `<outputDir>/<query_id>/`. The canonical file names are
written in `capture/trajectory-saver.ts` (`metadata.json`, `attempt.json`,
`grades.json`, `messages.jsonl` via `capture/message-logger.ts`, `screenshots/`)
and mirrored in `capture/trajectory-saver.ts` → `hasExistingGraderResults`,
`cli/commands/grade.ts`, `dashboard/server.ts`, `publishing/r2-publisher.ts`,
`reporting/task-metrics.ts`. Adding or renaming one of these files means
touching every one of those readers.

### EV2 — `runs/` is the implementation, `runner/` is the alias layer
`runner/eval-runner.ts`, `runner/parallel-executor.ts` and
`runner/task-executor.ts` are one-line re-exports of `runs/eval-runner.ts`,
`runs/task-worker-pool.ts` and `runs/task-run-pipeline.ts` under the old
names. `runner/browseros-app-manager.ts`, `runner/task-loader.ts`,
`runner/infinity-app-manager.ts` and `runner/types.ts` are real code.
Observed convention: new orchestration goes in `runs/`; the `runner/` shims
are import-compat only. (Observed, not enforced by any check.)

### EV3 — Graders are registered in exactly one switch
`grading/grader-registry.ts` `createGrader(name)` is the only lookup, and
`PASS_FAIL_GRADER_ORDER` there fixes the precedence used for pass rate by
`runner/types.ts` `getPrimaryGraderResult` and by
`reporting/run-summary.ts` (which keeps a duplicated copy of the same list —
change both). A grader is a class with `name` + `grade(input)` implementing
`grading/types.ts` `Grader`.

### EV4 — Secrets never reach an artifact
Provider keys are either an ALL_CAPS env var name resolved by
`utils/resolve-env.ts` / `utils/resolve-provider-config.ts`, or read from
`process.env` at the point of use. `publishing/` writes everything under the
run dir to a public CDN, so never log or persist a resolved key.
`suites/resolve-variant.ts` deliberately splits `agent` from `publicMetadata`
(the latter only carries `apiKeyConfigured: boolean`) for this reason.

### EV5 — Per-worker isolation is arithmetic, not discovery
Worker N uses `base_cdp_port + N`, `base_server_port + N`,
`base_extension_port + N` (`runner/browseros-app-manager.ts`) and
`agents/single-agent.ts` recomputes the CDP port the same way. Change one and
you must change the other, or the agent will attach to another worker's Chrome.

### EV6 — Validate with zod at every boundary
`types/config.ts`, `types/task.ts`, `suites/schema.ts` are the schemas;
`runner/task-loader.ts`, `utils/config-validator.ts`, `suites/load-suite.ts`
and `suites/config-adapter.ts` are the parse sites. Don't hand-roll checks in
a consumer.

### EV7 — Two config formats coexist; new suites use the suite format
`configs/suites/*.json` matches `suites/schema.ts` (id/dataset/agent/graders/
workers/browseros). `configs/legacy/*.json` matches `types/config.ts`
`EvalConfigSchema` and is adapted by `suites/config-adapter.ts`. `cli/args.ts`
treats an unrecognised first token as the legacy `-c <config>` form.

## Workflows

### Adding a grader
1. Write the class in `graders/benchmark/` or `graders/performance/`
   implementing `grading/types.ts` `Grader`; write any Python helper into
   `graders/python/` and call it via `grading/python-evaluator.ts`.
2. Register it in `grading/grader-registry.ts` (case + `PASS_FAIL_GRADER_ORDER`).
3. Add the name to the `graders` array of the relevant `configs/suites/*.json`
   and to the matching `data/*.jsonl` task lines.
4. Dump debug JSON/text under `grader-artifacts/<grader>/` with
   `grading/artifacts.ts`.
5. Add tests under `tests/grading/`.

### Adding an agent type
1. Add the config schema in `types/config.ts` and the discriminator case in
   `agents/index.ts` `createAgent`.
2. Implement `AgentEvaluator.execute()` in `agents/<name>/index.ts`, wrapping
   the run in `utils/with-eval-timeout.ts` and writing `metadata` through
   `capture.trajectorySaver`.
3. Mirror it in `suites/schema.ts` `SuiteAgentSchema` and
   `cli/commands/suite.ts` `suiteToEvalConfig`.

### Running a suite end to end
1. `bun run eval suite --suite configs/suites/agisdk-daily-10.json` (from
   `apps/eval`, with `.env.development` copied from `.env.example`).
2. `bun run eval grade --run <outputDir>` re-scores an existing run.
3. `bun run eval publish --run <outputDir> --target r2` uploads it.

## Cross-references

- [`../../../AGENTS.md`](../../../AGENTS.md) — Bun monorepo parent (authoritative).
- [`../AGENTS.md`](../AGENTS.md) — `apps/eval` app overview (predates the current layout).
- [`../README.md`](../README.md) — operator-facing usage notes.
- [`../tests/AGENTS.md`](../tests/AGENTS.md) — test tree that mirrors this one.
- [`../../AGENTS.md`](../../AGENTS.md) — `apps/` sibling map.
