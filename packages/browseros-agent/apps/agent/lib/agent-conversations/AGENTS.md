# `lib/agent-conversations/` — Agent transcripts (IndexedDB)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The local, on-device transcript of conversations with a **named agent**
(as opposed to a plain LLM chat). It is the one store in `lib/` that does
not use `chrome.storage.local`: it writes structured turns — user text,
attachment previews, and assistant parts (text / thinking / tool batches) —
through `idb-keyval` into IndexedDB, because the payloads are far larger
than the 5 MB practical limit of extension local storage. Keys are
`agent-conv:<agentId>:<sessionKey>`.

## Contents

```
agent-conversations/
├── types.ts    ← AssistantTextPart / AssistantThinkingPart /
│                  AssistantToolBatchPart → AssistantPart union;
│                  ToolEntry { id, name, label, subject?, status, durationMs? };
│                  UserAttachmentPreview (in-memory only, by design);
│                  AgentConversationTurn (turnId from the `X-Turn-Id`
│                  response header, or from the active-turn payload on
│                  resume); AgentConversation { agentId, agentName,
│                  sessionKey, turns, createdAt, updatedAt }
└── storage.ts  ← saveConversation(), getLatestConversation(agentId) —
                  lists matching keys and returns the newest by updatedAt —
                  and deleteConversation(agentId, sessionKey)
```

## Rules

- **ACON1 — IndexedDB via `idb-keyval`, not `chrome.storage`.** This is the
  deliberate exception to the "storage.ts uses `local:`" convention. Don't
  "normalise" it to `@wxt-dev/storage`; the quota will bite.
- **ACON2 — Key format is load-bearing.** `agent-conv:${agentId}:${sessionKey}`
  is what `getLatestConversation` prefix-filters on. Changing the separator
  orphans every existing transcript.
- **ACON3 — `getLatestConversation` sorts by `updatedAt` descending** and
  returns `undefined` when nothing matches. Callers rely on the `undefined`
  case, not an empty object.
- **ACON4 — Attachment `dataUrl`s are intentionally ephemeral.** They live
  only for the live turn; a history reload re-fetches attachments from the
  server's JSONL. Don't try to persist them in IndexedDB.
- **ACON5 — `turnId` is optional by design.** It is absent during the brief
  optimistic window before response headers arrive, and the historic-files
  fallback fetch requires it.

## Workflows

**Persisting a live agent turn**
1. `saveConversation({ agentId, agentName, sessionKey, turns, createdAt, updatedAt })`
2. Append to `turns`; keep `updatedAt` current or "latest" lookups go stale.

**Loading the newest transcript for an agent on mount**
1. `const conv = await getLatestConversation(agentId)`
2. `undefined` means no history — render the empty state, not a spinner.

**Adding a new assistant part kind**
1. Extend the `AssistantPart` union in `types.ts`.
2. Add a renderer for it in the sidepanel message components.
3. Bump nothing in storage — old records simply won't have the new kind.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB7; this folder is LIB2's documented exception).
- [`../../graphql/QueryProvider.tsx`](../graphql/QueryProvider.tsx) — the other `idb-keyval` user in `lib/`.
- [`../conversations/AGENTS.md`](../conversations/AGENTS.md) — the separate `chrome.storage.local` chat history.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the sidepanel that renders these turns.
