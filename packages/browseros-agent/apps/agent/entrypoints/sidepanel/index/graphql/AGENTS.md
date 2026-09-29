# `entrypoints/sidepanel/index/graphql/` — chat session GraphQL documents

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel/index`.

## What's here

Four documents covering remote conversation persistence: create a conversation with
its first message, append a message, bump `lastMessagedAt`, and read a conversation
back with its messages. One file, no components, no hooks.

This is the write path used by `../useRemoteConversationSave.ts`; the read path
lives in `../../history/graphql/chatHistoryDocument.ts`.

## Contents

```
graphql/
└── chatSessionDocument.ts
    ├── CreateConversationWithMessage   (mutation: conversation + first message)
    ├── AppendConversationMessage        (mutation: one message with orderIndex)
    ├── UpdateConversationLastMessagedAt (mutation: touch)
    └── GetConversationWithMessages      (query: first 100 messages, ORDER_INDEX_ASC)
```

## Rules

- **G1 — One `<feature>Document.ts` per surface.** Sibling pattern:
  `../../../../app/ai-settings/graphql/` and `../../../../app/profile/graphql/`.
- **G2 — `graphql` is imported from `@/generated/graphql/gql`.** Codegen owns the
  types; do not hand-write result interfaces.
- **G3 — The SDL is the source of truth.** New fields go into
  `apps/agent/schema/schema.graphql`, then `bun run codegen` from
  `packages/browseros-agent`.
- **G4 — `message` is a `JSON!` scalar** holding a serialised AI SDK `UIMessage`. The
  shape is a wire contract with the server; changing it requires a coordinated change
  there, not just a re-selection.
- **G5 — Ordering is explicit.** Appends take an `orderIndex`; reads request
  `ORDER_INDEX_ASC` with `first: 100`. Do not drop the order-by clause — messages
  come back in insertion order or the transcript is scrambled.
- **G6 — `lastMessagedAt: "now()"` is a server-side expression string,** not a
  client timestamp. Keep the quotes.
- **G7 — Cache keys come from `getQueryKeyFromDocument`.** Invalidate the same way in
  `../useRemoteConversationSave.ts` and `../../history/ChatHistory.tsx`.

## Workflows

**Adding a persisted message field**
1. Update the `message` payload in the server's conversation-message resolver.
2. Adjust the `UIMessage` serialisation in `../useRemoteConversationSave.ts`.
3. Leave these documents unchanged unless the *selection* changes.

**Adding a query here**
1. Add the operation to `chatSessionDocument.ts` with a `PascalCase` name ending in
   `Document`.
2. Ensure the field exists in the SDL, then run `bun run codegen`.
3. Consume it with `useGraphqlQuery` / `useGraphqlMutation`.

**Debugging a history that restores out of order**
1. Confirm `AppendConversationMessage` is passed a monotonically increasing
   `orderIndex` by the caller.
2. Confirm the read query still requests `ORDER_INDEX_ASC`.
3. Check `GetConversationWithMessages` is not being served from a cache entry written
   before the ordering change — invalidate by document key.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the chat surface and its persistence rules.
- [`../useRemoteConversationSave.ts`](../useRemoteConversationSave.ts) — the caller.
- [`../../history/graphql/AGENTS.md`](../../history/graphql/AGENTS.md) — the read/delete side.
- [`../../../../../schema/schema.graphql`](../../../../schema/schema.graphql) — hand-maintained SDL.
- [`../../../../../lib/graphql/`](../../../../lib/graphql/) — hooks and key derivation.
