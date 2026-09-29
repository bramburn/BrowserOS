# `entrypoints/newtab/index/` — the new-tab omnibox screen

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/newtab`.

## What's here

The default new-tab page. `NewTab.tsx` is by far the largest file in this tree
(≈29 KB): a Downshift combobox where one input fans out into BrowserOS actions,
AI tab actions and Google search, plus attached-tab picking, voice input, provider
selection, MCP app selection, workspace selection, and a small results panel for
scheduled-task runs.

`NewTabChat.tsx` is the sibling route at `/home/chat` — the in-tab provider chat
that reuses the side panel's session machinery. `lib/` under this folder is the
feature-local data layer for the combobox (four small folders, each with its own
guide).

## Contents

```
index/
├── NewTab.tsx              ← the omnibox (combobox, mentions, selectors, voice, actions)
├── NewTabChat.tsx          ← `/home/chat` — provider chat inside the new tab
├── NewTabBranding.tsx      ← wordmark used at the top
├── NewTabTip.tsx           ← rotating tip card
├── ImportDataHint.tsx      ← "import from Chrome" callout
├── SignInHint.tsx          ← sign-in callout
├── SearchSuggestions.tsx   ← renders the suggestion sections from useSuggestions
├── TopSites.tsx            ← five most-visited tiles
├── ScheduleResults.tsx     ← recent scheduled-task runs (new-tab mirror)
├── ShortcutsDialog.tsx     ← keyboard-shortcut sheet (also mounted by app/layout/SidebarLayout)
├── tips.ts                 ← the tip copy
├── useTopSites.ts          ← chrome.topSites.get(), top 5, favicons via getFavicons
├── useActiveHint.ts        ← which hint (import / sign-in / none) to show
└── lib/                    ← suggestion sources (see below)
    ├── suggestions/        ← aggregation + item types  ← its own AGENTS.md
    ├── aiTabSuggestions/   ← tab-count-aware AI actions
    ├── browserOSSuggestions/← BrowserOS-mode suggestion
    └── searchSuggestions/  ← Google suggest endpoint via SWR
```

## Rules

- **NI1 — `NewTab.tsx` is a route component, not a data module.** Anything with state
  or an API call that is not about the combobox belongs in a hook file here or in
  `lib/`. The file is already the largest in the extension; do not grow it.
- **NI2 — Voice transcripts land in the combobox input,** not in a separate field.
  The effect depends on `voice.transcript` + `!voice.isTranscribing` and is
  deliberately excluded from exhaustive-deps; leave the biome-ignore in place.
- **NI3 — Attached tabs are a `chrome.tabs.Tab[]` held in component state** and are
  turned into an action via `createAITabAction` / `createBrowserOSAction` from
  `@/lib/chat-actions/types`. Never hand-roll the action shape.
- **NI4 — `useSuggestions` is the single aggregator.** `SearchSuggestions.tsx` renders
  sections; it does not call the source hooks itself.
- **NI5 — `chrome.topSites` needs the `topSites` permission** (already in
  `wxt.config.ts`) and returns at most 5 here via `take(urls, 5)`.
- **NI6 — `ShortcutsDialog` is shared with `../../app/layout/SidebarLayout.tsx`.**
  It lives here, not in `components/`; moving it breaks that import.
- **NI7 — Analytics constants are named per surface** (`NEWTAB_*`). Do not reuse
  `SIDEPANEL_*` events for new-tab actions; the two surfaces are measured separately.

## Workflows

**Adding a new top-sites tile behaviour**
1. Change `useTopSites.ts` (count, ordering, favicon fallback).
2. Update `TopSites.tsx` for layout only.
3. Track clicks with a new `NEWTAB_*` constant in `@/lib/constants/analyticsEvents`.

**Adding a new control to the omnibox**
1. Add the control component in this folder.
2. Wire it into `NewTab.tsx`'s selector row.
3. Keep the selected value in local state; persist through the matching
   `lib/<feature>/…Storage` module, not `chrome.storage` directly.
4. Fire a `NEWTAB_*` event on change.

**Sharing a hint with the agent home**
1. `ImportDataHint`, `SignInHint` and `useActiveHint` are already imported by
   `../../app/agent-command/AgentCommandHome.tsx`.
2. Changes to them affect both surfaces; check the agent home too.

**Debugging the new tab**
1. `bun scripts/dev/inspect-ui.ts open-app`, then load `app.html#/home`.
2. `snapshot` to get element ids, `click` / `fill` by id, `screenshot` to verify.
3. Check the console for capability errors — alpha-gated controls hide silently.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the new-tab route rules.
- [`./lib/suggestions/AGENTS.md`](./lib/suggestions/AGENTS.md) — the combobox aggregation layer.
- [`../../sidepanel/layout/AGENTS.md`](../../sidepanel/layout/AGENTS.md) — `ChatSessionContext` consumed by `NewTabChat` and `NewTab`.
- [`../../app/agent-command/AGENTS.md`](../../app/agent-command/AGENTS.md) — shares hints, cards and chat targets.
- [`../../../lib/chat-actions/types.ts`](../../../lib/chat-actions/types.ts) — action shapes.
- [`../../../lib/voice/useVoiceInput.ts`](../../../lib/voice/useVoiceInput.ts) — the voice hook.
