# `entrypoints/sidepanel/history/` — conversation history

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel`.

## What's here

The `/history` route of the side panel. `ChatHistory.tsx` is a thin switch: signed in
→ `RemoteChatHistory` (GraphQL, cursor-paginated, deletable); signed out →
`LocalChatHistory` (extension storage, no pagination). Both paths converge on the
same `ConversationList` component, so grouping, time labels, and empty states are
shared.

Active-conversation highlighting comes from `useChatSessionContext()` — the list
reads `conversationId` from the session rather than tracking its own selection.

## Contents

```
history/
├── ChatHistory.tsx          ← signed-in ? RemoteChatHistory : LocalChatHistory
├── components/              ← list, group, item, types, grouping utils
├── graphql/                 ← GetConversationsForHistory + DeleteConversation
└── local/                   ← LocalChatHistory (storage-backed)
```

## Rules

- **HS1 — Two sources, one list.** Both branches must map into
  `HistoryConversation` (`{ id, lastMessagedAt, lastUserMessage }`) and call
  `groupConversations` before rendering. `ConversationList` knows nothing about
  storage or GraphQL.
- **HS2 — Pagination is remote-only.** `hasNextPage` / `onLoadMore` / `isRefreshing`
  are optional props; the local path omits them and infinite scroll never arms.
- **HS3 — Deletion invalidates the list query** with
  `getQueryKeyFromDocument(GetConversationsForHistoryDocument)`, and the local path
  calls `removeConversation`. Both must exist for a delete to look immediate.
- **HS4 — `lastMessagedAt` may lack a trailing `Z`.** The remote path appends one
  before `new Date(...)`; the naive parse shifts the timestamp into the wrong time
  group. Keep that normalisation.
- **HS5 — Time grouping is pure and lives in `components/utils.ts`**
  (`getTimeGroup`, `groupConversations`, `extractLastUserMessage`, `TIME_GROUP_LABELS`).
  Add a group there, with its label, rather than branching in the list.
- **HS6 — `useConversations()` is called unconditionally in `ChatHistory`,** with the
  comment "needed to initiate remote-sync". Removing it breaks the signed-out
  population of the local list.
- **HS7 — A conversation with no user message previews as "New conversation".**
  That is `extractLastUserMessage`'s fallback; keep it.

## Workflows

**Adding a field to a history row**
1. Add it to `HistoryConversation` in `components/types.ts`.
2. Extend the `GetConversationsForHistory` selection in `graphql/chatHistoryDocument.ts`
   (and the server schema) for the remote path.
3. Populate it in **both** `RemoteChatHistory` and `LocalChatHistory` — a field
   present in one path and missing in the other is the classic bug here.
4. Render it in `components/ConversationItem.tsx`.

**Adding a time group**
1. Extend `TimeGroup` and `GroupedConversations` in `components/types.ts`.
2. Add the `getTimeGroup` branch and the `TIME_GROUP_LABELS` entry in
   `components/utils.ts`.
3. Initialise the new bucket in `groupConversations`.
4. Render it in `components/ConversationList.tsx`.

**Debugging an empty list**
1. Signed out: check `useConversations()` storage directly.
2. Signed in: the query is gated on `profileId` from
   `GetProfileIdByUserIdDocument`; no profile → a permanent spinner, not an empty
   list.
3. Check the cursor `getNextPageParam` against `pageInfo.endCursor`.

**Verifying**
1. `bun scripts/dev/inspect-ui.ts open-sidepanel`, then `snapshot sidepanel` on
   `#/history`.
2. Test both branches: sign in for remote, sign out for local.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — panel overview.
- [`./components/AGENTS.md`](./components/AGENTS.md) — list, grouping, item.
- [`./graphql/AGENTS.md`](./graphql/AGENTS.md) — the remote read/delete documents.
- [`./local/AGENTS.md`](./local/AGENTS.md) — the storage-backed path.
- [`../layout/AGENTS.md`](../layout/AGENTS.md) — `ChatSessionContext` for the active id.
- [`../../../../lib/conversations/conversationStorage.ts`](../../../lib/conversations/conversationStorage.ts) — local conversations.
