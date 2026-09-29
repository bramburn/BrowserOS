# `components/execution-history/` — Task and step cards

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

Two collapsible cards that render a recorded agent run:
`ExecutionTaskCard` is the outer card (status icon, relative timestamp,
duration, action/approval/denied/error counts, a collapsible step list and
a delete affordance); `ExecutionStepItem` is one tool call inside it,
rendered through the vendored `Tool` / `ToolInput` / `ToolOutput`
primitives from `../ai-elements/`. Both consume the record types defined
in `lib/execution-history/types.ts` and hold no fetching logic.

## Contents

```
execution-history/
├── ExecutionTaskCard.tsx  ← per-conversation task summary; maps
│                            ExecutionTaskRecord['status'] to an icon and
│                            renders <ExecutionStepItem> per step
└── ExecutionStepItem.tsx  ← one ExecutionStepRecord; Collapsible +
                            ToolInput/ToolOutput; state → label/icon map
                            (Preparing / Running / Approval Needed /
                            Approval Responded / Completed / Denied / Error)
```

## Rules

- **EXH1 — Read-only over the record types.** Both components take
  `ExecutionTaskRecord` / `ExecutionStepRecord` and render. Persistence lives
  in `lib/execution-history/storage.ts`; producing the records from AI-SDK
  message parts lives in `lib/execution-history/normalize.ts`.
- **EXH2 — Status → label/icon mapping is duplicated in both files on
  purpose** (task status vs step state are different unions). Add a new
  `ExecutionStepState` in `lib/execution-history/types.ts` and update
  `formatStateLabel` / `getStateIcon` in `ExecutionStepItem.tsx` in the same
  change.
- **EXH3 — `dayjs` plugins are extended once, at module scope, in
  `ExecutionTaskCard.tsx`** (`relativeTime`, `duration`). Don't re-extend them.
- **EXH4 — Icons are lucide; status colours are literal Tailwind palettes**
  (`text-green-500`, `text-orange-500`, `text-[var(--accent-orange)]`).
  Keep the token-based accent for "running" so it follows the theme.

## Workflows

**Adding a new step state**
1. Add the value to `ExecutionStepState` in `lib/execution-history/types.ts`.
2. Add it to `TERMINAL_STEP_STATES` in `lib/execution-history/normalize.ts`
   if it ends a step.
3. Map it in `formatStateLabel()` and `getStateIcon()` here.

**Deleting a task from the UI**
1. Trigger the removal through `lib/execution-history/storage.ts`
   (or the conversation-level `removeConversation`).
2. Keep the button in `ExecutionTaskCard` as a prop/callback, not a
   direct storage write.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules (CMP1).
- [`../ai-elements/AGENTS.md`](../ai-elements/AGENTS.md) — `tool.tsx` (`Tool`, `ToolInput`, `ToolOutput`).
- [`../../lib/execution-history/AGENTS.md`](../../lib/execution-history/AGENTS.md) — record types, storage, normalizer.
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — `Collapsible`, `Badge`, `Button`.
