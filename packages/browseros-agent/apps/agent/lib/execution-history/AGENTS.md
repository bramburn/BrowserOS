# `lib/execution-history/` — Recorded agent runs

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A durable record of what the agent actually did during a run. As a turn
streams, the sidepanel normalizes the AI-SDK message parts into
`ExecutionStepRecord`s grouped into an `ExecutionTaskRecord`, upserts them
per conversation, and the UI renders them with
`components/execution-history/`. This is what powers the "what did that
just do?" view and the approval trail, and it survives a panel reload.

## Contents

```
execution-history/
├── types.ts            ← ExecutionTaskStatus (running | completed | stopped |
│                          failed | interrupted); ExecutionStepState
│                          (input-streaming, input-available,
│                          approval-requested, approval-responded,
│                          output-available, output-error, output-denied);
│                          ExecutionStepApproval, ExecutionStepRecord,
│                          ExecutionTaskRecord, ConversationExecutionHistory,
│                          ExecutionHistoryByConversation
├── storage.ts          ← executionHistoryStorage
│                          (local:executionHistoryByConversation, version 1);
│                          upsertConversationExecutionTask(), and friends
│                          (removeConversationExecutionHistory, …)
├── normalize.ts        ← AI-SDK UIMessage parts → records; excludes the
│                          "nudge" tools (suggest_schedule,
│                          suggest_app_connection); truncates previews to 180
│                          chars; maps approval/denied/error states
└── normalize.test.ts   ← the normaliser's unit tests
```

## Rules

- **EXL1 — Persist shape is keyed by conversation id.** The storage value is
  `Record<conversationId, { updatedAt, tasks }>`. Never store a flat task
  list; `removeConversationExecutionHistory(id)` depends on the key.
- **EXL2 — `normalize.ts` filters out nudge tools.** `suggest_schedule` and
  `suggest_app_connection` are UI suggestions, not actions; adding a new
  suggestion tool means adding it to `NUDGE_TOOL_NAMES` or it will show up as
  a real execution step.
- **EXL3 — Previews are capped at 180 characters** (`MAX_PREVIEW_CHARS`).
  Long tool output must be truncated before it reaches `chrome.storage.local`.
- **EXL4 — Terminal step states are enumerated in `TERMINAL_STEP_STATES`.** A
  state that ends a step must be added there, or the task never completes.
- **EXL5 — `types.ts` is the shared contract** with
  `components/execution-history/`. Adding a `ExecutionStepState` requires
  updating `formatStateLabel` / `getStateIcon` there in the same change.
- **EXL6 — Bump the storage `version` on shape changes.** The item is at
  `version: 1`; add a migration rather than reshaping stored records in place.

## Workflows

**Recording a streamed run**
1. Feed the current `UIMessage` parts into `normalize.ts`.
2. Upsert the resulting `ExecutionTaskRecord` via
   `upsertConversationExecutionTask(record)`.
3. The storage `watch` keeps the UI in sync — no manual refresh needed.

**Adding a step state**
1. Add it to `ExecutionStepState` in `types.ts`.
2. Add it to `TERMINAL_STEP_STATES` in `normalize.ts` if it ends a step.
3. Map it in `components/execution-history/ExecutionStepItem.tsx`.
4. Add a case to `normalize.test.ts`.

**Clearing history**
1. Per conversation: `removeConversationExecutionHistory(conversationId)`.
2. It is already chained into `useConversations().removeConversation` in
   `../conversations/conversationStorage.ts` — don't call it twice.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB7).
- [`../../components/execution-history/AGENTS.md`](../../components/execution-history/AGENTS.md) — the renderers.
- [`../conversations/AGENTS.md`](../conversations/AGENTS.md) — removal coupling.
- [`../chat-actions/AGENTS.md`](../chat-actions/AGENTS.md) — the composer that triggers runs.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — `useExecutionHistoryTracker.ts` in the sidepanel.
