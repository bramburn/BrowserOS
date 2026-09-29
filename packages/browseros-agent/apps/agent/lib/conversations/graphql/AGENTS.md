# `lib/conversations/graphql/` — Conversation upload documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/conversations`.

## What's here

One file, `uploadConversationDocument.ts`, holding the five operations
that make conversation sync incremental. Their ordering inside
`../uploadConversationsToGraphql.ts` is the whole algorithm: resolve the
profile, check existence, check how many messages are already there, then
create the conversation and bulk-create only the missing messages.

## Contents

```
graphql/
└── uploadConversationDocument.ts
    ├── GetProfileIdByUserIdDocument          ← userId → profile rowId
    ├── ConversationExistsDocument            ← pConversationId → boolean
    ├── GetUploadedMessageCountDocument       ← conversationMessages(first: 0)
    │                                           → totalCount
    ├── CreateConversationForUploadDocument   ← rowId, profileId, lastMessagedAt,
    │                                           createdAt
    └── BulkCreateConversationMessagesDocument← conversationMessages input
```

## Rules

- **CVG1 — `GetProfileIdByUserIdDocument` is reused elsewhere.**
  `../../schedules/syncSchedulesToBackend.ts` imports it from this file rather
  than declaring its own. Keep that single definition.
- **CVG2 — `GetUploadedMessageCountDocument` uses `first: 0`.** The count
  comes back on the connection, not from rows; don't "fix" it to `first: 1`.
- **CVG3 — Named operations only** (they are the query-cache keys).
- **CVG4 — Selection sets drive payload size.** `BulkCreate…` selects only
  `id, rowId, conversationId, orderIndex` — response bloat, not a bug.
- **CVG5 — `schema/schema.graphql` is the source.** Update it, run
  `bun run codegen`, then edit these documents.

## Workflows

**Adding a synced field**
1. Add the column to `schema/schema.graphql`; `cd packages/browseros-agent`
   and `bun run codegen`.
2. Extend the relevant selection set here.
3. Map it in `../uploadConversationsToGraphql.ts`.

**Debugging a skipped upload**
1. `ConversationExistsDocument` tells you whether the row exists.
2. `GetUploadedMessageCountDocument` tells you how many messages landed;
   if it is already `>= messages.length` the client intentionally skips.
3. `GetProfileIdByUserIdDocument` returning null aborts the whole upload.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — conversation rules (CONV2, CONV3).
- [`../uploadConversationsToGraphql.ts`](../uploadConversationsToGraphql.ts) — the caller.
- [`../../graphql/AGENTS.md`](../../graphql/AGENTS.md) — `execute()` and the hooks.
- [`../../../schema/AGENTS.md`](../../../schema/AGENTS.md) — the SDL source.
- [`../../schedules/AGENTS.md`](../../schedules/AGENTS.md) — the other consumer of `GetProfileIdByUserIdDocument`.
