# `lib/search-actions/` — Newtab → sidepanel handoff payload

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A single storage item, `searchActionsStorage`, holding the `SearchActionStorage`
payload that the newtab page writes when the user chooses "ask AI" from
search, and that the sidepanel reads when it opens. It is the durable half
of the handoff; the message channel that triggers the panel is
`../messaging/sidepanel/openSidepanelWithSearch.ts`.

## Contents

```
search-actions/
└── searchActionsStorage.ts   ← SearchActionStorage { query, mode: 'chat' | 'agent',
                               action?: ChatAction };
                               searchActionsStorage =
                               storage.defineItem<SearchActionStorage>('local:search-actions')
```

## Rules

- **SA1 — No `fallback` on this item.** It is a one-shot handoff slot, not a
  list; `getValue()` returning `undefined` is the normal "nothing pending"
  case, and the consumers branch on it. Don't add a `{}` fallback — that
  would make a stale empty payload indistinguishable from a real one.
- **SA2 — `ChatAction` comes from `../chat-actions/types`.** Import the type;
  the `AITabAction` / `BrowserOSAction` union is the shared contract between
  the newtab and the sidepanel.
- **SA3 — Persist before sending.** The payload is written to storage *and*
  pushed over the message channel, so a sidepanel that is already open reads
  it on mount.
- **SA4 — Clear after consuming.** A leftover payload replays the same query
  the next time the panel opens.
- **SA5 — `mode` is a two-value union** (`'chat' | 'agent'`); adding a mode
  means updating this type and the composer that branches on it.

## Workflows

**Sending a newtab search to the sidepanel**
1. `searchActionsStorage.setValue({ query, mode, action })`
2. `openSidePanelWithSearch.open(payload)` from
   `../messaging/sidepanel/`.
3. The sidepanel reads the same storage on mount.

**Consuming and clearing**
1. `const payload = await searchActionsStorage.getValue()`
2. If defined, seed the composer and `await searchActionsStorage.removeValue()`.
3. Treat `undefined` as "no pending handoff", not as an error.

**Adding a field to the handoff**
1. Extend `SearchActionStorage` here.
2. Extend the matching action type in `../chat-actions/types.ts`.
3. Update both consumers: the newtab search surface and the sidepanel.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2).
- [`../chat-actions/AGENTS.md`](../chat-actions/AGENTS.md) — `ChatAction`, `BrowserOSAction`.
- [`../messaging/sidepanel/AGENTS.md`](../messaging/sidepanel/AGENTS.md) — the message channel.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the newtab and sidepanel consumers.
