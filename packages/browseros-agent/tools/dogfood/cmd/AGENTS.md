# `tools/dogfood/cmd/` — `browseros-dogfood` commands

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

Every cobra command of the dogfood CLI. `root.go` builds the root command and
its custom usage template with four groups (Setup, Run, Inspect, Other) plus
template funcs registered in `style.go`. The verbs split into setup
(`init`, `config`), run (`start`, `start-background`, `pull`,
`refresh-profile`), inspect (`status`, `logs`), and control
(`stop`, `restart`, plus the hidden `daemon`). `source_profile.go` backs the
`source-profile` listing used by `init`.

## Contents

```
tools/dogfood/cmd/
├── root.go             ← rootCmd, usageTemplate, AddGroup(setup/run/inspect/other), Execute()
├── style.go            ← group id consts, color styles, helpHeader/helpHint/groupedHelp
├── init.go             ← init: prompts for repo path, browser binary, source profile
├── start.go            ← start / start-background [--refresh-profile] [--headless]
├── daemon.go           ← daemon (hidden): the background worker, runPaths, server health wait
├── control.go          ← status / stop / restart, daemonMonitor, log following
├── logs.go             ← logs: list log files in the config log dir
├── pull.go             ← pull [--force]: writes env files, reports branch/head/dirty
├── refresh_profile.go  ← refresh-profile: copy source profile into dev profile
├── source_profile.go   ← source-profile: list the real profiles in the source user-data dir
├── config.go           ← config edit: open config.yaml in $EDITOR
└── *_test.go           ← root, start, init, control, daemon, logs, pull, refresh_profile, source_profile tests
```

## Rules

**CC1 — Register in `init()` and set a `GroupID`.** Every user-facing command
belongs to `groupSetup`, `groupRun`, or `groupInspect`; `help` is
`groupOther` via `rootCmd.SetHelpCommandGroupID`.

**CC2 — `daemon` stays `Hidden: true`.** It is the implementation behind
`start-background`. Users get `status`/`stop`/`restart`.

**CC3 — `start` and `start-background` share the lock, the build, and the
launch path** in `start.go`; the difference is whether the process stays in
the foreground. Don't fork the logic.

**CC4 — Control commands go over IPC, not signals.**
`control.go` sends `ipc.Request{command: status|stop|restart}` to the daemon's
Unix socket and consumes a `runlog` stream for `logs tail`. A missing daemon
surfaces as `ipc.ErrDaemonNotRunning`, handled here.

**CC5 — Server readiness in the daemon is 120 attempts × 500 ms** (60 s),
`serverHealthAttempts` / `serverHealthInterval` in `daemon.go`. Keep those
constants together.

**CC6 — Interactive commands read stdin explicitly** (`bufio.Scanner`) and
print with the shared styles in `style.go`, never raw ANSI escapes.

**CC7 — `pull` is `--ff-only` by default** and refuses a dirty checkout unless
`--force`; it reports branch, short head, and dirty state before acting.

## Workflows

**Adding a command:** 1. New `cmd/<name>.go` with a package-level
`cobra.Command` (`Use`, `Short`, `GroupID`, `RunE`). 2. Bind flags in `init()`
and `rootCmd.AddCommand`. 3. Add `cmd/<name>_test.go`. 4. Add a README
section under Daily Use.

**Adding a daemon capability:** 1. Add a command const in
`../ipc/ipc.go`. 2. Handle it in the daemon's `Handler.Handle` in
`daemon.go`. 3. Expose it from `control.go` with a `GroupID` and a
`daemonMonitor` entry.

**Debugging "already running":** another `start`/`start-background` holds the
`runtime` flock. `browseros-dogfood status` tells you which mode and PID;
`stop` releases it.

**Changing default ports:** edit `config.Defaults` in `../config/config.go`
(9015/9115/9315), not the call sites.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rules DF1–DF7).
- [`root.go`](root.go) — command + group registration.
- [`../ipc/AGENTS.md`](../ipc/AGENTS.md) — the control protocol used here.
- [`../config/AGENTS.md`](../config/AGENTS.md) — config load/save/validate.
- [`../README.md`](../README.md) — the documented user workflow.
