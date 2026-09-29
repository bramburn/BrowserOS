# `lib/messaging/server/` — Server-health channel and chat body builder

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/messaging`.

## What's here

Two files with different jobs. `serverMessages.ts` is a typed extension
message channel letting a UI surface ask the background worker about the
local server (`checkHealth`, `fetchMcpTools`) — used because content-facing
pages can't reach `127.0.0.1` reliably on their own.
`buildChatRequestBody.ts` is a **pure** function that assembles the JSON
body for the chat endpoint, factored out precisely so it can be unit
tested without a network.

## Contents

```
server/
├── serverMessages.ts          ← ServerMessagesProtocol:
│                                 checkHealth(): { healthy: boolean }
│                                 fetchMcpTools(): { tools: McpTool[], error? }
│                                 exports: sendServerMessage / onServerMessage
├── buildChatRequestBody.ts    ← buildChatRequestBody({ conversationId,
│                                 provider, message, mode, browserContext,
│                                 userSystemPrompt, userWorkingDir,
│                                 supportsImages, previousConversation,
│                                 declinedApps, selectedText,
│                                 selectedTextSource, isScheduledTask });
│                                 also exports ChatHistoryEntry and
│                                 ChatRequestBrowserContext
└── buildChatRequestBody.test.ts
```

## Rules

- **SRVM1 — `buildChatRequestBody` performs no I/O.** It is a pure mapper;
  the caller supplies the `agentServerUrl` and does the `fetch`. That purity
  is what makes the test meaningful.
- **SRVM2 — `ChatRequestBrowserContext` is the browser-state contract.** New
  browser context the agent needs (window id, active tab, selected tabs,
  enabled/custom MCP servers) goes in this interface, not in an ad-hoc param.
- **SRVM3 — `previousConversation` is `ChatHistoryEntry[] | string` for a
  reason.** Old servers accept only a string; the array form is gated by
  `Feature.PREVIOUS_CONVERSATION_ARRAY` in `../../browseros/capabilities.ts`.
  Don't collapse the union.
- **SRVM4 — `fetchMcpTools` returns the `McpTool` type from
  `../../mcp/client.ts`.** Import it; don't re-declare `{ name, description? }`.
- **SRVM5 — Both messages are synchronous typed returns, not promises in the
  protocol map.** `defineExtensionMessaging` handles the transport; the map
  describes the shape only.

## Workflows

**Adding a field to the chat request**
1. Extend `ChatRequestBodyParams` (and `ChatRequestBrowserContext` if it is
   browser state) in `buildChatRequestBody.ts`.
2. Add the mapping in the returned object.
3. Extend `buildChatRequestBody.test.ts` with the new case.
4. Check the server accepts it; gate old servers with a `Feature` if not.

**Checking server health from a page**
1. `const { healthy } = await sendServerMessage.checkHealth()`
2. The `onMessage` handler that pings `/health` lives in the background
   entrypoint; `../../browseros/helpers.ts` has `getHealthCheckUrl()`.

**Fetching tools for a server**
1. `const { tools, error } = await sendServerMessage.fetchMcpTools()`
2. Render `tools`; surface `error` when present.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — messaging rules (MSG1–MSG5).
- [`../../mcp/AGENTS.md`](../../mcp/AGENTS.md) — `McpTool`.
- [`../../browseros/AGENTS.md`](../../browseros/AGENTS.md) — `getHealthCheckUrl()`, `Feature.PREVIOUS_CONVERSATION_ARRAY`.
- [`../../conversations/AGENTS.md`](../../conversations/AGENTS.md) — `formatConversationHistory`, which produces the array form.
- [`buildChatRequestBody.test.ts`](buildChatRequestBody.test.ts) — the colocated test.
