# `entrypoints/sidepanel/history/components/` — history list UI

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel/history`.

## What's here

The presentational half of the history route: a `ConversationList` that renders time
groups, each group a `ConversationGroup`, each entry a `ConversationItem` (link to
`#/`, delete action, active highlight). `types.ts` holds the row contract and
`utils.ts` the pure grouping/formatting helpers.

Both history paths — GraphQL and local storage — feed this same list, so anything
added here must be expressible from `HistoryConversation` alone.

## Contents

```
components/
├── ConversationList.tsx    ← groups + infinite-scroll sentinel (IntersectionObserver)
├── ConversationGroup.tsx   ← one time bucket: label + its items
├── ConversationItem.tsx    ← one row: title, time, active state, delete
├── types.ts                ← HistoryConversation, TimeGroup, GroupedConversations
└── utils.ts                ← TIME_GROUP_LABELS, getTimeGroup, extractLastUserMessage, groupConversations
```

## Rules

- **HC1 — The row contract is `HistoryConversation`** (`id`, `lastMessagedAt`,
  `lastUserMessage`). Do not widen the props to reach for provider, model, or message
  counts; extend the contract and both mappers instead.
- **HC2 — Grouping is computed by the caller and passed in**
  (`groupedConversations` prop). The list does not group; it renders.
- **HC3 — Infinite scroll is optional and prop-driven.** `hasNextPage` + `onLoadMore`
  arm an `IntersectionObserver` on a sentinel div; without both, no observer is
  created. Keep the early return.
- **HC4 — Pagination props are optional on purpose,** because the local (storage)
  path never supplies them. Any new prop that is required breaks that path.
- **HC5 — Pure formatting goes in `utils.ts`;** rendering goes in the components.
  `utils.ts` imports `dayjs` and the `ai` `UIMessage` type and nothing from React.
- **HC6 — Time groups are `today | thisWeek | thisMonth | older`,** in that display
  order, via `TIME_GROUP_LABELS`. Adding a group means extending the type, the label
  map, `getTimeGroup`, the bucket initialisation, **and** the list's render order.
- **HC7 — "New conversation" is the empty preview fallback** and is produced by
  `extractLastUserMessage`, not by the item component.

## Workflows

**Adding a column to a history row**
1. Extend `HistoryConversation` in `types.ts`.
2. Populate it in both `../ChatHistory.tsx` (remote) and `../local/LocalChatHistory.tsx`.
3. Render it in `ConversationItem.tsx`.
4. If it needs formatting, add a pure helper to `utils.ts`.

**Adding a time group**
1. Extend `TimeGroup` and `GroupedConversations` in `types.ts`.
2. Add the label to `TIME_GROUP_LABELS` and the branch to `getTimeGroup`.
3. Initialise the bucket in `groupConversations`.
4. Add the group in `ConversationList.tsx` in display order.

**Debugging an empty or ungrouped list**
1. `groupConversations` fills all four buckets up front — a missing key renders as
   nothing, not as an error.
2. Check `lastMessagedAt` is epoch milliseconds, not an ISO string, on both paths.
3. Check the `extractLastUserMessage` fallback before assuming the data is empty.

**Verifying**
1. `bun scripts/dev/inspect-ui.ts open-sidepanel`, `snapshot sidepanel` on
   `#/history`.
2. Scroll to the bottom to exercise the infinite-scroll sentinel (remote only).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the history route and its two data sources.
- [`./types.ts`](./types.ts) — the contract to read first.
- [`../local/AGENTS.md`](../local/AGENTS.md) — the storage-backed caller.
- [`../../../../lib/conversations/conversationStorage.ts`](../../../../lib/conversations/conversationStorage.ts) — local conversation shape.
