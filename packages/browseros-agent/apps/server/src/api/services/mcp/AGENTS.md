# `src/api/services/mcp/` — MCP server construction

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/`.

## What's here

Three files that turn the `ToolRegistry` into a live MCP server. The MCP
endpoint itself is `POST /mcp` in [`../../routes/mcp.ts`](../../routes/mcp.ts)
over the `StreamableHTTPTransport` from `@hono/mcp`; this folder builds the
`McpServer` object that transport serves.

## Contents

| File | Purpose |
|---|---|
| `mcp-server.ts` | `createMcpServer(deps: McpServiceDeps): McpServer`. Creates the server (`name: 'browseros_mcp'`, title `BrowserOS MCP server`) with `capabilities: { logging: {} }` and `instructions: MCP_INSTRUCTIONS`; registers a no-op `SetLevelRequestSchema` handler; then calls `registerTools` and the Klavis registration. |
| `register-mcp.ts` | `registerTools(mcpServer, registry, ctx)` — the per-tool wrapper: arg logging, timing, metrics, monitoring observer, `windowId` defaulting, `executeTool()` dispatch. The catch block records a failed call via `metrics.log` + `ctx.observer?.onToolEnd` only — there is no Sentry call here. |
| `mcp-prompt.ts` | `MCP_INSTRUCTIONS` — the instructions string advertised to MCP clients ("Browser automation and 40+ external service integrations"). |

## Contents (data flow)

`routes/mcp.ts` → `createMcpServer()` → `registerTools(server, registry, …)` →
for each `ToolDefinition` an MCP `tool()` whose handler calls
`executeTool()` from `../../../tools/framework.ts` → `Browser` → CDP.

## Rules

**MCP1 — Registry is the only source of tools.** `registerTools` iterates
`registry.all()`. It never imports individual tool files, and adding a tool
happens in `../../../tools/registry.ts` alone.

**MCP2 — `windowId` defaulting is schema-driven.** `inputHasWindowIdField()`
checks `z.ZodObject` for a `windowId` key; injection happens only when the
host sent `X-BrowserOS-Default-Window-Id`, the tool accepts one, and the
caller did not set one explicitly. Do not add a per-tool allowlist.

**MCP3 — Every tool call is observed.** `ctx.observer?.onToolStart` /
`onToolEnd` feed `../../../monitoring/`. Dropping them silently disables
monitoring for that path.

**MCP4 — Errors are reported per-tool, not per-server.** A handler failure
becomes an error tool result with the tool name in the message; it must not
take down the transport.

**MCP5 — Update `MCP_INSTRUCTIONS` when the tool surface changes.** It is the
only description an MCP client sees before listing tools, and it currently
claims "40+ external service integrations" (Klavis), which is a different
number from the 61 browser tools in the registry — keep the two claims
honest and distinct.

## Workflows

**Adding an MCP tool:** 1. Create the `ToolDefinition` in
`../../../tools/`. 2. Register it in `../../../tools/registry.ts`. 3. Nothing
to change here — `registerTools` picks it up. 4. If the client-facing
description is now wrong, edit `mcp-prompt.ts`.

**Tracing a tool call end to end:** `routes/mcp.ts` (scope id, default window
id) → `mcp-server.ts` (server construction) → `register-mcp.ts` (wrapper) →
`framework.ts:executeTool` → the tool handler.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/services/` conventions.
- [`../../../tools/AGENTS.md`](../../../tools/AGENTS.md) — the canonical registry and tool contract.
- [`../../../../../../../../docs/MCP_TOOL_SPEC.md`](../../../../../../../../docs/MCP_TOOL_SPEC.md) — captured schemas from the *port 9200* server, for naming/protocol comparison only.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
