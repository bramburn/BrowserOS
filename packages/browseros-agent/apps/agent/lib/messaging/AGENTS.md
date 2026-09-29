# `lib/messaging/` — Extension-internal message protocols

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

Typed, one-way channels between extension contexts (background service
worker ↔ sidepanel ↔ newtab), built on
[`@webext-core/messaging`](https://github.com/ZeroSocket/wxt-messaging)'s
`defineExtensionMessaging<Protocol>()`. Each subfolder declares exactly one
protocol with an explicit request/response type map and re-exports
`sendMessage` / `onMessage` under a domain-specific alias, so a sender in
the newtab and a listener in the sidepanel can never disagree on a payload
shape.

## Contents

```
messaging/
├── schedules/scheduleMessages.ts          ← runScheduledJob({ jobId }) and
│                                             cancelScheduledJobRun({ runId }) →
│                                             { success, error? }
│                                             exports: sendScheduleMessage /
│                                             onScheduleMessage
├── server/serverMessages.ts               ← checkHealth() → { healthy },
│                                             fetchMcpTools() → { tools, error? }
│                                             exports: sendServerMessage /
│                                             onServerMessage
├── server/buildChatRequestBody.ts         ← pure builder for the chat POST body
│                                             (conversationId, provider, mode,
│                                             browserContext, previousConversation,
│                                             declinedApps, selectedText, …)
│                                             + buildChatRequestBody.test.ts
└── sidepanel/openSidepanelWithSearch.ts   ← open(SearchActionStorage) —
│                                             carries a newtab search/action
│                                             payload into the sidepanel
│                                             exports: openSidePanelWithSearch /
│                                             onOpenSidePanelWithSearch
```

## Rules

- **MSG1 — One protocol per subfolder.** Never add a second
  `defineExtensionMessaging` to an existing file; a new channel gets its own
  folder with its own request/response map.
- **MSG2 — Sender and receiver share the declared types.** Import the payload
  type from the feature (`SearchActionStorage`, `McpTool`,
  `ScheduledJob`) rather than redeclaring it in the protocol map.
- **MSG3 — `buildChatRequestBody` is pure and separately tested.** It is the
  single definition of what the chat endpoint receives; changing its shape
  means updating `buildChatRequestBody.test.ts` in the same change. Note the
  `previousConversation` field is typed `ChatHistoryEntry[] | string` because
  older servers only accept a string (see
  `Feature.PREVIOUS_CONVERSATION_ARRAY`).
- **MSG4 — Use the aliased exports** (`sendScheduleMessage`, not the raw
  `sendMessage`) so the call site names the channel.
- **MSG5 — Background listeners are registered in `entrypoints/background/`.**
  This folder only declares the protocol; the sidepanel and newtab register
  the `onMessage` halves.

## Workflows

**Adding a new background message**
1. Create `lib/messaging/<domain>/<domain>Messages.ts` with the
   `defineExtensionMessaging<Protocol>()` type map.
2. Register the `onMessage` handler in `entrypoints/background/`.
3. Call the aliased `send<Domain>Message` from the sending surface.
4. Reuse existing payload types from the owning `lib/<feature>/`.

**Changing the chat request body**
1. Edit `server/buildChatRequestBody.ts` (keep it a pure function — no fetch).
2. Update `server/buildChatRequestBody.test.ts`.
3. Check server-side compatibility; add a `Feature` gate in
   `../browseros/capabilities.ts` if older servers must keep working.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB1, LIB3).
- [`../schedules/AGENTS.md`](../schedules/AGENTS.md) — the jobs these messages run.
- [`../mcp/AGENTS.md`](../mcp/AGENTS.md) — `McpTool` returned by `fetchMcpTools`.
- [`../search-actions/AGENTS.md`](../search-actions/AGENTS.md) — `SearchActionStorage` handed to the sidepanel.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — where the `onMessage` handlers live.
