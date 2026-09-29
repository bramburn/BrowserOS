# `tools/dogfood/ipc/` — Unix-socket control protocol

> Part of [`../AGENTS.md`](../AGENTS.md) in
> `packages/browseros-agent/tools/dogfood`.

## What's here

The control channel between the foreground CLI and the background daemon.
`ipc.go` defines the request/response envelope, the three commands
(`status`, `stop`, `restart`), a `Handler` interface with a `HandlerFunc`
adapter, a `Server` that listens on a Unix domain socket, and the client side
that sends a request and reads the response. `cmd/control.go` and
`cmd/daemon.go` sit on either side of it; `tools/dev/cmd/dogfood_stop.go` also
speaks this protocol from the sibling CLI.

## Contents

```
tools/dogfood/ipc/
├── ipc.go      ← Cmd{Status,Stop,Restart}, Request, Response, Handler/HandlerFunc, Server, NewServer, client send, ErrDaemonNotRunning
└── ipc_test.go ← server/client round-trip and error tests
```

## Rules

**IP1 — Three commands, named by string constants.** Add a new command as a
const in `Cmd*` and handle it in the daemon's `Handler`; don't pass
free-form strings.

**IP2 — Responses are always `Response{OK bool, Data any, Error string}`.**
A non-OK response with an empty `Error` is a bug — the CLI prints `Error`
verbatim to the user.

**IP3 — The server read timeout is 5 s; the client default response timeout
is 15 minutes.** `restart` can be slow (rebuild), which is why the client
budget is large; the server's read timeout stays short so a dead client
doesn't wedge a connection.

**IP4 — A missing socket is `ErrDaemonNotRunning`,** not a generic dial
error. Every caller branches on that sentinel to print "not running" instead
of a stack trace.

**IP5 — Stale sockets are removed before listening.** The `Server` must
unlink the socket path on start (and clean it up on stop), otherwise a crashed
daemon leaves a file that makes every later start fail.

**IP6 — `HandlerFunc` is the adapter for plain functions.** Implement
`Handler` only when you need state; most commands are a closure.

**IP7 — No new transport.** The protocol is newline-free JSON over a Unix
socket because that is what `tools/dev` also implements; a second mechanism
means a second state file.

## Workflows

**Adding a control command:** 1. Add `CmdRestart`-style const in `ipc.go`.
2. Handle it in the daemon's `Handler.Handle` in `../cmd/daemon.go`. 3. Add a
cobra verb in `../cmd/control.go` that sends the request. 4. Extend
`ipc_test.go` with a round-trip.

**From the dev CLI:** `tools/dev/cmd/dogfood_stop.go` implements the same
`{command}` JSON over the socket with a 10 s stop timeout, to make
`bun run dev:reset` safe when a dogfood daemon is up.

**Debugging "already running" vs "not running":** "already running" comes from
the `runtime` lock (`../runtime/AGENTS.md`); "not running" comes from
`ErrDaemonNotRunning` here. They are different failures with different fixes.

**Checking the daemon directly:** read the socket path from the `RunState`
JSON and send `{"command":"status"}` over it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `browseros-dogfood` scope.
- [`ipc.go`](ipc.go) — the protocol definition.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — client (`control.go`) and server (`daemon.go`).
- [`../runtime/AGENTS.md`](../runtime/AGENTS.md) — the lock and `RunState` carrying the socket path.
- [`../../dev/cmd/AGENTS.md`](../../dev/cmd/AGENTS.md) — the other client of this protocol.
