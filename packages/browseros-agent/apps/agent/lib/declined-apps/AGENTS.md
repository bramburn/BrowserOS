# `lib/declined-apps/` — Declined app connections

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

One file and one idea: the set of MCP app ids the user has explicitly
declined, so the agent stops suggesting them and the request body carries
`declinedApps` to the server. It is a `string[]` of ids, not objects.

## Contents

```
declined-apps/
└── storage.ts   ← declinedAppsStorage =
                     storage.defineItem<string[]>('local:declinedApps',
                       { fallback: [] })
```

## Rules

- **DEC1 — Imports `storage` from `#imports`, not `@wxt-dev/storage`.** This
  is the only storage module in `lib/` that uses WXT's auto-import alias.
  Don't "fix" it to `@wxt-dev/storage` in isolation — if you change it,
  change the convention across `lib/` and re-check the background worker.
- **DEC2 — Values are app ids, strings only.** Keep the shape flat; a
  `{ id, reason, declinedAt }` object is a different store and would need
  its own file.
- **DEC3 — Declining is not disconnecting.** This list records "don't offer
  this again". Removing an actual connection is
  `removeServer(id)` in `../mcp/mcpServerStorage.ts`. Don't conflate them.
- **DEC4 — It's a deny-list that reaches the model.** `declinedApps` is sent
  in the chat request body (`../messaging/server/buildChatRequestBody.ts`),
  so entries must be stable app ids that the server also recognises — never
  user-visible labels.
- **DEC5 — Un-declining means removing the id**, not a separate flag.

## Workflows

**Recording a decline**
1. `const current = await declinedAppsStorage.getValue() ?? []`
2. `await declinedAppsStorage.setValue([...current, appId])` (de-duplicated).

**Sending the deny-list with a chat request**
1. Read the item and pass it as `declinedApps` to `buildChatRequestBody`.
2. Omit the field entirely when the list is empty.

**Un-declining**
1. Filter the id out and write the array back.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, and the `#imports` note).
- [`../mcp/AGENTS.md`](../mcp/AGENTS.md) — `mcpServerStorage`, the connection list.
- [`../messaging/server/AGENTS.md`](../messaging/server/AGENTS.md) — the `declinedApps` request field.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the Connect Apps UI that triggers a decline.
