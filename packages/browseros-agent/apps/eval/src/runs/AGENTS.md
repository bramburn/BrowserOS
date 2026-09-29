# `src/runs/` — Run orchestration

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; the top of the execution stack.

## What's here

`eval-runner.ts` is the entry point every CLI path funnels into: validate the
config, resolve dataset/output paths, load tasks, start the dashboard, run the
pool, summarise. `task-worker-pool.ts` owns N isolated BrowserOS stacks and the
task queue. `task-run-pipeline.ts` runs one task end to end
(navigate → agent → graders → cleanup). `artifact-paths.ts` and
`run-manifest.ts` describe the newer, suite-era `runs/<runId>/` layout used by
the publisher and viewer.

```
runs/
├── eval-runner.ts       ← runEval(options): validate → load → dashboard → pool → summary
├── task-worker-pool.ts  ← TaskWorkerPool: N workers, TaskQueue, captcha key patch
├── task-run-pipeline.ts ← TaskRunPipeline.execute(task), TaskExecutionError
├── artifact-paths.ts    ← createRunId, getRunPaths (runs/<id>/tasks/<taskId>/…)
└── run-manifest.ts      ← buildRunManifest (reproducibility record)
```

## Rules

### RS1 — `runEval` is the only public entry
Everything else takes a resolved `EvalConfig`. If you need a new capability,
add an option to `RunEvalOptions` (`../runner/types.ts`) rather than a second
entry point.

### RS2 — Output dir is `results/<config>/<UTC timestamp>/` by default
`resolvePaths` derives the config name from the config filename and appends
`formatTimestamp` (`YYYY-MM-DD-HHMM`, **UTC**). `defaultResultsBase` handles
both the flat `configs/` layout and the nested `configs/suites/` layout.
`--output`/`output_dir` override it. `../reporting/run-summary.ts`
`extractConfigName` parses the same shape back out.

### RS3 — Each worker is a full stack, not a shared one
`TaskWorkerPool.runWorker` builds a `BrowserOSAppManager`, starts it once, and
runs a `TaskRunPipeline` pinned to `appManager.getServerUrl()`. With
`restart_server_per_task`, it restarts the stack between tasks. `num_workers`
is capped at 20 by `../types/config.ts`.

### RS4 — The task queue is index-atomic
`TaskQueue` hands out tasks by incrementing an index, which is safe under
Bun's single-threaded async. `stop()` makes `next()` return `null`, which is
what the dashboard's stop button relies on. Results are re-ordered back into
task order before being returned.

### RS5 — NopeCHA key is patched before any worker launches
`execute()` reads `config.captcha.api_key_env` from `process.env` and calls
`BrowserOSAppManager.patchNopechaApiKey(apiKey)` once, before spawning workers.
Add per-worker setup there, not inside the worker loop.

### RS6 — Pipeline phases are `navigation`, `agent_execution`, `grading`, `cleanup`
`TaskExecutionError` carries a `phase`, and the phase doubles as the
`ErrorSource` recorded in the run summary. Navigation failures abort the task;
agent failures are recorded as errors in metadata and still proceed to grading.

### RS7 — Resume is checked before anything is written
`TaskRunPipeline.execute` calls `hasExistingGraderResults` first and returns the
stored result (mapping `termination_reason: 'timeout'` back to a `timeout`
status) instead of re-running. Cleanup navigates to `about:blank` in a
`finally`, and Infinity app servers are stopped there too.

### RS8 — `artifact-paths.ts` describes the suite-era layout
`createRunId` builds `<suite>__<variant>__<timestamp>` with a path-safe
`safeSegment`; `getRunPaths` yields `runs/<runId>/{run.json, summary.json,
viewer-manifest.json, upload-manifest.json}` and
`tasks/<taskId>/{attempt.json, trace.jsonl, messages.jsonl, grades.json,
screenshots/, grader-artifacts/}`. **Undocumented in the code:** the task
pipeline still writes the flat `<outputDir>/<query_id>/` layout from
`../capture/trajectory-saver.ts`; the two layouts are bridged only in
`../publishing` and `../viewer`. Do not assume a run directory already has the
`tasks/` shape.

## Workflows

### Adding a per-task phase
1. Add the phase to the `TaskExecutionError` phase union in
   `task-run-pipeline.ts` and to `ErrorSourceSchema` in `../types/errors.ts`.
2. Throw it with the task attached; the catch block maps it to a `failed`
   `TaskResult` with the matching `errorSource`.
3. Confirm the run summary's `errorsBySource` still aggregates.

### Running with N workers
Set `num_workers` in the config or `workers` in the suite. Each worker adds one
to the base CDP/server/extension ports, so `N` workers need `3N` free ports.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness-wide rules.
- [`../runner/AGENTS.md`](../runner/AGENTS.md) — process/port management and the legacy shims.
- [`../cli/commands/AGENTS.md`](../cli/commands/AGENTS.md) — the caller.
- [`../dashboard/AGENTS.md`](../dashboard/AGENTS.md) — started/stopped around the run.
- [`../../../tests/runs/AGENTS.md`](../../tests/runs/AGENTS.md) — artifact-path and manifest tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
