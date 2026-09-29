# `apps/cli/` — Go CLI (`browseros-cli`)

> Sub-package AGENTS file. The Go CLI for power users. For the
> cross-repo architecture see
> [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).

## What's here

A Go-based CLI that drives BrowserOS through the MCP server. It is a
standalone Go module (`module browseros-cli`, `go 1.25.7`) with **no
`package.json`**, so it is *not* a Bun workspace member even though the
root `workspaces` field globs `apps/*` — the root scripts address it by
path, not by filter. The npm distribution wrapper lives one level deeper
in `npm/`, which the glob also does not reach.

The server URL is **never hard-coded**. It comes from `--server/-s`, the
`BROWSEROS_URL` env var, or `config.yaml` (`server_url`) in the user's
config dir; a bare port like `9100` is normalised to
`http://127.0.0.1:<port>`. Only the integration test has a default
(`http://127.0.0.1:9105`).

```
apps/cli/
├── main.go                         ← entry point; sets version, calls cmd.Execute()
├── go.mod, go.sum                  ← module browseros-cli (go 1.25.7)
├── Makefile                        ← default `browseros-cli`, install, clean, vet,
│                                     test, release, npm-version, npm-publish
├── README.md
├── CHANGELOG.md
├── cmd/                            ← 28 command files + 3 test files
├── config/                         ← user/server config (config.yaml)
├── mcp/                            ← MCP StreamableHTTP client
├── analytics/                      ← PostHog telemetry
├── output/                         ← printer.go / JSON + exit codes
├── update/                         ← self-update (minio/selfupdate)
├── npm/                            ← npm packaging wrapper (bin/, scripts/)
├── scripts/
├── integration_test.go             ← `//go:build integration`
└── ...
```

## Opinionated rules

### C1 — Cobra for subcommands
`cmd/root.go` builds the root command; every other file in `cmd/` is a
self-registering `*cobra.Command` added via `init()` +
`rootCmd.AddCommand(...)`. Don't sprinkle flag parsing throughout — keep
it in the cmd file.

### C2 — The server URL is user-supplied, never a constant
`cmd/root.go` resolves it: `defaultServerURL()` reads `BROWSEROS_URL`,
then `config.Load()`; `normalizeServerURL` trims a trailing `/mcp` and
`/`, and expands a bare port to `http://127.0.0.1:<port>`. If nothing is
configured, `validateServerURL` errors with setup instructions rather
than guessing a port. Never introduce a hard-coded port here.

### C3 — npm packaging lives in `npm/`
`npm/package.json` wraps the Go binary for npm distribution
(`npm/bin/browseros-cli.js`, `npm/scripts/postinstall.js`). The release
target builds it via `make npm-version` / `make npm-publish`. Keep it
thin; no CLI logic belongs in there.

### C4 — Integration tests live at the package root
`integration_test.go` is `package main` behind `//go:build integration`.
It targets `$BROWSEROS_URL`, defaulting to `http://127.0.0.1:9105`, and
**exits 0 early** when `<url>/health` is unreachable, so `make test` is
safe to run without a server. Per-package tests live in their
sub-package directories (`cmd/root_test.go`, `mcp/client_test.go`, …).

### C5 — Help is grouped, and the group is an annotation
`cmd/root.go` sorts the help output into six buckets via
`Annotations["group"]`: `Navigate:`, `Observe:`, `Input:`, `Resources:`,
`Integrations:`, `Setup:`. A command with no annotation silently falls
into `Setup:`. Set it on every new command.

## Opinionated workflow

### "Add a new CLI subcommand"
1. Create `cmd/<sub>.go` with a self-registering cobra command.
2. Register it with `rootCmd.AddCommand(cmd)` inside that file's `init()`.
3. Add the `Annotations["group"]` value (see C5) and the MCP call.
4. Test in `cmd/<sub>_test.go` or `integration_test.go`.

### "Add a new MCP tool wrapper"
The CLI mostly delegates to MCP. The client entry point is
`(*mcp.Client).CallTool`:

```go
result, err := c.CallTool("<tool_name>", map[string]any{ /* args */ })
```

It takes no `context.Context` — it derives one internally from the
`--timeout` value. It returns `(*mcp.ToolResult, error)`. There is no
`mcp.Client.Call`; see [`mcp/AGENTS.md`](mcp/AGENTS.md) for the full
transport contract. Prefer `c.CallTool(...)` at the call site over
adding a new wrapper method in `mcp/`.

### "Build the CLI"
```bash
# default target — writes ./browseros-cli
make

# or, to put it on $GOBIN
make install

# or directly
go build -o browseros-cli .
```

The binary is `browseros-cli` and lands in the package root — there is
no `bin/` directory and no `make build` target. Cross-compile with
`GOOS=linux GOARCH=amd64 go build -o browseros-cli .`, or use
`make release VERSION=<x.y.z>` to build the six-platform matrix
(darwin/linux/windows × amd64/arm64) into `dist/` with checksums.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../AGENTS.md`](../AGENTS.md) — the other three apps.
- [`cmd/AGENTS.md`](cmd/AGENTS.md) — the cobra command surface.
- [`mcp/AGENTS.md`](mcp/AGENTS.md) — the MCP client transport.
- [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md) — overall map.
- [`../../../../docs/MCP_TOOL_SPEC.md`](../../../../docs/MCP_TOOL_SPEC.md) — captured tool schemas.
