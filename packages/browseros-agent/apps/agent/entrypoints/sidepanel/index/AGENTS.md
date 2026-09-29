# `entrypoints/sidepanel/index/` — the side-panel chat surface

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel`.

## What's here

The chat screen: message list, composer, tool-call rendering, provider/target
selection, and the hooks that actually own the conversation. `useChatSession.ts`
(≈22 KB) is the heart of the extension — it wraps `useChat` from `@ai-sdk/react`,
builds the request (LLM chat vs. harness agent chat), handles streaming, persistence
(local + remote), credits invalidation, stop signalling, and analytics.

`Chat.tsx` is the presentational shell: it reads everything from
`useChatSessionContext()` and delegates to `ChatMessages` / `ChatFooter`. Sibling
files handle one concern each — attached tabs, selected text, active-tab
notification, execution history, remote save, and the target model.

## Contents

```
index/
├── Chat.tsx                    ← the screen: input, attached tabs, voice, mode, errors
├── ChatMessages.tsx            ← scrolling message list
├── ChatMessageActions.tsx      ← per-message actions (copy, like, dislike)
├── ChatEmptyState.tsx          ← zero-message state + starter suggestions
├── ChatError.tsx               ← agent-URL and chat error banners
├── ChatHeader.tsx              ← provider/target picker, new conversation
├── ChatFooter.tsx              ← composer + mode toggle + stop + attached-tab strip
├── ChatInput.tsx               ← the text area / send control
├── ChatModeToggle.tsx          ← chat ⇄ agent
├── ChatAttachedTabs.tsx        ← attached-tab chips
├── ChatSelectedText.tsx        ← selected-text context chip
├── ConnectAppCard.tsx          ← prompt to connect an MCP app
├── ScheduleSuggestionCard.tsx  ← prompt to schedule the current work
├── GetActiveTabToolCall.tsx    ← renders the active-tab tool call
├── ToolBatch.tsx               ← groups parallel tool calls
├── UserActionMessage.tsx       ← non-user-authored system messages
├── JtbdPopup.tsx               ← survey popup
├── chatTypes.ts                ← ChatMode, Suggestion, CHAT_/AGENT_SUGGESTIONS
├── getMessageSegments.ts       ← message → renderable segments, tool states, nudges
├── useChatSession.ts           ← the session hook (transport, stream, persistence)
├── useChatSessionRequest.ts    ← builds { api, body } per target kind
├── useChatRefs.ts              ← imperative refs for scroll/textarea behaviour
├── useExecutionHistoryTracker.ts
├── useNotifyActiveTab.tsx      ← tells the active tab the agent is working
├── useRemoteConversationSave.ts← remote conversation persistence
├── sidepanel-chat-targets.ts   ← LLM vs ACP (harness agent) target model + selection
├── sidepanel-chat-targets.test.ts
├── useChatSession.test.ts
└── graphql/
    └── chatSessionDocument.ts  ← create/append conversation + message mutations
```

## Rules

- **CI1 — `useChatSession` is the only place that opens a stream.** Components call
  `sendMessage` / `stop` from `useChatSessionContext()`. No component may call
  `useChat`, `fetch`, or `EventSource` for chat.
- **CI2 — Two target kinds, two endpoints.** `useChatSessionRequest.ts` returns
  `{ api, body }`: ACP targets go to
  `<agentServerUrl>/agents/<id>/sidepanel/chat`; LLM providers go to
  `<agentServerUrl>/chat` via `buildChatRequestBody`. Add a target kind there, not in
  a component.
- **CI3 — `sidepanel-chat-targets.ts` is the target contract,** shared with
  `../../app/agent-command/AgentCommandHome.tsx` and `../../newtab`. Note it imports
  `isAdapterHidden` by **relative** path, not `@/` — that is deliberate so the module
  stays loadable under `bun test`. Keep the relative import.
- **CI4 — Selected text and active-tab notification are side effects with contracts.**
  `useNotifyActiveTab` writes to `selectedTextStorage`/`stopAgentStorage`; the content
  script in `../../selection.content.ts` and the Glow script read them. Change the
  shape in `@/lib/...` and update both ends.
- **CI5 — `getMessageSegments.ts` is pure and drives rendering.** Tool invocation
  state, nudges (`schedule_suggestion`, `app_connection`) and attachments are all
  derived there. Do not recompute them in a component.
- **CI6 — Analytics names are `SIDEPANEL_*`, `MESSAGE_*`, `PROVIDER_SELECTED_*`,
  `CONVERSATION_RESET_*`, `GLOW_STOP_*`.** Add constants in
  `@/lib/constants/analyticsEvents`; never reuse a `NEWTAB_*` name here.
- **CI7 — `ChatLayout` gates on a provider, so `Chat.tsx` may assume
  `selectedProvider` exists.** Don't re-add null guards for it.
- **CI8 — Local persistence is always mirrored remotely when signed in.**
  `useRemoteConversationSave` + `useConversations` handle that; removing either
  silently loses history on the GraphQL path.

## Workflows

**Adding a new chat target kind**
1. Extend `SidepanelChatTarget` in `sidepanel-chat-targets.ts`.
2. Add a branch in `buildSidepanelPreparedSendMessagesRequest` in
   `useChatSessionRequest.ts` returning the new `{ api, body }`.
3. Add the case in `toProviderOption` so the header can render it.
4. Add coverage in `sidepanel-chat-targets.test.ts`.

**Adding a message renderer**
1. Add a segment kind in `getMessageSegments.ts` (pure derivation).
2. Render it in `ChatMessages.tsx`.
3. Reuse primitives from `@/components/chat/`; do not re-implement markdown or
   tool-call chrome here.

**Changing persistence**
1. Local: `@/lib/conversations/conversationStorage` (`useConversations`).
2. Remote: the documents in `graphql/chatSessionDocument.ts` plus
   `useRemoteConversationSave.ts`.
3. After a change, verify both the signed-in and signed-out paths — `../history/`
   switches between them.

**Debugging a chat that does not start**
1. Check `selectedProvider` (the layout spinner means it is not resolved).
2. Check `hasSynced` from `useSyncRemoteIntegrations` — the session gates on it.
3. Check `agentUrlError` / `chatError` rendered by `ChatError.tsx`.
4. Inspect live: `bun scripts/dev/inspect-ui.ts open-sidepanel` then `snapshot`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — panel overview.
- [`../layout/AGENTS.md`](../layout/AGENTS.md) — `ChatSessionContext`, which exposes everything here.
- [`../history/AGENTS.md`](../history/AGENTS.md) — reads the conversations written here.
- [`./graphql/AGENTS.md`](./graphql/AGENTS.md) — the persistence documents.
- [`../../../../lib/messaging/server/buildChatRequestBody.ts`](../../../lib/messaging/server/buildChatRequestBody.ts) — the LLM request body.
- [`../../../../components/chat/`](../../../components/chat/) — shared chat components and `chatComponentTypes`.
