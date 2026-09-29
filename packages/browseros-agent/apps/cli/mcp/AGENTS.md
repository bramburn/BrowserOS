# `mcp/` — MCP client transport

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros-agent`,
> the Go CLI. This is the only place in the CLI that speaks the
> Model Context Protocol on the wire.

## What's here

A thin, stateless client for the BrowserOS MCP server.
`client.go` opens a `StreamableHTTP` session to `<BaseURL>/mcp` using
the official `github.com/modelcontextprotocol/go-sdk`, converts the SDK
result into a plain `ToolResult`, and adds two non-MCP REST helpers
(`/health`, `/status`). `types.go` holds the two small structs every
command depends on. There is no tool registry here — the CLI calls
whatever tool name it passes in.

## Contents

```
mcp/
├── client.go        ← Client, NewClient, connect, CallTool,
│                       convertResult, ResolvePageID, extractPageID,
│                       intValue, Health, Status, restGET,
│                       connectionSetupInstructions
├── types.go         ← ToolResult, ContentItem, TextContent(),
│                       ImageContent()
└── client_test.go   ← extractPageID table tests
```

## Rules

### MCP1 — Nothing else in the CLI imports the MCP SDK
`cmd/` calls `Client.CallTool` and handles `*mcp.ToolResult`. If a
command needs the SDK type `sdkmcp.CallToolResult`, the change belongs
in this package, not in `cmd/`. This is what keeps `cmd/` free of
protocol knowledge.

### MCP2 — The endpoint is `<BaseURL>/mcp`, with SSE disabled
`connect` builds `&sdkmcp.StreamableClientTransport{Endpoint: c.BaseURL + "/mcp",
DisableStandaloneSSE: true}`. `BaseURL` arrives already normalised —
`cmd/root.go` strips a trailing `/mcp` and `/` in `normalizeServerURL`.
Never append `/mcp` from a caller.

### MCP3 — One session per tool call
`CallTool` calls `connect`, then `defer session.Close()`, then
`session.CallTool`. It is deliberately not a long-lived session: the
CLI process is short and a fresh handshake costs one round trip. Only
change this with a measured reason, and keep the `Close` on every path.

### MCP4 — `IsError` returns both a result and an error
When the server sets `IsError`, `CallTool` returns
`(result, fmt.Errorf("%s", result.TextContent()))`. The result is
non-nil on purpose. Callers that immediately `output.Error(err, 1)`
and exit are correct as written; don't "fix" the double return.

### MCP5 — `StructuredContent` is `map[string]any`; coerce, don't assert
`output/printer.go` reads its keys with the `intVal` / `strVal` /
`boolVal` helpers in that file, because JSON numbers arrive as
`float64`. `extractPageID` handles both the flat `pageId` shape and the
nested `page.pageId` shape. Any new reader of structured content
should go through the same helpers, and any new shape should be added
to `client_test.go`'s table.

### MCP6 — Connection errors must end with `connectionSetupInstructions()`
`connectionSetupInstructions()` is appended to every "cannot connect"
error from `connect` and `restGET`. It is the CLI's only onboarding
surface: it tells the user to run `init`, `launch`, or `install`. A new
transport-level error path that bypasses it produces an unactionable
message — don't.

### MCP7 — `/health` and `/status` are plain REST GETs
`Health()` and `Status()` use `restGET`, not MCP. They decode into
`map[string]any` and fail on HTTP >= 400 with the response body
included. Don't route these through a tool call, and don't add new MCP
tools here — `/health` and `/status` are the only REST endpoints the
CLI needs.

### MCP8 — `Client.Debug` is currently write-only
`cmd/root.go` sets `c.Debug = debug`, but nothing in this package reads
it. If you add debug output, wire it here; otherwise don't assume
`--debug` produces MCP-level logging today.

## Workflows

### "Adding a typed helper for a tool result"
1. Add it to `client.go` as a method on `*Client` (or a package func if
   it needs no client), e.g. `func (c *Client) ActiveTitle() (string, error)`.
2. Reuse `CallTool`; don't open a second transport.
3. Return `(*mcp.ToolResult, error)` when the caller may need
   `StructuredContent`, and a plain value when it only needs text.
4. Add a table case to `client_test.go` for any new structured-content
   parsing.

### "A tool call returns nothing usable"
1. Run the same command with `--json` to see the raw
   `structuredContent` the server sent.
2. Check the shape against `convertResult` — non-`map[string]any`
   structured content is round-tripped through `json.Marshal`/`Unmarshal`
   and can be silently dropped if it isn't an object.
3. If the key name differs from what the command expects, fix the
   command's `output/printer.go` formatter rather than the transport.

### "Changing how the CLI connects"
1. `connect` is the single choke point — start there, not in `cmd/`.
2. Keep `connectionSetupInstructions()` attached to the new error path.
3. Integration coverage is the root `integration_test.go`
   (`-tags integration`, skipped when no server is reachable at
   `BROWSEROS_URL`, default `http://127.0.0.1:9105`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the Go CLI as a whole.
- [`../cmd/AGENTS.md`](../cmd/AGENTS.md) — the only caller.
- [`../output/AGENTS.md`](../output/AGENTS.md) — consumes `ToolResult`.
- [`../README.md`](../README.md) — architecture notes and the server-URL table.
- [`../../server/AGENTS.md`](../../server/AGENTS.md) — the server implementing these tools.
- [`../../../../docs/MCP_TOOL_SPEC.md`](../../../../../docs/MCP_TOOL_SPEC.md) — captured tool schemas.
