# `tools/dev/browser/` — BrowserOS launch args and CDP readiness

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dev`.

## What's here

The two things needed to start a dev browser: how to build its command line
and how to know it is up. `args.go` assembles the launch flags from an
`ArgsConfig`; `cdp.go` polls the DevTools HTTP endpoint until it answers. Both
are small, pure-ish helpers with unit tests, deliberately kept out of `cmd/`
so the flag list has exactly one home.

## Contents

```
tools/dev/browser/
├── args.go        ← ArgsConfig{Root, Ports, UserDataDir, Headless, LoadDevExtensions}, BuildArgs
├── args_test.go   ← asserts the produced flag list
└── cdp.go         ← WaitForCDP(ctx, port, maxAttempts) → polls /json/version
```

## Rules

**BR1 — `BuildArgs` is the only place BrowserOS flags are written.** It is
consumed by `cmd/watch.go` and `cmd/test.go`. Adding a switch at a call site
means the `args_test.go` assertions no longer describe reality.

**BR2 — The dev identity flags are mandatory.**
`--use-mock-keychain`, `--show-component-extension-options`,
`--disable-browseros-server`, `--browseros-dock-icon=dev`. The last one is
what makes a dev instance visually distinct; the second one disables the
bundled server so the dev server owns the port.

**BR3 — Dev-extension mode is a coherent bundle.** When
`LoadDevExtensions` is true it adds `--no-first-run`,
`--no-default-browser-check`, `--disable-browseros-extensions`, and the
extension load path; otherwise it adds `--enable-logging=stderr` instead.
Don't cherry-pick flags from the two branches.

**BR4 — Headless is `--headless=new`,** never the legacy `--headless`.

**BR5 — Readiness is a 200 from `/json/version`,** polled every 500 ms with a
1 s per-request client timeout, bounded by `maxAttempts` *and* the context.
`WaitForCDP` returns `false` rather than erroring on timeout — the caller
decides whether that's fatal.

**BR6 — CDP and server readiness are separate.** `/json/version` here,
`/health` in `../server/`. The server must not start before the browser
answers, and the browser is not considered ready until its DevTools endpoint
does.

## Workflows

**Adding a launch flag:** 1. Extend `ArgsConfig` if it needs a new input.
2. Append the flag in `BuildArgs` in the correct branch. 3. Extend
`args_test.go`. 4. Confirm the flag exists in Chromium/BrowserOS before
relying on it — an unknown switch can prevent startup.

**Debugging "server never started":** the watch flow gates on
`WaitForCDP`; raise `maxAttempts` at the call site rather than removing the
gate, and check that `--remote-debugging-port` matches the port passed in.

**Switching to random ports (`--new`):** `ArgsConfig.Ports` is filled from
`proc.ResolveWatchPorts(true)`, and the reserved listeners must be released
after the browser has bound them.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dev` scope (rule DV6 covers this folder).
- [`args.go`](args.go) — the flag list.
- [`../proc/AGENTS.md`](../proc/AGENTS.md) — `proc.Ports` source.
- [`../server/AGENTS.md`](../server/AGENTS.md) — the paired health poll.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `watch` and `test`, the callers.
