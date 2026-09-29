# `entrypoints/newtab/index/lib/suggestions/` — suggestion aggregation and types

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `entrypoints/newtab/index/lib`.

## What's here

The hub of the new-tab combobox. `types.ts` defines the discriminated
`SuggestionItem` union (`search` | `ai-tab` | `browseros`) and the `SuggestionSection`
wrapper. `useSuggestions.ts` calls the three source hooks, de-dupes and orders the
results, assigns ids, and builds the sections that `../../SearchSuggestions.tsx`
renders. It also exports `getSuggestionLabel`, the text Downshift puts back into the
input when an item is highlighted or selected.

This is the only folder the other three source folders depend on — and it depends on
all of them. It is the seam.

## Contents

```
suggestions/
├── types.ts             ← SuggestionType, SearchSuggestionItem, AITabSuggestionItem,
│                           BrowserOSSuggestionItem, SuggestionItem, SuggestionSection
└── useSuggestions.ts    ← useSuggestions() + getSuggestionLabel() + SEARCH_PROVIDER
```

## Rules

- **SG1 — `getSuggestionLabel` is an exhaustive switch with no default.** Adding a
  `SuggestionItem` variant without adding a case is a type error at the call site;
  keep it that way.
- **SG2 — Ids are positional** (`browseros-<index>`, `ai-tab-<index>`,
  `search-<index>`) and are produced here, not in the source hooks. A source returning
  its own ids is a bug.
- **SG3 — Section order and exclusivity are product behaviour.** The current order is
  BrowserOS → then *either* AI tab actions *or* Google search (an `if/else`, not two
  blocks). Changing it changes what the user sees first.
- **SG4 — The raw query is always the first search result,** inserted before the API
  list and then de-duped case-insensitively. That is what makes "press Enter to
  search exactly this" work.
- **SG5 — Keep `sections`, `flatItems` and `providerConfig` as the return shape.**
  The rendering component flattens for keyboard navigation via `flatItems`; renaming
  these breaks it.
- **SG6 — Memo dependencies must list everything used.** The `sections` memo
  deliberately depends on `selectedTabs.length` rather than the array identity so
  re-selecting the same tab does not rebuild the list.
- **SG7 — Section titles live here,** including the empty title for the BrowserOS
  section (single item by design).

## Workflows

**Adding a new section**
1. Add the item variant in `types.ts` and extend the `SuggestionItem` union.
2. Call the source hook inside `useSuggestions`.
3. Map the raw result into items and push a `SuggestionSection` in the `sections`
   memo, with the new value in the dependency array.
4. Add the `getSuggestionLabel` case.
5. Teach `../../SearchSuggestions.tsx` to render the new `type`.

**Changing the search provider**
1. Edit `SEARCH_PROVIDER` (`id`, `name`, `searchUrl`) at the top of
   `useSuggestions.ts`.
2. Update the section title interpolation, which uses `SEARCH_PROVIDER.name`.
3. Update `../searchSuggestions/` and the `host_permissions` in `wxt.config.ts`.

**Testing behaviour without a browser**
1. `buildSearchResults` and `getSuggestionLabel` are pure; call them directly.
2. For ordering, drive `useSuggestions` with stub source modules.
3. Run `bun run test` from `packages/browseros-agent` (Bun test runner).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the new-tab data-layer rules.
- [`../searchSuggestions/AGENTS.md`](../searchSuggestions/AGENTS.md) — the network source.
- [`../../AGENTS.md`](../../AGENTS.md) — `SearchSuggestions.tsx` and `NewTab.tsx`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — the new-tab route rules.
