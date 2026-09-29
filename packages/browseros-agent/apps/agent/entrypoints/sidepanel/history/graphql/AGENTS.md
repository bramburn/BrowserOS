# `entrypoints/sidepanel/history/graphql/` — history GraphQL documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel/history`.

## What's here

Two documents for the signed-in history path: a cursor-paginated list of
conversations (each with the last two messages, enough to derive a preview) and a
delete mutation. One file, no components, no hooks.

This is the read/delete counterpart to
[`../../index/graphql/chatSessionDocument.ts`](../../index/AGENTS.md), which writes
the same records.

## Contents

```
graphql/
└── chatHistoryDocument.ts
    ├── GetConversationsForHistory  (query, first: 50, LAST_MESSAGED_AT_DESC, cursor)
    └── DeleteConversation          (mutation by rowId)
```

## Rules

- **G1 — Only two messages are requested per conversation** (`first: 2`,
  `ORDER_INDEX_DESC`) because the list only needs the latest user message. Raising
  this multiplies the payload for no UI gain.
- **G2 — Ordering is `LAST_MESSAGED_AT_DESC`** with a cursor (`after: $after`).
  Changing the order without changing the grouping expectations in
  `../components/` breaks the "Today / This Week" grouping.
- **G3 — `lastMessagedAt` may come back without a trailing `Z`.** The caller in
  `../ChatHistory.tsx` appends one before parsing. Do not "simplify" the caller to
  `new Date(node.lastMessagedAt)`.
- **G4 — Delete invalidates by document key**, not a literal array:
  `getQueryKeyFromDocument(GetConversationsForHistoryDocument)`.
- **G5 — `message` is the serialised AI SDK `UIMessage` as a JSON scalar.** The
  caller casts it; keep the shape aligned with
  `../../index/graphql/chatSessionDocument.ts`.
- **G6 — SDL first.** Any new field belongs in `apps/agent/schema/schema.graphql`
  before `bun run codegen`.

## Workflows

**Adding a field to the history list**
1. Add it to the schema, run `bun run codegen`.
2. Add it to the `nodes` selection in `GetConversationsForHistory`.
3. Map it into `HistoryConversation` in `../ChatHistory.tsx` **and** in
   `../local/LocalChatHistory.tsx`.
4. Render it in `../components/ConversationItem.tsx`.

**Changing the page size**
1. `first: $first` defaults to `50` in the operation.
2. `initialPageParam: undefined` and `getNextPageParam` read
   `pageInfo.endCursor` — a page-size change alone is safe, but check the sentinel
   scroll in `../components/ConversationList.tsx`.

**Debugging a stuck spinner**
1. The query is `enabled: !!profileId`; without a profile it never fires and the UI
   shows a spinner forever rather than an empty state.
2. `profileId` comes from `GetProfileIdByUserIdDocument` in
   `@/lib/conversations/graphql/uploadConversationDocument` — check the session first.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the history route.
- [`../ChatHistory.tsx`](../ChatHistory.tsx) — the consumer, including the `Z` normalisation.
- [`../../index/graphql/AGENTS.md`](../../index/graphql/AGENTS.md) — the write side.
- [`../../../../../schema/schema.graphql`](../../../../schema/schema.graphql) — hand-maintained SDL.
