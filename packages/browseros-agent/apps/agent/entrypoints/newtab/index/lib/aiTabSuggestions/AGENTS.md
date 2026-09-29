# `entrypoints/newtab/index/lib/aiTabSuggestions/` — AI tab actions

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `entrypoints/newtab/index/lib`.

## What's here

A single file and a single hook. It holds a hard-coded list of five AI actions
(summarise this page, summarise selected tabs, identify topics, extract comments,
compare tabs), each with a `minTabs` / `maxTabs` window and a Lucide icon. The hook
filters the list by how many tabs the user has attached, and prepends a synthetic
"just do what I typed" entry carrying the raw input as the action name.

There is no network call and no storage here — it is a pure function of
`(selectedTabs, input)`.

## Contents

```
aiTabSuggestions/
└── useAITabSuggestions.ts   ← AITabSuggestion type, the `actions` array, the hook
```

## Rules

- **AS1 — The hook is synchronous and pure.** It takes `{ selectedTabs, input }` and
  returns a filtered array. Do not add effects, fetches, or storage.
- **AS2 — Adding an action means adding one object to `actions`.** Give it a
  `minTabs`/`maxTabs` window that matches reality — the array is filtered by those
  numbers before it reaches the UI, so a wrong window silently hides the action.
- **AS3 — The raw input always becomes the first entry** (`icon: Bot`, `maxTabs:
  Infinity`). This is what makes free-text submission work; do not gate it on tab
  count beyond the built-in `minTabs: 1`.
- **AS4 — Names are user-facing prompts.** The action `name` is what gets sent, so it
  must be an instruction, not a label. Keep the style of the existing five.
- **AS5 — Use `compact` from `es-toolkit` for filtering,** as the file already does,
  rather than adding a lodash-style filter chain.
- **AS6 — The aggregator assigns the `id`s.** Return raw entries; `useSuggestions`
  turns them into `AITabSuggestionItem`s with `ai-tab-<index>` ids.

## Workflows

**Adding a "compare 3+ tabs" style action**
1. Append to `actions` in `useAITabSuggestions.ts`.
2. Set `minTabs: 3` (or whatever the feature needs) and pick a `lucide-react` icon.
3. Write the `description` as a one-line explanation of the action.
4. Confirm the action appears with the right number of attached tabs — attach fewer
   than `minTabs` and it should vanish.

**Changing the free-text entry**
1. Edit the `inputAction` object in the hook.
2. Its `name` is the user's own text; keep `maxTabs: Infinity` unless the feature
   genuinely needs tabs.
3. Do not remove the entry — the combobox submits through it.

**Verifying**
1. `bun scripts/dev/inspect-ui.ts open-app`, load `app.html#/home`.
2. Attach 1 tab, then 3, and confirm the action list changes accordingly.
3. `snapshot` to see the rendered entries.

## Cross-references

- [`../suggestions/AGENTS.md`](../suggestions/AGENTS.md) — consumes this hook and builds the section.
- [`../AGENTS.md`](../AGENTS.md) — the new-tab data-layer rules.
- [`../../AGENTS.md`](../../AGENTS.md) — the screen that renders suggestions.
