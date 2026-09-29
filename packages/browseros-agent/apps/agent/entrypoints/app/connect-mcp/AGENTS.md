# `entrypoints/app/connect-mcp/` — connect apps (`/connect-apps`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The "connect apps" page — where users add MCP servers, either **managed** (a
catalogue served by the local agent server at `<baseUrl>/klavis/servers`) or
**custom** (hand-entered transport + URL). Managed servers can require an OAuth URL
or an API-key URL; the page branches into `AddManagedMCPDialog` or `ApiKeyDialog`
accordingly.

Four small `useSWRMutation`/SWR hooks own the HTTP calls. This folder is also the
single source of `McpServerIcon` and `useGetUserMCPIntegrations`, which the new-tab
page and the onboarding demo both import.

## Contents

```
connect-mcp/
├── ConnectMCP.tsx             ← page container; add/remove flows, error toasts + Sentry
├── AvailableManagedServers.tsx← catalogue grid
├── AddManagedMCPDialog.tsx    ← add a catalogue server
├── AddCustomMCPDialog.tsx     ← add a custom server
├── ApiKeyDialog.tsx           ← paste-an-API-key step
├── McpServerIcon.tsx          ← icon resolution for a server (imported elsewhere too)
├── useGetMCPServersList.tsx   ← SWR: GET <baseUrl>/klavis/servers
├── useGetUserMCPIntegrations.tsx ← SWR: user's connected integrations
├── useAddManagedServer.tsx    ← SWR mutation: POST <baseUrl>/klavis/servers/add
├── useRemoveManagedServer.tsx ← SWR mutation: remove a managed server
└── useSubmitApiKey.tsx        ← SWR mutation: submit an API key for a server
```

## Rules

- **CM1 — Every request URL is built from `useAgentServerUrl()`.** Pass
  `agentServerUrl ? \`${agentServerUrl}/klavis/...\` : null` as the SWR key so the
  hook stays disabled until the port is resolved.
- **CM2 — Hooks own transport; the page owns flow.** `ConnectMCP.tsx` sequences
  dialogs and toasts; the `use*.tsx` files do one request each. Do not inline
  `fetch` into the page.
- **CM3 — Failures are reported twice: toast + Sentry.** `failedToAddMcp` /
  `failedToRemoveMcp` are the house pattern. Keep both.
- **CM4 — After a successful add/remove, revalidate the integrations list.** Use the
  SWR `mutate` the page already destructures; a stale grid is the common bug here.
- **CM5 — `useSyncRemoteIntegrations()` is called once by the page** and hands its
  `hasSynced` flag to `useChatSession` via
  `../../sidepanel/layout/ChatSessionContext`. Removing that call breaks chat start-up
  gating.
- **CM6 — This folder is imported from outside `app/`.** `newtab/index/NewTab.tsx`,
  `onboarding/demo/OnboardingDemo.tsx` and `ai-settings/McpPromoBanner` all reach in
  for `McpServerIcon` / `useGetUserMCPIntegrations`. Keep those exports stable and
  dependency-light.

## Workflows

**Adding a new managed-server auth flow**
1. Add the response field to the local server's `/klavis/servers/add` payload.
2. Extend `AddServerResponse` in `useAddManagedServer.tsx`.
3. In `ConnectMCP.tsx`, branch the response: `oauthUrl` → open it, `apiKeyUrl` →
   `setApiKeyServer(...)` to open `ApiKeyDialog`.
4. Track with a new constant in `@/lib/constants/analyticsEvents`.

**Adding a custom-server transport type**
1. Extend the transport union the dialog writes into `useMcpServers()`
   (`@/lib/mcp/mcpServerStorage`).
2. Update `AddCustomMCPDialog.tsx` validation.
3. Add the icon case in `McpServerIcon.tsx` (or the asset in `apps/agent/assets/mcp-icons/`).

**Debugging an empty catalogue**
1. Check `useAgentServerUrl()` resolved — the port lives in the live `proxy_port` pref.
2. `bun scripts/dev/inspect-ui.ts eval app "chrome.storage.local.get(null)"`.
3. Remember the SWR key is `null` until the base URL resolves, so "no data" can mean
   "no base URL" rather than "request failed".

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../mcp-settings/AGENTS.md`](../mcp-settings/AGENTS.md) — the *local* MCP server settings (different thing).
- [`../../../../lib/mcp/`](../../../lib/mcp/) — `useMcpServers`, `useSyncRemoteIntegrations`, `client.ts`.
- [`../../../../assets/mcp-icons/`](../../../assets/mcp-icons/) — server icons.
- [`../../../newtab/index/AGENTS.md`](../../newtab/index/AGENTS.md) — another consumer of this folder.
