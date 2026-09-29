# `tools/dev/` — `browseros-dev` process orchestrator

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`.

## What's here

The `browseros-dev` Go CLI (module `browseros-dev`, Go 1.25.7): a
process-supervision harness for local BrowserOS development. It installs
dependencies, launches the agent extension (WXT HMR or a static build),
waits for the browser's CDP endpoint and the server's `/health`, starts the
server, and supervises all of it until Ctrl+C. It also owns port allocation,
a single-instance run-state lock, and destructive cleanup helpers. Every
`bun run dev:*` script in the monorepo root package.json dispatches here via
`run.sh`.

## Contents

```
tools/dev/
├── main.go        ← package main → cmd.Execute()
├── go.mod, go.sum ← module browseros-dev; cobra, fatih/color, yaml.v3
├── Makefile       ← `go build -o browseros-dev .`, `make clean`
├── run.sh         ← checks for Go, `make -sC`, execs ./browseros-dev "$@"
├── setup.sh       ← thin wrapper: `exec run.sh setup "$@"`
├── cmd/
│ ├── root.go        ← root cobra command "browseros-dev", SilenceUsage/Errors
│ ├── setup.go       ← bun install + agent GraphQL codegen
│ ├── watch.go       ← the dev loop: agent → CDP → server, signal handling
│ ├── test.go        ← start env, run `bun test` with passthrough args, clean up
│ ├── target.go      ← dev | dogfood | prod targets: ports, dirs, Lima home
│ ├── reset.go       ← guided destructive reset (profile + Lima VM)
│ ├── cleanup.go     ← kill processes, free ports, remove temp dirs
│ ├── dogfood_stop.go← talks to the dogfood daemon's IPC socket to stop it
│ ├── style.go       ← shared fatih/color styles
│ └── *_test.go      ← tests beside each command
├── proc/          ← run.go, managed.go, process.go (lock/state), ports.go, log.go
├── browser/       ← args.go (launch flags), cdp.go (WaitForCDP)
└── server/        ← health.go (WaitForHealth against /health)
```

## Rules

**DV1 — `run.sh` is the only supported entry.** It verifies Go is installed,
builds via `make -sC`, then `exec`s the binary. The `Makefile` target is
`$(BINARY): $(SOURCES)` over `find . -name '*.go'` plus `go.mod`/`go.sum`.

**DV2 — Ports come from `proc.Ports` and the target system.**
`DefaultLocalPorts()` is `{9000, 9100, 9300}`; `--new` picks random ports in
9000–9999 and holds the sockets open via `PortReservations` so nothing steals
them before the child binds. These mirror `DEFAULT_PORTS` in
`../../packages/shared/src/constants/ports.ts` — change both together.

**DV3 — Only one watch run at a time.** `proc/process.go` writes a
`WatchRunState` (pid, pgid, mode, profile, ports) and holds an exclusive
`flock`; a second run gets `dev watch run is already locked`.

**DV4 — `ManagedProc` is the supervision primitive.**
`StartManaged(ctx, wg, ProcConfig)` restarts a child when `Restart` is set and
cancels it when the context is cancelled. Direct `exec.Command` belongs in
`RunBlocking` / `RunBlockingWithEnv`.

**DV5 — Readiness is polled, never assumed.** `browser.WaitForCDP` polls
`/json/version` and `server.WaitForHealth` polls `/health`, both 500 ms apart
with a 1 s per-request timeout, both bounded by `maxAttempts` and the context.

**DV6 — Browser launch args are assembled in one place.** `browser.BuildArgs`
is the only function that knows the flags: `--disable-browseros-server`,
`--browseros-dock-icon=dev`, `--use-mock-keychain`,
`--remote-debugging-port=`, optional `--load-extension` /
`--headless=new`. Add flags there, not at call sites.

**DV7 — Targets are a closed enum.** `dev`, `dogfood`, `prod` in
`cmd/target.go`, each with its own BrowserOS dir (`.browseros-dev` vs
`.browseros`), Lima home, and ports.

## Workflows

**Watch mode:** `bun run dev:watch` starts the agent, waits for CDP, then the
server; `--new` uses random ports and a fresh user-data dir; `--manual`
replaces WXT HMR with a static `apps/agent` build. Ctrl+C tears everything
down through the context.

**Adding a supervised process:** 1. Add it to the startup sequence in
`cmd/watch.go`. 2. Wrap it in `proc.StartManaged` with a `Tag` from
`proc/log.go`. 3. Gate the next step on a readiness poll (`WaitForCDP` /
`WaitForHealth`).

**Adding a subcommand:** 1. New `cmd/<name>.go` with a `cobra.Command`. 2.
Register it in `init()` via `rootCmd.AddCommand`. 3. Add `cmd/<name>_test.go`.
4. Optionally expose it as a `bun run dev:*` script in
`../../package.json`.

**Resetting state:** `bun run dev:reset` (or `:dogfood`, `:prod`) walks
through safe cleanup, Lima VM shutdown/deletion, and profile reset, with
prompts. `bun run dev:cleanup` is the non-interactive variant.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tools/` scope (rules TL1–TL6).
- [`../dogfood/AGENTS.md`](../dogfood/AGENTS.md) — the sibling CLI (shares proc/browser design).
- [`../../package.json`](../../package.json) — the `dev:*`, `test:env`, `install:browseros-dogfood` scripts.
- [`../../packages/shared/src/constants/AGENTS.md`](../../packages/shared/src/constants/AGENTS.md) — the port constants this mirrors.
- [`../../scripts/dev/AGENTS.md`](../../scripts/dev/AGENTS.md) — the older TypeScript dev path.
