# `tools/dogfood/proc/` — processes, ports, and log files

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The runtime layer of the dogfood CLI. `managed.go` supervises a child process
(optionally restarting it) and can tee its output to a log file.
`run.go` runs one-shot commands with streamed, tagged output. `ports.go`
resolves the configured CDP/server/extension ports upward until it finds free
ones. `log.go` defines the colour `Tag` set, the `LineHandler` callback, and
the log-file helpers (listing, retention).

## Contents

```
tools/dogfood/proc/
├── managed.go   ← ProcConfig{Tag, Dir, Env, Restart, Cmd, LogPath, LineHandler}, ManagedProc, StartManaged
├── run.go       ← RunBlocking, RunBlockingWithEnv
├── ports.go     ← ResolvePorts(start config.Ports) (config.Ports, changed bool, error)
├── log.go       ← Tag, LogFile, LineHandler, ListLogFiles, LogMaxAge (24h), StreamLines
└── *_test.go    ← managed_test.go, ports_test.go, log_test.go
```

## Rules

**PC1 — Ports walk upward, they don't randomise.** `ResolvePorts` starts at
the configured value and increments to 65535, tracking already-taken ports
within the same call, and reports whether anything changed so the caller can
log the effective ports. This is deterministic and different from the dev
CLI's random-port strategy — don't "unify" them.

**PC2 — `ProcConfig.LogPath` and `LineHandler` are optional and independent.**
Set `LogPath` to tee to a file (e.g. `server.log`, `chromium.log` from
`../cmd/start.go`), `LineHandler` to react to each line live.

**PC3 — `StartManaged` joins a `sync.WaitGroup`.** It spawns the supervising
goroutine and returns; callers wait on the group for clean shutdown.

**PC4 — `Restart` is opt-in per process.** The server and the browser are not
restarted automatically in dogfood; a crash should stop the run so the log
tells you why.

**PC5 — `LogMaxAge` is 24 h** and applies to log-file retention when the log
directory is cleaned. Raising it grows `~/.config/browseros-dogfood/logs`
without bound.

**PC6 — Streaming goes through `StreamLines`,** which tags stdout and stderr
separately so `logs tail` can filter by stream.

**PC7 — Close handles.** Every helper that opens a file or pipe must release
it on both the success and error path.

## Workflows

**Adding a supervised process:** `proc.StartManaged(ctx, wg, proc.ProcConfig{
Tag: proc.TagServer, Dir: agentRoot, Env: env, Cmd: […],
LogPath: cfg.LogPath("server.log"), LineHandler: …})`.

**Handling a port clash:** nothing to do — `ResolvePorts` already walks up
from the configured values and returns `changed = true`. Log the effective
ports so users know what to point a client at.

**Adding a new log stream:** add a `Tag` in `log.go`, pass a distinct
`LogPath` in `ProcConfig`, and reference the new name in `../cmd/logs.go`
(which enumerates the log directory).

**Debugging a silent process:** either `LogPath` is unset or the tag colour
is being swallowed — check both `cfg.LogDir()` and that `StreamLines` is
actually wired into the `ProcConfig`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rule DF5).
- [`../runlog/AGENTS.md`](../runlog/AGENTS.md) — the structured JSONL run log.
- [`../config/AGENTS.md`](../config/AGENTS.md) — `LogDir`, `LogPath`, default ports.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `start` and `daemon`, the callers.
- [`../../dev/proc/AGENTS.md`](../../dev/proc/AGENTS.md) — the sibling package (different port strategy).
