# `entrypoints/sidepanel/history/local/` — signed-out history

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel/history`.

## What's here

A single small component: the storage-backed history, used when there is no signed-in
user. It reads `useConversations()` from `@/lib/conversations/conversationStorage`,
maps each conversation into the shared `HistoryConversation` shape, groups it with
the same helper as the remote path, and renders the same `ConversationList`.

It is deliberately thin — all the difference from the remote path is the data source
and the absence of pagination.

## Contents

```
local/
└── LocalChatHistory.tsx   ← useConversations() → map → group → ConversationList
```

## Rules

- **LC1 — Reuse the shared mapping and grouping**, `extractLastUserMessage` and
  `groupConversations` from `../components/utils.ts`. Duplicating either means the two
  paths can show different groups for the same data.
- **LC2 — No pagination props.** `ConversationList`'s `hasNextPage` / `onLoadMore` /
  `isRefreshing` are optional precisely because this path omits them; passing
  `hasNextPage` without `onLoadMore` would arm nothing and confuse the next reader.
- **LC3 — Delete is `removeConversation` from the storage hook.** There is no
  mutation to invalidate here — the storage hook is the source of truth.
- **LC4 — The active highlight still comes from the session**
  (`useChatSessionContext().conversationId`), not from local state. Keep it so the
  signed-out panel highlights the open conversation too.
- **LC5 — Do not add network calls.** If this component needs data it does not have,
  the fix belongs in the storage hook, not here.

## Workflows

**Adding a field to signed-out history**
1. Extend `HistoryConversation` in `../components/types.ts`.
2. Extend the mapping in this file to populate it.
3. Mirror the same change in `../ChatHistory.tsx` (the remote path).
4. Render it in `../components/ConversationItem.tsx`.

**Debugging a missing conversation**
1. Check `useConversations()` returns entries — it is async and gated on storage load.
2. Check the id matches what the session reports as `conversationId`; a mismatch
  means the row exists but is not highlighted.
3. Check the storage version/migration in
   `@/lib/conversations/conversationStorage` if older conversations are missing.

**Verifying**
1. Sign out, then `bun scripts/dev/inspect-ui.ts open-sidepanel` and `snapshot` on
   `#/history`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the history route and the signed-in/signed-out switch.
- [`../components/AGENTS.md`](../components/AGENTS.md) — the shared list and grouping.
- [`../../../../lib/conversations/conversationStorage.ts`](../../../../lib/conversations/conversationStorage.ts) — `useConversations`, `removeConversation`.
