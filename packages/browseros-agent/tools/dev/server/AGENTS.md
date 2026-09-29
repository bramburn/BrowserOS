# `tools/dev/server/` — server readiness probe

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dev`.

## What's here

One file, `health.go`, with one function: `WaitForHealth(ctx, port,
maxAttempts) bool`. It polls `http://127.0.0.1:<port>/health` until it gets a
200, gives up after `maxAttempts`, and aborts immediately if the context is
cancelled. It exists so the dev orchestrator doesn't start extensions or hand
off to tests against a server that isn't listening yet.

## Contents

```
tools/dev/server/
└── health.go   ← WaitForHealth(ctx context.Context, port int, maxAttempts int) bool
```

## Rules

**SR1 — `/health` is the readiness contract.** The server's
`api/routes/health.ts` must keep answering this path; a health probe is not a
place to add auth or make the handler heavier.

**SR2 — Poll cadence is 500 ms with a 1 s per-request client timeout,** the
same as `../browser/cdp.go`. Keep the two probes symmetrical so the dev
orchestrator has one timing model.

**SR3 — Return a bool, don't error.** `false` means "not ready within
`maxAttempts` or the context ended". The caller decides whether that aborts
the run; the probe has no opinion about why.

**SR4 — Respect `ctx.Done()` between attempts,** via the same
`select` used in the browser probe. A `WaitForHealth` that ignores cancellation
delays Ctrl+C by the full attempt budget.

**SR5 — Close every response body.** The probe discards the body but must
still `resp.Body.Close()` or the connection is leaked across attempts.

**SR6 — Only one readiness probe lives here.** Anything else about the
server (start command, env, restart) belongs in `cmd/`, not in a health
module.

## Workflows

**Gating a step on server readiness:** `if !server.WaitForHealth(ctx,
ports.Server, attempts) { return errors.New("server did not become healthy") }`
— keep the failure message explicit, because "server never started" is the
most common dev-loop complaint.

**Raising the startup budget:** pass a larger `maxAttempts` at the call site
(0.5 s × attempts ≈ wall time). Don't change the poll interval here to buy
time; that would desync it from the browser probe.

**After changing the health route:** re-run `bun run dev:watch` and confirm
the gate still passes. A health route that returns 503 while starting up is
fine; a route that 404s is not.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dev` scope (rule DV5 covers both probes).
- [`health.go`](health.go) — the only file.
- [`../browser/AGENTS.md`](../browser/AGENTS.md) — the paired CDP probe.
- [`../../../apps/server/AGENTS.md`](../../../apps/server/AGENTS.md) — the `/health` route implementation.
