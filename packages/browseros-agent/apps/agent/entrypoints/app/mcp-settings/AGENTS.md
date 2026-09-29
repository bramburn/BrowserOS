# `entrypoints/app/mcp-settings/` — local MCP server settings (`/settings/mcp`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

Configuration and diagnostics for the **local** BrowserOS MCP server — the one on
`127.0.0.1` whose port lives in a live browser pref. This is a different thing from
`../connect-mcp`, which manages third-party MCP servers the user connects.

Three sections: the server header (URL + health + restart), a "quick setup" block of
copy-paste commands for external MCP clients (Claude Code, Gemini CLI, Codex, Claude
Desktop), and the live tool list fetched from the server.

## Contents

```
mcp-settings/
├── MCPSettingsPage.tsx           ← page; loads URL then tools, owns the loading/error split
├── MCPServerHeader.tsx           ← server URL, health state, restart action
├── ServerSettingsCard.tsx        ← connection settings
├── ServerPortEditor.tsx          ← edit the server port/URL
├── server-port-editor.helpers.ts ← pure port/URL parsing + validation
├── server-port-editor.test.ts
├── server-health.ts              ← waitForServerHealth(): poll checkHealth, 60s cap
├── QuickSetupSection.tsx         ← copy-paste `mcp add` commands per client
└── MCPToolsSection.tsx           ← tool list + manual refresh
```

## Rules

- **MS1 — Health and tool fetches go through the service worker**, via
  `sendServerMessage('checkHealth' | 'fetchMcpTools', undefined)`. The handlers live in
  `../../../background/index.ts`. Calling the local port directly from the page breaks
  as soon as the port pref changes.
- **MS2 — `waitForServerHealth` polls, it does not block.** 60 s timeout, 2 s interval
  (`HEALTH_CHECK_TIMEOUT_MS` / `HEALTH_CHECK_INTERVAL_MS`). Extend the constants in
  `server-health.ts`, not at the call site.
- **MS3 — Port validation is pure and tested.** `server-port-editor.helpers.ts` holds
  the parsing/validation; add cases to `server-port-editor.test.ts` rather than
  inlining checks in the editor.
- **MS4 — Loading and error state are tracked separately for URL and tools.**
  `MCPSettingsPage` keeps `urlLoading`/`urlError` apart from `toolsLoading`/`toolsError`
  on purpose: the URL comes from a pref, the tools come from the network.
- **MS5 — `getMcpServerUrl()` is the single source of the URL.** It resolves from the
  live `proxy_port` pref, so it is correct after a server restart. Do not cache the URL
  in component state beyond the current load.
- **MS6 — New client snippets go in `QuickSetupSection`'s `clients` array.** One entry
  per tool, each with a `getSnippet(url)` function, so a port change propagates.

## Workflows

**Adding an external MCP client snippet**
1. Add a `ClientConfig` entry to `clients` in `QuickSetupSection.tsx` with `id`, `name`,
   `type` and `getSnippet`.
2. The `type: 'command' | 'json'` split drives the copy button and the file name; set
   `fileName` for JSON configs.
3. No other change needed — the URL argument is supplied at render time.

**Changing how the port is validated or normalised**
1. Edit `server-port-editor.helpers.ts`.
2. Add cases to `server-port-editor.test.ts`.
3. Confirm `ServerPortEditor.tsx` still uses the helper (not an inline regex).

**Debugging "server unreachable"**
1. Read the resolved URL: `bun scripts/dev/inspect-ui.ts eval app "..."` or check the
   `proxy_port` pref.
2. Poll from the worker (`checkHealth` handler) — a page-side fetch will be blocked or
   hit the wrong port.
3. Check `waitForServerHealth` timing: 60 s is the cap, not the expectation.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../connect-mcp/AGENTS.md`](../connect-mcp/AGENTS.md) — third-party MCP servers (adjacent but separate).
- [`../../../background/AGENTS.md`](../../background/AGENTS.md) — the `checkHealth` / `fetchMcpTools` handlers.
- [`../../../../lib/messaging/server/serverMessages.ts`](../../../lib/messaging/server/serverMessages.ts) — `sendServerMessage`.
- [`../../../../lib/browseros/helpers.ts`](../../../lib/browseros/helpers.ts) — `getMcpServerUrl`, `getHealthCheckUrl`.
