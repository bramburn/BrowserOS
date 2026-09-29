# `entrypoints/newtab/index/lib/` — new-tab feature-local data layer

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/newtab/index`.

## What's here

Four tiny folders, one per suggestion source for the new-tab combobox. Each exports a
single hook (and, where relevant, a types file) and nothing else. There is no barrel
and no shared state between them.

`../SearchSuggestions.tsx` never imports these directly — it consumes the aggregated
`sections` array produced by `suggestions/useSuggestions.ts`.

## Contents

```
lib/
├── suggestions/          ← types + aggregation (the only folder others import)
├── aiTabSuggestions/     ← tab-count-aware AI actions
├── browserOSSuggestions/ ← BrowserOS-mode suggestion
└── searchSuggestions/    ← Google suggest endpoint via SWR (debounced)
```

## Rules

- **NL1 — One source per folder, one exported hook per folder.** Adding a second hook
  to `searchSuggestions/` means adding a folder instead.
- **NL2 — Source hooks return raw shapes; the aggregator maps them.** A source returns
  plain data (no `id`, no section title). `suggestions/useSuggestions.ts` assigns ids
  (`browseros-0`, `ai-tab-0`, `search-0`) and section titles.
- **NL3 — Network I/O happens only in `searchSuggestions/`.** The other three sources
  are synchronous derivations of their inputs.
- **NL4 — Dedupe and ordering are the aggregator's job.** `useSuggestions` seeds the
  search list with the raw query, then de-dupes case-insensitively. Do not duplicate
  that in a source.
- **NL5 — `SEARCH_PROVIDER` is a local constant in `suggestions/useSuggestions.ts`**
  (Google + its search URL). Changing the default engine means editing that constant
  and the host permission in `wxt.config.ts` together.

## Workflows

**Adding a source folder**
1. Create `lib/<name>/use<Name>Suggestions.ts` returning a plain array.
2. Add a `<Name>SuggestionItem` variant in `../lib/suggestions/types.ts` and extend
   the `SuggestionItem` union.
3. Map it into a `SuggestionSection` in `useSuggestions.ts` and add a
   `getSuggestionLabel` case (the switch is exhaustive — it has no default).
4. Render the section in `../SearchSuggestions.tsx`.

**Changing section priority**
1. Edit the `sections` `useMemo` in `useSuggestions.ts`.
2. AI tab actions and Google search are currently an `if/else`; making them additive
   changes behaviour for the common case and needs a product decision.
3. Update the dependency array in the same memo.

**Debouncing or changing the suggest request**
1. Edit `searchSuggestions/useSearchSuggestions.ts` (`debounceMs`, default 300).
2. The SWR key tuple is `['google-search-suggestions', debouncedQuery]`; the fetcher
   destructures that shape.
3. If you change the host, update `host_permissions` in `wxt.config.ts`.

## Cross-references

- [`./suggestions/AGENTS.md`](./suggestions/AGENTS.md) — aggregation and item types.
- [`../AGENTS.md`](../AGENTS.md) — the consuming screen.
- [`../AGENTS.md`](../AGENTS.md) — the new-tab route rules.
- [`../../../../wxt.config.ts`](../../../../wxt.config.ts) — `https://suggestqueries.google.com/*` host permission.
