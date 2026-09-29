# `tools/` — Go developer tooling (not the shipped CLI)

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

Two independent Go modules that orchestrate local development, both outside
the Bun workspace. `dev/` is the `browseros-dev` CLI — it starts, supervises,
and tears down the browser + server + extension for local work, and is what
every `bun run dev:*` script dispatches to. `dogfood/` is the
`browseros-dogfood` CLI — it runs a separate checkout against a copied profile
with a distinct alpha Dock icon, including a background daemon.

**This is not the user-facing CLI.** The shipped BrowserOS CLI is a separate
Go app at [`../apps/cli/AGENTS.md`](../apps/cli/AGENTS.md), whose `go.mod`
module is `browseros-cli` and whose binary is `browseros-cli`. If you are
looking for "the Go CLI", it is `apps/cli` — this tree is developer-only.

## Contents

```
tools/
├── dev/                          ← module browseros-dev → binary browseros-dev
│ ├── main.go, go.mod, go.sum, Makefile, run.sh, setup.sh
│ ├── cmd/                       ← cobra: setup, watch, test, target, reset, cleanup, dogfood-stop
│ ├── proc/                      ← managed processes, port allocation, run-state lock, colored logs
│ ├── browser/                   ← BrowserOS launch args + CDP readiness
│ └── server/                    ← /health readiness poll
└── dogfood/                     ← module browseros-dogfood → binary browseros-dogfood
    ├── main.go, go.mod, go.sum, Makefile, README.md
    ├── cmd/                     ← cobra: init, start, start-background, daemon, status, stop, restart, pull, logs, config, refresh-profile, source-profile
    ├── config/                  ← ~/.config/browseros-dogfood/config.yaml
    ├── profile/                 ← profile import (allowlist copy) + Local State parsing
    ├── pipeline/                ← setup.sh + wxt build, .env.production writing, git ops
    ├── proc/                    ← managed processes, port resolution, log files
    ├── runlog/                  ← JSONL run log writer/reader
    ├── runtime/                 ← flock-based single-instance lock + run state
    ├── ipc/                     ← Unix-socket JSON control protocol
    ├── browser/                 ← launch args (--browseros-dock-icon=alpha) + CDP wait
    └── internal/fspath/         ← IsSameOrChild guard
```

## Rules

**TL1 — Three separate Go modules, three identities.** `browseros-dev` and
`browseros-dogfood` live here; `browseros-cli` lives in `apps/cli`. Never
import across modules, and never assume a helper in one exists in another.

**TL2 — Both tools are macOS-first.** They launch
`/Applications/BrowserOS.app/Contents/MacOS/BrowserOS`, use `syscall.Flock`,
`/bin/sh`-style temp dirs, and (dogfood) `codesign`. They will not work as-is
on Windows or Linux.

**TL3 — `bun run dev:*` goes through `tools/dev/run.sh`,** which checks for Go,
runs `make -sC tools/dev`, then execs the binary. `scripts/dev/start.ts` is
the older TypeScript path — prefer the Go one.

**TL4 — Every command file has a test.** Both modules keep `*_test.go` beside
each source file (`go test ./...`, wired as `make test` in dogfood's
Makefile). Add tests with behaviour changes.

**TL5 — Shared behaviour is duplicated, not abstracted.** `proc/`,
`browser/`, and the CDP wait loop exist in both modules with slight
differences (dogfood adds log files and a `LineHandler`). Keeping the copies
independent is deliberate — they have separate `go.mod` files.

**TL6 — This tree is not published to npm/Bun.** No `package.json`, no
workspace entry. It is invoked through `make`, shell wrappers, or the Go
toolchain.

## Workflows

**Starting the dev environment:** `bun run dev:watch` (WXT HMR) or
`bun run dev:watch:new` (random free ports, fresh user-data dir) or
`bun run dev:manual` (static extension build, no HMR).

**Cleaning up after a broken run:** `bun run dev:cleanup` (or
`dev:cleanup:dogfood` / `dev:cleanup:prod`) kills the target's processes,
frees its ports, and removes its temp directories.

**Running tests against a live browser:** `bun run test:env` — the Go CLI
kills conflicting ports, starts server + browser, waits for readiness, runs
`bun test`, then cleans up. Add `-- -- <bun test args>` to pass arguments;
`--keep` leaves the environment up for debugging.

**Adding a dogfood command:** 1. New `cmd/<name>.go` with a `cobra.Command`
and a `GroupID` (`groupSetup`, `groupRun`, `groupInspect`). 2. Register it via
`init()` → `rootCmd.AddCommand`. 3. Add a `*_test.go`. 4. Document it in
`dogfood/README.md`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — Bun monorepo parent.
- [`../package.json`](../package.json) — the `dev:*` and `test:env` scripts.
- [`dev/AGENTS.md`](dev/AGENTS.md) — the `browseros-dev` CLI.
- [`dogfood/AGENTS.md`](dogfood/AGENTS.md) — the `browseros-dogfood` CLI.
- [`../apps/cli/AGENTS.md`](../apps/cli/AGENTS.md) — the actual shipped Go CLI.
- [`../CLAUDE.md`](../CLAUDE.md) — UI self-testing workflow.
