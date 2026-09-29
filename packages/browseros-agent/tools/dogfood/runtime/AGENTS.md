# `tools/dogfood/runtime/` — single-instance lock and run state

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The mutual-exclusion primitive for dogfood runs. `lock.go` takes an exclusive
`syscall.Flock` on a lock file (returning `ErrAlreadyRunning` on contention)
and, alongside it, reads and writes a `RunState` — pid, mode, start time,
socket path, log path — so `status` can report what is running and `stop` can
find the daemon. This is what guarantees only one dogfood environment exists
at a time, in both `start` and `start-background` mode.

## Contents

```
tools/dogfood/runtime/
├── lock.go       ← Lock, AcquireLock(path), Lock.Close(), RunState, ErrAlreadyRunning
└── lock_test.go  ← lock contention and run-state tests
```

## Rules

**RT1 — `flock`, not a PID file check.** A stale PID file says nothing about
whether the holder is alive; an advisory `LOCK_EX|LOCK_NB` lock is released
by the kernel when the process dies. Keep using `flock`.

**RT2 — `ErrAlreadyRunning` is a first-class outcome,** not a wrapped
failure. `cmd/start.go` and `cmd/daemon.go` render it as "already running"
and exit; they must not retry or treat it as a crash.

**RT3 — Create the parent directory before locking.**
`AcquireLock` does `os.MkdirAll(filepath.Dir(path))` first, so callers don't
pre-create the run directory.

**RT4 — `Close` is nil-safe and unlocks explicitly.** It tolerates a nil
`*Lock` and a nil file, calls `LOCK_UN`, then closes. Defer it immediately
after acquiring.

**RT5 — `RunState` is a JSON contract with the Go CLI ecosystem.** Fields are
`pid`, `mode`, `started_at`, `socket_path`, `log_path` in snake_case. Don't
rename them; `status` output and any external tooling read them.

**RT6 — The lock file and the state file are separate concerns.** The lock
serialises; the state file reports. Deleting the state file to "unstick"
things is wrong — the lock is the gate.

## Workflows

**Guarding a run:** `lock, err := runtime.AcquireLock(path)`; on
`ErrAlreadyRunning` print the current `RunState` (pid, mode, since) and exit;
otherwise `defer lock.Close()` and write the state file with the socket and
log paths once they're known.

**Implementing `stop`:** read `RunState` → send `ipc.Request{command:
"stop"}` to `state.SocketPath` → wait for the daemon to exit → the lock is
released by the kernel. Never kill by PID without checking the mode.

**Changing what's recorded:** extend `RunState` with new JSON fields. Consumers
must tolerate older state files that lack them.

**Debugging a lock that won't clear:** the holder is a live process, not a
stale file. `browseros-dogfood status` shows the pid and mode; stop that run
first.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope (rule DF3).
- [`../ipc/AGENTS.md`](../ipc/AGENTS.md) — the socket path recorded in `RunState`.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `start` / `start-background` / `status` / `stop`.
- [`../cmd/daemon.go`](../cmd/daemon.go) — the long-running lock holder.
