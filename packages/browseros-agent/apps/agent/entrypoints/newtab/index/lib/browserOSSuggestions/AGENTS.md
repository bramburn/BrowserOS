# `entrypoints/newtab/index/lib/browserOSSuggestions/` — BrowserOS-mode suggestion

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `entrypoints/newtab/index/lib`.

## What's here

The smallest folder in the new-tab tree: one file exporting a `BrowserOSSuggestion`
type and a `useBrowserOSSuggestions` hook. The hook takes a query and returns a
single `{ mode: 'agent', message: query }` entry — i.e. "run what I typed as an
agent task" — rather than a list of matching commands.

It is intentionally a stub. The aggregator in `../suggestions/useSuggestions.ts`
already handles the empty case, and the comment in that file notes the section
carries no title because there is only ever one item.

## Contents

```
browserOSSuggestions/
└── useBrowserOSSuggestions.ts   ← BrowserOSSuggestion type + the hook
```

## Rules

- **BS1 — One entry, by design.** Do not "improve" this into a command list without a
  product decision; the aggregator and the section title both assume a single item.
- **BS2 — `mode: 'agent'` is what routes the submission.** It feeds
  `createBrowserOSAction` downstream. Changing it to `'chat'` changes routing
  behaviour, not just a label.
- **BS3 — The message is the raw query, untrimmed-and-unfiltered by this hook.** The
  caller (`useSuggestions`) passes `query.trim()`; trimming is not repeated here.
- **BS4 — Keep it React-free apart from the hook signature.** No effects, no fetches,
  no storage. A future implementation that needs a catalogue should return data, not
  perform requests during render.
- **BS5 — Both exports are `@public` in the file's JSDoc.** That annotation is
  deliberate — keep it when editing.

## Workflows

**Adding a second BrowserOS suggestion**
1. Extend the return array in `useBrowserOSSuggestions.ts`.
2. Re-check `../suggestions/useSuggestions.ts`: the section title is `''` because the
   list is expected to hold one item, and the `browseros-<index>` id scheme already
   supports more.
3. Update the comment in the aggregator that states the single-item assumption.

**Making the query smarter**
1. Filter or rank inside the hook; keep the return type an array of
   `BrowserOSSuggestion` so the aggregator is unchanged.
2. Return `[]` for a blank query — the aggregator already checks `browserOSResults.length > 0`.
3. Do not add a network call here; that belongs in a `lib/` feature module.

## Cross-references

- [`../suggestions/AGENTS.md`](../suggestions/AGENTS.md) — the aggregator that consumes this hook.
- [`../AGENTS.md`](../AGENTS.md) — the new-tab data-layer rules.
- [`../../../../../../../lib/chat-actions/types.ts`](../../../../../lib/chat-actions/types.ts) — `createBrowserOSAction` and the `mode` values.
