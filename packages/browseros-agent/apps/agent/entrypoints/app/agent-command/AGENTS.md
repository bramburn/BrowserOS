# `entrypoints/app/agent-command/` — home composer + harness agent chat

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

The alpha "Agent Command" surface. It is two routes served from the app shell:
`AgentCommandHome` at `/home` (a composer that routes a submission to either the
in-tab LLM chat or a named harness agent) and `AgentCommandConversation` at
`/home/agents/:agentId` (a full streaming chat against one agent, with a left
rail of sibling agents, a queued-message panel, and harness history restore).
Both are only mounted when `Feature.ALPHA_FEATURES_SUPPORT` is on — see
`app/App.tsx`.

`agent-command-layout.tsx` is the shared outlet context: it loads harness agents
once and hands `{ agents, agentsLoading }` to both routes via `useOutletContext`,
so neither page refetches the list.

## Contents

```
agent-command/
├── agent-command-layout.tsx    ← Outlet context: { agents, agentsLoading }
├── AgentCommandHome.tsx        ← `/home` — composer, agent card dock, recent agents
├── AgentCommandConversation.tsx← `/home/agents/:agentId` — chat screen controller
├── AgentChat.tsx               ← message thread for one agent
├── AgentChatMessage.tsx        ← single message renderer
├── ConversationHeader.tsx      ← chat-screen title bar
├── ConversationInput.tsx       ← composer (largest file here; attachments, voice)
├── ConversationMessage.tsx     ← message bubble wrapper
├── AgentRail.tsx               ← left rail of agents
├── AgentRailRow.tsx            ← single rail entry
├── AgentCardDock.tsx           ← horizontal card dock on home
├── HomeAgentCard.tsx           ← one home-screen agent card
├── QueuePanel.tsx              ← queued (not yet running) harness messages
├── useAgentConversation.ts     ← SSE turn lifecycle: send, cancel, reattach
├── useHarnessChatHistory.ts    ← paged harness history for one agent
├── agent-chat-types.ts         ← history ⇄ UI-message mapping helpers
├── agent-stream-events.ts      ← normalises tool status strings
├── harness-history-mapper.ts   ← harness page → chat history
├── home-agent-card.helpers.ts  ← home card ordering
├── home-compose.helpers.ts     ← pure router: home submit → /home/chat or /home/agents/:id
├── pending-initial-message.ts  ← module-scope, 10s-TTL handoff of text + attachments
└── *.test.ts                   ← bun tests for the pure helpers above
```

## Rules

- **AC1 — Handoffs between home and chat go through `pending-initial-message.ts`, not
  the URL.** Attachments are data URLs; `?q=` cannot carry them. Keep the 10s TTL
  and the destructive `consumePendingInitialMessage` read — StrictMode double-mount
  will double-send otherwise.
- **AC2 — `routeHomeSend` stays pure.** `home-compose.helpers.ts` decides *where* a
  submission goes and nothing else; the caller performs `setPendingInitialMessage`.
  Do not add storage or navigation side effects to it.
- **AC3 — List data comes from the outlet context.** `useAgentCommandData()` reads
  `useOutletContext`. Do not call `useHarnessAgents()` again inside a page here.
- **AC4 — Tool status strings are normalised once**, in `agent-stream-events.ts`.
  Anything that renders a tool badge must use `mapAgentHarnessToolStatus`, not its
  own `status === 'done'` checks.
- **AC5 — Sends go through `useAgentConversation`,** which owns the SSE stream
  (`consumeSSEStream`), reattach-to-active-turn, and cancel. Components call
  `send`/`stop`; they do not open `fetch` or `EventSource` themselves.
- **AC6 — Helpers with logic get a `*.test.ts` next to them.** The convention here is
  a sibling test, not a `__tests__/` directory. If you add branching to a helper,
  add a case.

## Workflows

**Routing a new home composer action to an agent**
1. Add the case in `home-compose.helpers.ts#routeHomeSend` (pure — return a `HomeSendRoute`).
2. Set the payload with `setPendingInitialMessage({ agentId, text, attachments })`.
3. Navigate to the `path` the helper returned.
4. Cover the branch in `home-compose.helpers.test.ts`.

**Adding a new pane to the agent conversation screen**
1. Add the component in this folder (PascalCase `.tsx`, co-located).
2. Render it from `AgentCommandConversation.tsx` or `AgentChat.tsx`.
3. Pull any new data from `useAgentConversation` or the outlet context — do not add a
   new `useQuery` for something the harness listing already carries.

**Adding a harness history field**
1. Extend the mapper in `agent-chat-types.ts` (or `harness-history-mapper.ts`).
2. Thread the field into the `AgentChatHistory*` types in the same file.
3. Update `useHarnessChatHistory.ts` only if the *query* changed.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table and provider order.
- [`../agents/AGENTS.md`](../agents/AGENTS.md) — `useAgents`, harness types, ordering.
- [`../../sidepanel/index/AGENTS.md`](../../sidepanel/index/AGENTS.md) — the other chat surface; reuses the same target model.
- [`../../../../lib/agent-conversations/`](../../../lib/agent-conversations/) — `AgentConversationTurn` / `ToolEntry` source types.
- [`../../../../lib/sse.ts`](../../../lib/sse.ts) — `consumeSSEStream`.
