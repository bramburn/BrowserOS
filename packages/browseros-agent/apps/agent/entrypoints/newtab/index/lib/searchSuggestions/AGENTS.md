# `entrypoints/newtab/index/lib/searchSuggestions/` — Google search suggestions

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `entrypoints/newtab/index/lib`.

## What's here

The only network-backed suggestion source. Two files: a debouncing SWR hook and the
fetcher it calls. It hits Google's `suggestqueries.google.com/complete/search`
endpoint directly from the page, which is why `wxt.config.ts` carries
`https://suggestqueries.google.com/*` in `host_permissions`.

There is a standing TODO in `getSearchSuggestions.ts` to move this fetch into the
background service worker to avoid CORS issues. Keep that TODO attached to the code.

## Contents

```
searchSuggestions/
├── getSearchSuggestions.ts  ← fetch against the Google suggest endpoint
└── useSearchSuggestions.ts  ← debounce (300 ms default) + SWR(keepPreviousData)
```

## Rules

- **SS1 — The SWR key is a tuple** `['google-search-suggestions', debouncedQuery]`,
  and the fetcher destructures that exact shape. Changing one without the other
  yields `undefined` queries.
- **SS2 — An empty query means a `null` key,** which disables the request. Do not
  fetch on empty input.
- **SS3 — `keepPreviousData: true` is deliberate** so the list does not flash empty
  between keystrokes. Do not remove it.
- **SS4 — Debounce lives in the hook, not the fetcher** (`debounceMs = 300`). Do not
  add a second debounce upstream.
- **SS5 — The endpoint is fixed in this folder,** as is the `SEARCH_PROVIDER` constant
  in `../suggestions/useSuggestions.ts`. Swapping engines means touching both plus
  `wxt.config.ts` host permissions.
- **SS6 — CORS is a known open problem,** flagged in-code. Do not add more
  page-side third-party fetches next to this one; route new ones through the worker.

## Workflows

**Adding a second search provider**
1. Add the endpoint in `getSearchSuggestions.ts` and include the provider in the key
   tuple so both engines can be cached side by side.
2. Add the host to `host_permissions` in `wxt.config.ts`.
3. Update `SEARCH_PROVIDER` in `../suggestions/useSuggestions.ts` (and rename it if
   there is more than one).
4. Merge the two result lists there — de-duping already happens in the aggregator.

**Debugging empty suggestions**
1. Confirm the host permission is in the built `manifest.json` (host permissions are
   trimmed by the browser at install time).
2. Confirm the request is not CORS-blocked in the page console.
3. Check the debounce is not swallowing input (300 ms default).

**Verifying**
1. `bun scripts/dev/inspect-ui.ts open-app`, load `app.html#/home`.
2. Type into the omnibox, then `snapshot` to read the rendered section.

## Cross-references

- [`../suggestions/AGENTS.md`](../suggestions/AGENTS.md) — the aggregator and `SEARCH_PROVIDER`.
- [`../AGENTS.md`](../AGENTS.md) — the new-tab data-layer rules.
- [`../../../../../../wxt.config.ts`](../../../../../wxt.config.ts) — `host_permissions`.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — the background service worker (the TODO's destination).
