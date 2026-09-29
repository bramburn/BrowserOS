# `entrypoints/newtab/` — new-tab experience (routes inside `app.html`)

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

**This folder is not a WXT entry point.** There is no `index.html` here, and there
is no `/newtab` route. Everything in `newtab/` is a React route tree rendered by
`../app/App.tsx` inside `app.html`. `wxt.config.ts` sets
`chrome_url_overrides.newtab: 'app.html'` (no hash); `App.tsx` then redirects the
bare path with `<Route path="/" element={<Navigate to="/home" replace />} />`, so
the new tab actually lands on `app.html#/home`.

`App.tsx` picks the `/home` branch on `Feature.ALPHA_FEATURES_SUPPORT`, so what
lands here changes:

| Path | Component | Branch |
|---|---|---|
| `/home` (index) | `index/NewTab.tsx`, the classic new tab | non-alpha |
| `/home/chat` | `index/NewTabChat.tsx` | alpha |
| `/home/personalize` | `personalize/Personalize.tsx` | alpha |
| `/home` (index) | `../app/agent-command/AgentCommandHome.tsx` (not this folder) | alpha |
| `/home/agents/:agentId` | `../app/agent-command/AgentCommandConversation.tsx` (not this folder) | alpha |

The visible new tab is the omnibox: a combobox that fans a query out into
BrowserOS actions, tab actions, and Google suggestions, with a voice microphone,
attached-tab picker, and provider/MCP/workspace selectors. `index/lib/` is the
feature-local data layer for that combobox.

## Contents

```
newtab/
├── index/                 ← NewTab, NewTabChat, suggestions lib, top sites, hints
│   └── lib/               ← one folder per suggestion source (see its own AGENTS.md)
├── layout/                ← NewTabLayout (outlet + focus grid + optional chat session)
└── personalize/           ← `/home/personalize` markdown-based user profile editor
```

## Rules

- **NT1 — Do not add an `index.html` here.** This is a route tree, not a WXT HTML
  entry. If you find yourself adding one you are about to create a second new-tab
  override.
- **NT2 — A new screen here must be registered in `../app/App.tsx`,** inside the
  `home` branch. Unregistered files are dead code.
- **NT3 — The combobox is one input with three suggestion sources.** Add a source in
  `index/lib/suggestions/useSuggestions.ts` with a typed item in `index/lib/suggestions/types.ts`,
  plus a `getSuggestionLabel` case. Skipping the label case breaks selection.
- **NT4 — Section priority is explicit:** BrowserOS actions first, then AI tab
  actions *instead of* Google search (they are an `if/else`, not both). Preserve that
  when adding a section.
- **NT5 — `/home/chat` is the only route that mounts a `ChatSessionProvider` here,** and
  only when `useChatSessionOnHome` is set (non-alpha). The predicate lives in
  `layout/route-utils.ts` and is unit-tested.
- **NT6 — `ChatSessionContext` is imported from `../sidepanel/layout/`.** That is
  deliberate cross-surface reuse; do not fork a new-tab copy of the chat session.
- **NT7 — Google suggestions are fetched from the page** and carry a standing TODO to
  move to the background service worker to avoid CORS. Keep the TODO with the code.

## Workflows

**Adding a suggestion source**
1. Create `index/lib/<name>/use<Name>Suggestions.ts` returning the raw shape.
2. Add a `*SuggestionItem` variant in `index/lib/suggestions/types.ts`.
3. Map it into a `SuggestionSection` in `index/lib/suggestions/useSuggestions.ts`.
4. Add the `getSuggestionLabel` case.
5. Render it in `index/SearchSuggestions.tsx`.

**Adding a route under `/home`**
1. Create the component in a folder here.
2. Import it in `../app/App.tsx` and add the `<Route>` inside the `home` branch,
   respecting the alpha gate if it is an agent feature.
3. Update `layout/route-utils.ts` if it needs the focus grid hidden or a chat session
   mounted, and add a test case in `layout/route-utils.test.ts`.

**Changing the new-tab background**
1. `layout/NewTabFocusGrid.tsx` renders `bg-grid-pattern` + `bg-gradient-radial-focus`.
2. Suppress it per-path through `shouldHideFocusGrid` in `layout/route-utils.ts`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals.
- [`../app/AGENTS.md`](../app/AGENTS.md) — the route table that mounts this folder.
- [`../sidepanel/AGENTS.md`](../sidepanel/AGENTS.md) — the primary surface; shares `ChatSessionContext`.
- [`../../wxt.config.ts`](../../wxt.config.ts) — `chrome_url_overrides.newtab = 'app.html'` (no hash; `App.tsx` redirects `/` → `/home`).
- [`../../lib/personalization/personalizationStorage.ts`](../../lib/personalization/personalizationStorage.ts) — personalize storage.
