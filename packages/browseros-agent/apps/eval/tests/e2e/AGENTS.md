# `tests/e2e/` — Manual end-to-end script

> Part of [`../AGENTS.md`](../AGENTS.md) in `tests/`.

## What's here

`captcha-e2e.ts` is a single runnable script — not a `*.test.ts`, so `bun test`
does not collect it. It drives a live BrowserOS instance through a captcha flow
to verify that `CaptchaWaiter` plus the NopeCHA extension actually unblock a
task in practice, which is the one thing the unit tests in
`tests/capture/` cannot establish.

```
tests/e2e/
└── captcha-e2e.ts   ← manual: real server, real captcha, real wait loop
```

## Rules

### TE1 — Never rename it to `*.test.ts`
`bun test` would then try to run it as part of the fast suite, where it cannot
pass (no server, no API key, no network).

### TE2 — It needs a full local environment
A running BrowserOS (default `http://127.0.0.1:9110`), the NopeCHA extension
unpacked under `apps/eval/extensions/nopecha`, and `NOPECHA_API_KEY` set. The
harness launches its own stack when run through the CLI, but this script talks
to a server that is already up.

### TE3 — It is evidence, not a gate
Failures here mean the captcha path regressed; they do not block a commit the
way `bun test` does. Record what it printed when reporting a captcha issue.

## Workflows

### Running it
```bash
cd apps/eval
bun tests/e2e/captcha-e2e.ts
```
Expect it to: start (or attach to) a page with a widget, call
`CaptchaWaiter.waitIfCaptchaPresent`, and print the resulting
`CaptchaWaitResult` (`detected`, `type`, `solved`, `waitDurationMs`).

### Deciding whether a captcha bug is real
1. Reproduce here.
2. If it reproduces, check `src/capture/captcha-waiter.ts` detection script and
   the `captcha.api_key_env` wiring in
   `src/runner/browseros-app-manager.ts` (`patchNopechaApiKey`).
3. If it does not, the unit test in `tests/capture/` should still cover the
   new branch.

## Cross-references

- [`../../src/capture/AGENTS.md`](../../src/capture/AGENTS.md) — the code under test.
- [`../capture/AGENTS.md`](../capture/AGENTS.md) — the unit-level counterpart.
- [`../../src/runner/AGENTS.md`](../../src/runner/AGENTS.md) — NopeCHA extension loading and key patching.
- [`../AGENTS.md`](../AGENTS.md) — test-tree rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — Bun monorepo parent.
