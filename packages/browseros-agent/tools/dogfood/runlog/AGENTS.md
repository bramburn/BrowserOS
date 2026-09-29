# `tools/dogfood/runlog/` — structured run log

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The background run's append-only log. `Writer` serialises every
tagged/streamed line as a JSON object (`{time, tag, stream, line}`) to a file
under the config log directory; the reader side lets `logs tail` follow the
same file while the daemon is running. It is what makes `browseros-dogfood
logs tail` able to interleave daemon, BrowserOS, and server output after the
fact.

## Contents

```
tools/dogfood/runlog/
├── log.go       ← Entry{Time,Tag,Stream,Line}, Writer (mutex + json.Encoder, O_APPEND), NewWriter, Append, reader/follow helpers
└── log_test.go  ← writer and follow tests
```

## Rules

**RL1 — One JSON object per line (JSONL).** The file is appended with
`O_CREATE|O_WRONLY|O_APPEND` and a `json.Encoder`, so a truncated tail is
still parseable up to the last complete line.

**RL2 — `Append` is mutex-guarded and nil-safe.** A `*Writer` receiver that is
`nil` returns without doing anything, which lets callers wire a log writer
into an optional path without nil checks at every call site.

**RL3 — Don't reformat lines.** `Tag`, `Stream`, and `Line` are stored
verbatim; the consumer decides how to render them. Parsing must not be needed
to read a run log.

**RL4 — The file is created lazily on `NewWriter`,** including its parent
directory (`os.MkdirAll`). Callers don't pre-create the log dir.

**RL5 — Rotation is age-based, done by the caller.** `runlog` appends; log
retention (`LogMaxAge`, 24 h) is applied in `../proc/log.go`. Keep the two
concerns separate.

**RL6 — Following is a read loop, not a tail(1).** The follow helper polls
the growing file so it works the same on macOS, where `tail -f` semantics on
an actively written file are awkward.

## Workflows

**Wiring a run log into the daemon:** `runlog.NewWriter(cfg.LogPath("run.log"))`
→ pass as the `LineHandler` target in `../proc` `ProcConfig` →
`cmd/daemon.go` owns the writer lifetime and closes it on shutdown.

**Consuming a log:** `browseros-dogfood logs` lists the files; `logs tail`
follows the run log through `cmd/control.go`'s `daemonMonitor`, filtering by
tag when asked.

**Adding a field to an entry:** extend `Entry` — it's a JSON struct, so old
lines simply lack the key. Readers must tolerate missing fields.

**Debugging a truncated log:** check whether the process was killed mid-write;
JSONL recovers at the last complete line, and the follow helper skips a
partial trailing line.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope.
- [`../proc/AGENTS.md`](../proc/AGENTS.md) — produces the lines this stores.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — `logs` and `logs tail` consumers.
- [`../config/AGENTS.md`](../config/AGENTS.md) — `LogDir` / `LogPath` locations.
