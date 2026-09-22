# `apps/cli/` — Go CLI (`browseros` v2+)

> Sub-package AGENTS file. The Go CLI for power users. For the
> cross-repo architecture see
> [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).

## What's here

A Go-based CLI that drives BrowserOS via the MCP server. Different
from the legacy Python `browser-cli` wrapper
(`C:/dev/browser-cli/`, separate repo) which talks to the bundled
port-9200 MCP server. This CLI targets the modern port-9100 server.

```
apps/cli/
├── main.go                         ← cobra-style entry
├── go.mod, go.sum
├── Makefile                        ← `make build`, `make test`, etc.
├── README.md
├── CHANGELOG.md
├── cmd/                            ← subcommands
├── config/                         ← user/server config
├── mcp/                            ← MCP stdio client glue
├── analytics/
├── output/
├── npm/                            ← node-side glue (?)
├── scripts/
├── integration_test.go
└── ...
```

## Opinionated rules

### C1 — Cobra for subcommands
The CLI uses cobra (or similar). Subcommands in `cmd/<sub>.go`. Don't
sprinkle flag parsing throughout — keep it in the cmd file.

### C2 — Server protocol is port 9100 (Bun), not 9200 (bundled)
This CLI targets the modern MCP server. The 9200 protocol (bundled,
precompiled) is what `C:/dev/browser-cli/` (Python) talks to — it's
a different code path. Don't conflate them.

### C3 — Bun scripts live in `npm/` (?)
If there's any TypeScript glue (e.g. for npm publish), it lives in
`npm/`. Keep it minimal.

### C4 — Integration tests live at the package root
`integration_test.go` is the integration test entry. Per-package tests
in their sub-package directories.

## Opinionated workflow

### "Add a new CLI subcommand"
1. Create `cmd/<sub>.go` with the cobra command struct.
2. Register in `cmd/root.go` (or wherever `main.go` aggregates).
3. Add any HTTP/MCP calls via `mcp/<service>.go`.
4. Test in `cmd/<sub>_test.go` or `integration_test.go`.

### "Add a new MCP tool wrapper"
The CLI mostly delegates to MCP. If you need a new wrapper:
1. Add to `mcp/<tool>.go`.
2. The CLI command calls it via `mcp.Client.Call(ctx, "<tool>", args)`.

### "Build the CLI"
```bash
make build
# or
go build -o bin/browseros .
```

The output binary lands in `bin/browseros`. Cross-compile via
`GOOS=linux GOARCH=amd64 go build` etc.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — overall map.
- [`../../../docs/MCP_TOOL_SPEC.md`](../../../docs/MCP_TOOL_SPEC.md) — captured tool schemas.
