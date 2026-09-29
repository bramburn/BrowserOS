# `src/api/services/` — HTTP service layer

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/`.

## What's here

The behaviour behind the routes. `chat-service.ts` streams an AI-SDK UI
message response for `/chat`. `agents/agent-harness-service.ts` is the
largest file in the server (~27 KB) — it owns agent definitions, turn
lifecycle, and the ACPX runtime bridge. `mcp/` builds the MCP server object
and binds every registry tool to it. `klavis/` proxies the external Klavis
MCP service and caches its `createStrata` responses. Services are constructed
in `../server.ts` and passed to routes as `deps`, which is what keeps the
route handlers unit-testable.

## Contents

| File | Purpose |
|---|---|
| `chat-service.ts` | `ChatService` class + `ChatServiceDeps`; returns `createAgentUIStreamResponse` output. |
| `agents/agent-harness-service.ts` | `AgentHarnessService` — agent CRUD, `AgentLiveness` (`working`/`idle`/`asleep`/`error`), turn lifecycle listeners, `EnsureVmRuntimeReady`. Errors: `UnknownAgentError`, `InvalidAgentUpdateError`, `HermesProviderConfigInvalidError`, `TurnAlreadyActiveError`. |
| `mcp/mcp-server.ts` | `createMcpServer(deps)` — the `McpServer` instance with `MCP_INSTRUCTIONS` and a `SetLevelRequestSchema` no-op handler. |
| `mcp/register-mcp.ts` | `registerTools(mcpServer, registry, ctx)` — wraps each `ToolDefinition` in an MCP handler with timing, metrics, monitoring observer, and `windowId` defaulting. |
| `mcp/mcp-prompt.ts` | `MCP_INSTRUCTIONS` — the server's advertised capabilities text. |
| `klavis/strata-proxy.ts` | `connectKlavisProxy`, `connectKlavisInBackground`, `buildKlavisToolSet`, `registerKlavisTools`; `KlavisProxyHandle` / `KlavisProxyRef`. |
| `klavis/strata-cache.ts` | `KlavisStrataCache` + the `klavisStrataCache` singleton — in-process cache for Klavis `createStrata` so `/chat` does not block on a Worker-proxied call. |

## Rules

**SV1 — Services never read `process.env` or construct a Hono app.** They take
`deps` and return data. Anything that needs config is handed a value by
`../server.ts`.

**SV2 — `register-mcp.ts` is the single tool-execution seam.** Logging,
`performance.now()` timing, metrics, and the monitoring `onToolStart` /
`onToolEnd` observer all hang off this wrapper. A new execution path that
bypasses it is invisible to monitoring.

**SV3 — `windowId` injection is schema-driven, never an allowlist.**
`inputHasWindowIdField()` inspects the tool's `z.ZodObject` shape; any tool
with a `windowId` field participates automatically, and an explicit
`args.windowId` from the caller always wins over the
`X-BrowserOS-Default-Window-Id` default.

**SV4 — Klavis connects in the background and degrades to null.** `main.ts` /
`server.ts` call `connectKlavisInBackground(klavisRef, ...)` so browser tools
are available immediately; the ref starts as `{ handle: null }` and handlers
must tolerate that. `/shutdown` closes it if present.

**SV5 — Turn lifecycle is single-flight.** `AgentHarnessService` throws
`TurnAlreadyActiveError` rather than queueing a second concurrent turn;
`ActiveTurnRegistry` (in `../../lib/agents/`) backs that.

**SV6 — Cache the remote call, don't block on it.** Anything that reaches
Klavis during a chat turn goes through `strata-cache.ts` — the whole reason it
exists is that a blocking Worker-proxied `createStrata` stalled `/chat`.

## Workflows

**Adding an MCP tool wrapper behaviour:** change `mcp/register-mcp.ts` — it is
the only place tools are bound to the MCP transport, so timing, logging, and
observability stay consistent.

**Adding agent runtime readiness logic:** extend
`agents/agent-harness-service.ts` and its `EnsureVmRuntimeReady` callback;
`../server.ts` already wires the `hermes` case to
`ensureHermesRuntimeReady({ resourcesDir })`.

**Testing:** `tests/api/services/chat-service.test.ts`,
`tests/api/services/agents/agent-harness-service.test.ts`,
`tests/api/services/klavis/strata-cache.test.ts`, and
`strata-proxy.test.ts` cover this folder.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/` conventions.
- [`../routes/AGENTS.md`](../routes/AGENTS.md) — the route modules that call these.
- [`../../lib/agents/AGENTS.md`](../../lib/agents/AGENTS.md) — ACPX runtime, agent store, turn registry.
- [`../../../../tests/api/services/AGENTS.md`](../../../tests/api/services/AGENTS.md) — service tests.
