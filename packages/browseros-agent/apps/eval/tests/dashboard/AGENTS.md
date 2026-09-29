# `tests/dashboard/` — Dashboard tests

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

One file, `server.test.ts`, with two assertions on
`shouldAutoOpenDashboard(env)`: false when `CI` is set, true otherwise. That
helper is the only part of `src/dashboard/server.ts` that is pure enough to
test — the Hono app itself, the SSE stream, and the screenshot/messages routes
all read from disk and are exercised by running the harness, not by `bun test`.

```
tests/dashboard/
└── server.test.ts   ← shouldAutoOpenDashboard CI / non-CI behaviour
```

## Rules

### TDB1 — Pass `env` in, never mutate `process.env`
`shouldAutoOpenDashboard({ CI: 'true' })` takes an explicit env record. Keep the
parameter; a version that reads `process.env` directly would make the test
order-dependent.

### TDB2 — This is the whole coverage, and that is fine
Do not add HTTP tests that boot `Bun.serve`. The dashboard is a developer tool
on loopback; the run it displays is verified by the run itself. If a dashboard
change breaks the harness, it breaks `src/runs/eval-runner.ts` visibly.

### TDB3 — Auto-open is macOS-only
The implementation shells out to `open`. The test asserts the *decision*, not
the side effect, which is why it works on any platform — keep it that way if
the helper is ever generalised.

## Workflows

### Changing dashboard start-up
1. Edit `src/dashboard/server.ts`.
2. Update `server.test.ts` only if `shouldAutoOpenDashboard`'s contract changed.
3. Verify manually: `bun run eval` with no arguments starts the config-mode
   dashboard on `http://localhost:9900` and blocks forever.

## Cross-references

- [`../../src/dashboard/AGENTS.md`](../../src/dashboard/AGENTS.md) — the folder under test.
- [`../../src/runs/AGENTS.md`](../../src/runs/AGENTS.md) — the other caller of `startDashboard`.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
