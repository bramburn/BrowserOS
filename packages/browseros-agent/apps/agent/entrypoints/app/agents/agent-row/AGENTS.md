# `entrypoints/app/agents/agent-row/` — agent row card sub-components

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app/agents`.

## What's here

The presentation internals of a single agent row. `AgentRowCard` (one level up) is a
composition shell that owns no state; everything below renders a slice of the
`AgentRowData` object defined in `agent-row.types.ts`. The rule that makes this work
is depth: the page assembles one `AgentRowData` per row and hands it down — no prop
drilling past two levels.

## Contents

```
agent-row/
├── agent-row.types.ts     ← AgentRowData (the whole row contract), AgentTokenUsage, AgentAdapterHealth, AgentRowCallbacks
├── agent-row.helpers.ts   ← formatTokens, firstNonBlankLine, truncate, formatLocalDate, ROW_BAR_COUNT
├── agent-row.helpers.test.ts
├── AgentTile.tsx          ← icon/colour tile for the agent
├── AgentTitleRow.tsx      ← name + PinToggle
├── AgentMetaRow.tsx       ← adapter, model, reasoning-effort labels
├── AgentSummaryChips.tsx  ← status chips
├── AgentLastMessage.tsx   ← first user line preview
├── AgentTokenSummary.tsx  ← 7d / cumulative token counts
├── AgentSparkline.tsx     ← 14-bar turns/failures history
├── AgentErrorPanel.tsx    ← collapsible last-error detail
├── AgentActions.tsx       ← delete action + spinner state
└── PinToggle.tsx          ← pin / unpin switch
```

## Rules

- **AR1 — Sub-components take a slice, not the whole row.** Each of them declares its
  own narrow props; `AgentRowCard` does the destructuring. If you need a second slice,
  add the prop in `AgentRowCard.tsx`, don't widen the child.
- **AR2 — `AgentRowData` is the only cross-component contract.** Adding a field means
  editing `agent-row.types.ts` **and** whatever assembles the object in
  `../AdapterAgentsPane` / `../agents-list-order.ts` call sites.
- **AR3 — Formatting goes in `agent-row.helpers.ts`,** and branching logic there gets a
  case in `agent-row.helpers.test.ts`. This file is deliberately distinct from
  `../agent-display.helpers.ts` (page-level).
- **AR4 — The sparkline is 14 entries, oldest → newest, today last.** That is
  `ROW_BAR_COUNT` and the contract of `turnsByDay` / `failedByDay` in
  `agent-row.types.ts`. If you change the window, change all three together.
- **AR5 — Row state is encoded on the card, not in a child.** The card border
  communicates working (accent-orange glow) / error (destructive) / idle. Children
  that need to show the same state read it from the data they were given.
- **AR6 — Actions emit callbacks; they never mutate.** `AgentActions` and `PinToggle`
  call `onDelete` / `onPinToggle(id, next)`. The page owns the query invalidation.

## Workflows

**Adding a field to the row**
1. Add it to `AgentRowData` in `agent-row.types.ts` with a doc comment on its contract.
2. Populate it where `AgentRowData` is built (`../../ai-settings/AdapterAgentsPane.tsx`
   or `../AgentList.tsx`).
3. Render it in one of the existing sub-components, or add a small new one and mount
   it from `../AgentRowCard.tsx`.
4. If it needs formatting, add a pure helper to `agent-row.helpers.ts` + a test case.

**Adding a whole new sub-component**
1. Create a PascalCase `.tsx` in this folder.
2. Give it explicit props (no `AgentRowData` spread).
3. Mount it from `../AgentRowCard.tsx` in the right slot.
4. Keep it stateless; lift anything stateful to the page.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — data layer, ordering, query keys.
- [`../../ai-settings/AGENTS.md`](../../ai-settings/AGENTS.md) — where `AdapterAgentsPane` mounts `AgentList`.
- [`./agent-row.types.ts`](./agent-row.types.ts) — the contract; read it first.
