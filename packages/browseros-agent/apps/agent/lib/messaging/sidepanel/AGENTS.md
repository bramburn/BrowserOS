# `lib/messaging/sidepanel/` — Newtab → sidepanel handoff channel

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib/messaging`.

## What's here

One file, `openSidepanelWithSearch.ts`: a single-method message channel
that asks the background service worker to open the BrowserOS side panel
and hand it a prepared search/action payload. The newtab page sends it when
the user picks "ask AI" from the omnibox-style search; the sidepanel reads
the same payload back out of `../../search-actions/searchActionsStorage`.

## Contents

```
sidepanel/
└── openSidepanelWithSearch.ts
    ├── OpenSidePanelWithSearchParams { open(props: SearchActionStorage): void }
    └── aliases: openSidePanelWithSearch, onOpenSidePanelWithSearch
```

## Rules

- **SPMSG1 — The payload type is `SearchActionStorage`, imported from
  `../../search-actions/searchActionsStorage`.** Never redeclare the
  `{ query, mode, action? }` shape here; that is the contract both ends share.
- **SPMSG2 — The method is `open`, and that verb is the API.** Renaming it
  means updating every sender, which is a rename, not an edit.
- **SPMSG3 — Use the aliases** (`openSidePanelWithSearch`, not raw
  `sendMessage`) so the channel is identifiable at the call site.
- **SPMSG4 — Opening the panel is a background concern.** The sender does not
  call `chrome.sidePanel` itself; it sends the message and
  `entrypoints/background/` performs the toggle (see
  `../../browseros/toggleSidePanel.ts`).
- **SPMSG5 — Persist then send.** The payload goes into
  `searchActionsStorage` as well, so a sidepanel that is already open reads it
  on mount instead of relying on the message arriving.

## Workflows

**Sending the user from the newtab into the sidepanel**
1. Write the payload: `searchActionsStorage.setValue({ query, mode, action })`.
2. `await openSidePanelWithSearch.open(payload)`
3. Let the background worker open the panel.

**Consuming the handoff in the sidepanel**
1. `onOpenSidePanelWithSearch.open(handler)` to receive it live.
2. Also read `searchActionsStorage` on mount to cover the already-open case.
3. Clear the storage value after consuming so it doesn't replay.

**Adding a field to the handoff**
1. Extend `SearchActionStorage` in `../../search-actions/searchActionsStorage.ts`.
2. Extend the matching action type in `../../chat-actions/types.ts`.
3. Re-check both consumers.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — messaging rules (MSG1–MSG5).
- [`../../search-actions/AGENTS.md`](../../search-actions/AGENTS.md) — the payload type and storage.
- [`../../chat-actions/AGENTS.md`](../../chat-actions/AGENTS.md) — `ChatAction` / `BrowserOSAction`.
- [`../../browseros/AGENTS.md`](../../browseros/AGENTS.md) — `toggleSidePanel.ts`.
- [`../../entrypoints/AGENTS.md`](../../../entrypoints/AGENTS.md) — the background handler.
