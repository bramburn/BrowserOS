# `lib/conversations/` — Local conversation history and cloud sync

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The persisted chat history. `conversationStorage.ts` holds up to 50
conversations in `chrome.storage.local` as AI-SDK `UIMessage[]`, and
auto-uploads them to the cloud when a user is signed in.
`formatConversationHistory.ts` compresses a UI message list into the plain
`{ role, content }[]` shape the request body accepts, and
`graphql/uploadConversationDocument.ts` holds the operations that make the
upload idempotent (exists-check, count-check, bulk create).

## Contents

```
conversations/
├── conversationStorage.ts             ← Conversation { id, messages,
│                                         lastMessagedAt }; conversationStorage
│                                         (local:conversations, MAX 50);
│                                         useConversations() → save/remove;
│                                         removing a conversation also
│                                         clears its execution history
├── formatConversationHistory.ts       ← UIMessage[] → { role, content }[];
│                                         keeps the last 10 messages, joins
│                                         text parts, truncates at 64 KiB
├── uploadConversationsToGraphql.ts    ← resolves profileId by userId, then per
│                                         conversation: ConversationExists →
│                                         GetUploadedMessageCount →
│                                         Create → BulkCreateConversationMessages
└── graphql/uploadConversationDocument.ts
                                         ← GetProfileIdByUserId,
                                           CreateConversationForUpload,
                                           BulkCreateConversationMessages,
                                           ConversationExists,
                                           GetUploadedMessageCount
```

## Rules

- **CONV1 — History is a capped list, not a growing one.** `MAX_CONVERSATIONS`
  is 50; pruning happens on write. Raising it changes `chrome.storage.local`
  pressure — revisit the upload loop at the same time.
- **CONV2 — The upload is incremental and idempotent.** It compares the
  remote `totalCount` against `conversation.messages.length` and skips
  already-uploaded conversations. Do not replace it with a blind
  "delete and re-create".
- **CONV3 — Uploads are gated on a signed-in user.** `uploadConversationsToGraphql`
  returns early without `sessionInfo.user.id` or a resolvable `profileId`.
- **CONV4 — `removeConversation` must also clear execution history.** It calls
  `removeConversationExecutionHistory` from `../execution-history/storage` —
  keep that coupling when adding removals elsewhere.
- **CONV5 — `formatConversationHistory` bounds both ends.** Ten messages,
  65,536 characters per message, `[truncated]` marker. The request body
  contract in `../messaging/server/buildChatRequestBody.ts` depends on the
  exact shape; don't widen the caps casually.
- **CONV6 — This is not where the AI-SDK types end.** New per-message parts
  need handling in `formatConversationHistory` or they silently vanish from
  `previousConversation`.

## Workflows

**Adding a new message part to the history payload**
1. Update `formatConversationHistory.ts` to extract the part's text.
2. Confirm the backend accepts it in the `previousConversation` entry.
3. Re-check `Feature.PREVIOUS_CONVERSATION_ARRAY` gating in
   `../browseros/capabilities.ts` for older servers.

**Deleting a conversation everywhere**
1. `removeConversation(id)` from `useConversations()`.
2. It already chains into the execution-history store.
3. Remote rows are handled by the backend; the client does not issue a
   conversation delete.

**Adding a new upload field**
1. Add it to the selection sets in `graphql/uploadConversationDocument.ts`.
2. Update `schema/schema.graphql` and run `bun run codegen` first.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`graphql/AGENTS.md`](graphql/AGENTS.md) — the documents in this feature.
- [`../execution-history/AGENTS.md`](../execution-history/AGENTS.md) — history cleared on removal.
- [`../auth/AGENTS.md`](../auth/AGENTS.md) — `sessionStorage`, the upload gate.
- [`../messaging/server/AGENTS.md`](../messaging/server/AGENTS.md) — consumes `ChatHistoryEntry[]`.
- [`../agent-conversations/AGENTS.md`](../agent-conversations/AGENTS.md) — the separate IndexedDB transcript store.
