# `lib/mcp/` — MCP server state and JSON-RPC client

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Everything about the extension's view of MCP servers. `client.ts` is a
hand-rolled JSON-RPC 2.0 / MCP `2025-11-25` client (initialize, tools/list
with cursor pagination, notifications, and an optional server→client SSE
stream) used by the Connect Apps flow. `mcpServerStorage.ts` is the local
record of connected servers plus a small CRUD hook. `useSyncRemoteIntegrations`
reconciles that local list against the user's remote (Klavis) integrations,
which are keyed by email and therefore visible on other devices but missing
from this device's Chrome storage.

## Contents

```
mcp/
├── client.ts                ← McpTool; JSONRPC_VERSION '2.0',
│                              MCP_PROTOCOL_VERSION '2025-11-25',
│                              MCP_CLIENT_INFO {browseros-settings, 1.0.0};
│                              postJsonRpc / postJsonRpcNotification /
│                              openOptionalSseStream / normalizeTools;
│                              + client.test.ts
├── mcpServerStorage.ts      ← McpServer { id, displayName, type:
│                              'managed' | 'custom', managedServerName?,
│                              config? }, mcpServerStorage
│                              (local:mcpServers), useMcpServers()
│                              → { servers, addServer, removeServer }
└── useSyncRemoteIntegrations.ts
                              ← useSyncRemoteIntegrations(): fetches user
                              integrations + the server catalogue and writes
                              remote-but-missing ones into local storage;
                              returns { isSyncing, hasSynced }
```

## Rules

- **MCP1 — The MCP wire protocol lives only in `client.ts`.** Header names
  (`mcp-session-id`, `mcp-protocol-version`), the `accept: application/json,
  text/event-stream` negotiation, and cursor pagination are protocol
  concerns; don't reimplement them in a component or a new hook.
- **MCP2 — `MCP_PROTOCOL_VERSION` is a contract with the server.** Change it
  only alongside the server side, and re-run `client.test.ts`.
- **MCP3 — Server discovery responses are validated before use.**
  `normalizeTools` throws on a non-array `tools`; keep that guard rather than
  defaulting to `[]`.
- **MCP4 — `McpServer.type` is a discriminated union of `'managed' | 'custom'`.**
  Managed servers carry `managedServerName`; custom ones carry
  `config.url`. Gate rendering on `type`, never on which field happens to
  be set.
- **MCP5 — Consumers must gate on `hasSynced` before treating the list as
  complete.** `useSyncRemoteIntegrations` is async; rendering before
  `hasSynced` shows a partial list and lets the user "disconnect" a server
  that was simply not synced yet.
- **MCP6 — Session ids are echoed, never stored.** `client.ts` carries
  `mcp-session-id` through the request context; it does not persist it.

## Workflows

**Listing a server's tools**
1. `getMcpServerUrl()` from `../browseros/helpers` for the endpoint.
2. Call the client in `client.ts`; it performs initialize then tools/list.
3. Render `McpTool[]` — the icon comes from `../../assets/mcp-icons/`.

**Adding a new managed server to the UI**
1. Add its icon to `../../assets/mcp-icons/`.
2. Add the import/case in `entrypoints/app/connect-mcp/McpServerIcon.tsx`.
3. `addServer({ id, displayName, type: 'managed', managedServerName })`.

**Adding a synced-integration rule**
1. Extend `useSyncRemoteIntegrations.ts` — it is the only writer that merges
   remote state into `mcpServerStorage`.
2. Keep the `isSyncing` / `hasSynced` contract so consumers can gate.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3, LIB4).
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `getMcpServerUrl()` and the pref keys.
- [`../sse.ts`](../sse.ts) — the extension's own SSE client (distinct from MCP's optional stream here).
- [`../../assets/mcp-icons/AGENTS.md`](../../assets/mcp-icons/AGENTS.md) — server icons.
- [`../../entrypoints/app/connect-mcp/AGENTS.md`](../../entrypoints/app/connect-mcp/AGENTS.md) — the consuming route.
- [`client.test.ts`](client.test.ts) — protocol-level tests; update with the protocol.
