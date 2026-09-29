# `src/runner/` — Process, port, and task-source management

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; everything the runner owns outside `runs/`.

## What's here

Three legacy-named re-export shims (`eval-runner.ts`, `parallel-executor.ts`,
`task-executor.ts`) that forward to `../runs/`, plus four real modules:
`browseros-app-manager.ts` (Chrome + Bun server lifecycle per worker),
`infinity-app-manager.ts` (WebArena-Infinity app server per task),
`task-loader.ts` (JSONL → validated `Task[]`), and `types.ts` (the result
union and the pass/fail precedence helper).

```
runner/
├── browseros-app-manager.ts  ← BrowserOSAppManager: ports, Chrome, server, restarts
├── infinity-app-manager.ts   ← InfinityAppManager: per-task app server
├── task-loader.ts            ← loadTasks, TaskLoadError, TaskValidationError
├── types.ts                  ← TaskResult, BatchSummary, getPrimaryGraderResult
├── eval-runner.ts            ← re-export: runEval            → ../runs/eval-runner
├── parallel-executor.ts      ← re-export: TaskWorkerPool     → ../runs/task-worker-pool
└── task-executor.ts          ← re-export: TaskRunPipeline    → ../runs/task-run-pipeline
```

## Rules

### RN1 — The three shims exist for import compatibility only
`ParallelExecutor` is `TaskWorkerPool`, `TaskExecutor` is `TaskRunPipeline`.
`../dashboard/server.ts` still imports through them. New code should import from
`../runs/` directly; do not add exports to the shims. (Observed convention — no
automated check enforces it.)

### RN2 — Worker N owns ports `base + N`
`BrowserOSAppManager` computes `cdp = base.cdp + workerIndex`, likewise for
`server` and `extension`, and defaults to `{ cdp: 9010, server: 9110,
extension: 9310 }`. `../agents/single-agent.ts` recomputes the CDP port with the
same formula. Both must change together.

### RN3 — Restart policy is part of the contract
`restart()` retries up to `MAX_RESTART_ATTEMPTS = 3`, killing first, sleeping 2s,
then launching. Timeouts: `CDP_WAIT_TIMEOUT_MS = 30_000`,
`SERVER_HEALTH_TIMEOUT_MS = 90_000` (raised for cold dev-module graphs). Chrome
is launched with `--disable-browseros-server` and
`--disable-browseros-extensions`, a per-worker `mkdtemp('/tmp/browseros-eval-')`
profile, `--window-size=1440,900`, and optionally `--headless=new`. This
mirrors `scripts/dev/start.ts` in the server package — if that changes, this must
change.

### RN4 — Server logs are captured, not discarded
Both server stdout and stderr go to a per-worker file under
`BROWSEROS_SERVER_LOG_DIR` (default `/tmp/browseros-server-logs`) so the weekly
workflow can upload them as artifacts. Chrome's streams are ignored.

### RN5 — A task dir is defined by `metadata.json`
Task discovery in `task-loader.ts`'s consumers, `cli/commands/grade.ts` and
`../publishing/r2-publisher.ts`, all use "subdirectory containing
`metadata.json`". Keep the predicate identical.

### RN6 — Dataset loading is all-or-nothing
`loadTasks` parses each JSONL line through `TaskSchema`, collects errors, and
throws `TaskLoadError` listing up to five failing line numbers. It also rejects
duplicate `query_id`s, because `query_id` is the output directory name. A
single malformed line fails the whole file.

### RN7 — macOS-shaped by default
`BROWSEROS_BINARY` defaults to
`/Applications/BrowserOS.app/Contents/MacOS/BrowserOS` and the captcha
extension path is `../../extensions/nopecha` (git-ignored, unpacked locally).
The harness is built and CI-run on macOS; the dashboard's `open` call assumes
the same.

## Workflows

### Debugging a worker that won't start
1. `BROWSEROS_SERVER_LOG_DIR` files — one per worker, containing server
   stdout+stderr.
2. The console prints `[W<index>] Ports: CDP=… Server=… Extension=…` and the
   profile dir; a port clash with another BrowserOS instance shows up here.
3. `killApp()` is called at the start of every `restart()`; a stale process from
   a killed run will hold the port.

### Running a one-off task
`loadTasks({ type: 'single', query, startUrl })` fabricates a task with
`query_id: single-<epoch-ms>`, `dataset: 'manual'`,
`graders: ['performance_grader']`. This is the fastest way to smoke-test a
suite's `browseros` block.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness map and the `runs/` vs `runner/` rule.
- [`../runs/AGENTS.md`](../runs/AGENTS.md) — the orchestration these feed.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — the CDP port contract consumer.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — how a run is started.
- [`../../../tests/runs/AGENTS.md`](../../tests/runs/AGENTS.md) — `pipeline-compat.test.ts` pins the shims.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
