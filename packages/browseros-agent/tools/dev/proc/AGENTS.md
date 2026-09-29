# `tools/dev/proc/` — process supervision, ports, run-state lock

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dev`.

## What's here

The runtime primitives the dev commands are built on. `managed.go` supervises
a child process (with optional auto-restart) under a cancellable context.
`run.go` runs a one-shot command and streams its output line-by-line with a
colour tag. `process.go` implements the single-instance run-state lock
(`flock` + a JSON `WatchRunState` file). `ports.go` allocates and reserves
ports. `log.go` defines the shared `Tag` set and the coloured log helpers.

## Contents

```
tools/dev/proc/
├── managed.go        ← ProcConfig{Tag, Dir, Env, Restart, Cmd, BeforeStart}, ManagedProc, StartManaged
├── run.go            ← RunBlocking, RunBlockingWithEnv (streams stdout+stderr)
├── process.go        ← WatchRunIdentity/State/Lock, FindMonorepoRoot, errWatchRunLocked
├── ports.go          ← Ports{CDP,Server,Extension}, PortReservations, DefaultLocalPorts, ResolveWatchPorts
├── log.go            ← Tag + Tag{Build,Setup,Agent,Server,Browser,Info,Test}, LogMsg, StreamLines
└── *_test.go         ← process_test.go, ports_test.go, managed_test.go
```

## Rules

**PR1 — Two distinct port strategies.** `ResolveWatchPorts(useRandom)` either
returns the defaults or picks random ports in 9000–9999 *while holding the
listeners open* in `PortReservations`; release them only once the child has
bound. A plain "find a free port, close it, use the number" race is what this
avoids — don't reintroduce it.

**PR2 — `DefaultLocalPorts()` must match `DEFAULT_PORTS` in
`../../packages/shared/src/constants/ports.ts`** (`{9000, 9100, 9300}`). Two
places, one contract; change both or neither.

**PR3 — The run lock is a `flock`, not a PID check.**
`AcquireLock`-style code here writes `WatchRunState{pid, pgid, started_at,
identity{mode, profile, ports}}` and holds `LOCK_EX|LOCK_NB`; a second watcher
gets `errWatchRunLocked`. Both the state file and the lock must be released on
every exit path, including error paths.

**PR4 — `StartManaged` owns the goroutine.** It `wg.Add(1)`s internally and
returns immediately; callers join the `sync.WaitGroup`. `BeforeStart` runs
before each start attempt (use it for cleanup between restarts).

**PR5 — `ManagedProc.Stop` is mutex-guarded** because restarts and stops can
race. Preserve that.

**PR6 — Tags are the logging vocabulary.** Add a new `Tag` in `log.go` with a
colour; don't inline raw ANSI escapes in a command.

**PR7 — `RunBlocking` streams both pipes** through `StreamLines` with the
given tag, then returns the command's exit status. Use it for any long-running
child whose output matters.

## Workflows

**Adding a supervised process:** `proc.StartManaged(ctx, wg, proc.ProcConfig{
Tag: proc.TagServer, Dir: root, Env: env, Cmd: []string{…}, Restart: false })`,
then `mp.Stop()` on shutdown.

**Adding a new port to the triplet:** extend `Ports` and
`PortReservations`, update `ResolveWatchPorts`, and mirror the change in
`../../packages/shared/src/constants/ports.ts` and in the browser launch args
in `../browser/args.go`.

**Clearing a stale lock:** run `bun run dev:cleanup` (kills the owner, frees
ports) rather than deleting the state file — the flock is held by a live
process, and the file is not the source of truth.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dev` scope (rules DV1–DV7).
- [`managed.go`](managed.go) — the supervision primitive.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — the commands that use this package.
- [`../browser/AGENTS.md`](../browser/AGENTS.md) — consumes `proc.Ports` for launch args.
- [`../../../packages/shared/src/constants/AGENTS.md`](../../../packages/shared/src/constants/AGENTS.md) — the mirrored port constants.
