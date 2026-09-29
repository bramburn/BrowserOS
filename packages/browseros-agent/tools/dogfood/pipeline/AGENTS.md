# `tools/dogfood/pipeline/` — build, env, and git steps

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The steps between "you typed `start`" and "the browser has something to
launch". `build.go` runs the checkout's setup and extension build. `env.go`
writes the two `.env.production` files the build scripts need. `git.go`
wraps the update operations (`pull --ff-only`, `fetch --prune`,
`reset --hard @{upstream}`, `status --porcelain`, `rev-parse`, branch) behind
a `Runner` interface so they are testable without a real repo.

## Contents

```
tools/dogfood/pipeline/
├── build.go    ← Build(ctx, agentRoot, Runner); ExecRunner
├── env.go      ← WriteProductionEnvFiles(agentRoot, cfg), writeEnvFile, formatEnvLine
├── exec.go     ← runCommand (streaming), outputCommand (capturing)
├── git.go      ← Runner interface; Dirty, Pull, Fetch, ResetHardToUpstream, Head, Branch
└── *_test.go   ← build_test.go, env_test.go, git_test.go
```

## Rules

**PL1 — Everything goes through the `Runner` interface.** `Run(ctx, dir,
args…)` and `OutputRun(dir, args…)`; production uses `ExecRunner{}`, tests
use a fake. A direct `exec.Command` inside `git.go` makes it untestable —
don't add one.

**PL2 — `Build` is exactly two steps:** `./tools/dev/setup.sh`, then
`bun --cwd apps/agent --env-file=.env.development wxt build --mode
development`. Both are pinned; changing either changes what lands in
`apps/agent/dist/chrome-mv3-dev`, which is what `../browser/args.go` loads.

**PL3 — Env files are written sorted and quoted.** `writeEnvFile` sorts keys
before writing so diffs are stable; `formatEnvLine` handles quoting.

**PL4 — `Pull` is `--ff-only` and never automatic.** Only `pull` /
`restart --pull` in `../cmd/` trigger it. `start` does not.

**PL5 — `Dirty` is `git status --porcelain` trimmed non-empty,** and a dirty
checkout is a warning, not an error — `--force` on `pull` is the opt-in.

**PL6 — Writes are into the configured checkout, not this repo.** `agentRoot`
comes from `config.AgentRoot()`; the dogfood tool operates on whatever repo
the user configured, which is usually not the one these files live in.

**PL7 — `outputCommand` captures; `runCommand` streams.** Use `outputCommand`
for anything whose stdout you parse, `runCommand` for builds you want to
watch.

## Workflows

**A full `start`:** `pipeline.WriteProductionEnvFiles` →
`pipeline.Build(ctx, cfg.AgentRoot(), ExecRunner{})` → resolve ports →
launch browser with `--load-extension apps/agent/dist/chrome-mv3-dev`.

**Adding a build step:** 1. Add it to `Build` in `build.go` in the right
order. 2. Add a case to `build_test.go` asserting the command sequence through
a fake `Runner`. 3. Update `../README.md` if it changes what users wait for.

**Adding a git operation:** add a function in `git.go` that takes
`(ctx, repoPath, Runner)` and returns parsed output or an error. Never a
`Runner`-free function.

**Debugging a failed build:** run the two `Build` commands manually in the
configured checkout; the first (`tools/dev/setup.sh`) needs Go and Bun, the
second needs `apps/agent/.env.development`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rule DF6).
- [`build.go`](build.go) — the pinned build steps.
- [`../config/AGENTS.md`](../config/AGENTS.md) — `ProductionEnv` and `AgentRoot`.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `start` and `pull`, the callers.
- [`../../dev/AGENTS.md`](../../dev/AGENTS.md) — the `tools/dev/setup.sh` this invokes.
