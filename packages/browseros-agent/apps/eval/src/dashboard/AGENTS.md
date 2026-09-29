# `src/dashboard/` — Local run dashboard

> Part of [`../AGENTS.md`](../AGENTS.md) in `src/`; a localhost view of a running eval.

## What's here

`server.ts` is a Hono app served with `Bun.serve` on `127.0.0.1:9900`
(`startDashboard`), serving `index.html` at `/` and a live JSON/SSE API.
`index.html` is the operator UI (start/stop a run, watch per-task status,
screenshots, messages); `viewer.html` is the richer trajectory viewer. The
dashboard is started by `runs/eval-runner.ts` for every CLI run and by
`cli/index.ts` in config mode (no `--config` → a UI where you pick a config and
press run).

```
dashboard/
├── server.ts      ← Hono app, DashboardState, startDashboard/stopDashboard
├── index.html     ← operator UI (58 KB, inline JS)
└── viewer.html    ← trajectory viewer (86 KB, inline JS)
```

## Rules

### DB1 — The dashboard is loopback-only
`hostname: '127.0.0.1'`, default port 9900. It serves task queries and
screenshots straight off disk; never bind it to a public interface.

### DB2 — `setActiveExecutor` is the run/stop handshake
`runs/eval-runner.ts` calls `setActiveExecutor(executor)` before
`executor.execute(...)` and clears it in a `finally`; `TaskWorkerPool.stop()`
is what `POST /api/stop` invokes. `evalRunning` is derived from the executor
being non-null, and load/start/stop all return 409 while a run is active.

### DB3 — Events arrive as SSE, state as JSON
`GET /api/state` is a snapshot; `GET /api/events` is a `streamSSE` subscription
fed by `DashboardState.broadcastStreamEvent` / `broadcastScreenshot`. The
per-tool stream from `capture/context.ts` is what makes the UI live.

### DB4 — Config browsing is path-guarded
`/api/config/*` resolves against `../../configs` and `resolveConfigPath`
rejects anything that is not `*.json` or contains `.` / `..` segments, and
`GET /api/screenshots/:taskId/:index` rejects `..` and `/` in `taskId`. Keep
both guards when adding a route.

### DB5 — Don't auto-open in CI
`shouldAutoOpenDashboard(env)` returns false when `CI` is set, otherwise true.
It shells out to `open` (macOS). `tests/dashboard/server.test.ts` pins both
branches; the `open` call only works on macOS, which is also the only host the
rest of this harness is built for (see `BROWSEROS_BINARY`).

## Workflows

### Watching a long run
`bun run eval run --config configs/legacy/<name>.json` starts the dashboard
automatically; open `http://localhost:9900`. The CLI still prints the final
`summary.json` path when the run finishes.

### Loading a finished run
`POST /api/load-run` reads an existing `results/<config>/<timestamp>` directory
into `DashboardState` (the `/api/runs` list enumerates directories containing
timestamp-shaped subdirectories). It is rejected with 409 while an eval runs.

### Changing the UI
Both HTML files are self-contained (no build step, no bundler). Edit them in
place; there is no component tree to update.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — harness map.
- [`../runs/AGENTS.md`](../runs/AGENTS.md) — the caller that starts/stops the dashboard.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — the event stream the UI renders.
- [`../../../configs/AGENTS.md`](../../configs/AGENTS.md) — the configs the UI lists.
- [`../../../tests/dashboard/AGENTS.md`](../../tests/dashboard/AGENTS.md) — the one dashboard test.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
