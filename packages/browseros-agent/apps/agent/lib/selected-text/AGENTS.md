# `lib/selected-text/` — Per-tab text selection

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A single storage item mapping **tab id → the text the user selected on
that tab**, captured by the `selection.content.ts` content script so the
sidepanel can offer "ask about this selection". Each tab's selection is
independent, which is why this is a record rather than one slot.

## Contents

```
selected-text/
└── selectedTextStorage.ts   ← SelectedTextData { text, pageUrl, pageTitle,
                               tabId, timestamp };
                               selectedTextStorage =
                               storage.defineItem<Record<string, SelectedTextData>>
                               ('local:selectedTextMap', { defaultValue: {} })
```

## Rules

- **STT1 — Keyed by tab id as a string.** `Record<string, SelectedTextData>`
  because `chrome.tabs.Tab['id']` is a number that may be `undefined`; the
  map avoids that problem. Look-ups do `selectedTextMap[String(tabId)]`.
- **STT2 — This is the one item that uses `defaultValue`, not `fallback`.**
  The `@wxt-dev/storage` option is `defaultValue`; copying the `fallback:`
  shape from the other storage modules here is a bug.
- **STT3 — Clear the tab's entry when the panel consumes it**, and when a
  tab is closed. The map otherwise accumulates stale selections with old
  `timestamp`s.
- **STT4 — Captured text is untrusted page content.** It travels into the
  chat request as `selectedText` / `selectedTextSource`
  (`../messaging/server/buildChatRequestBody.ts`) — never as a system
  prompt, and never logged with its contents.
- **STT5 — Cap the captured length at the capture site** (the content
  script). `chrome.storage.local` is a shared, size-capped store.

## Workflows

**Recording a selection**
1. The content script writes
   `selectedTextMap[String(tabId)] = { text, pageUrl, pageTitle, tabId, timestamp }`
2. Read-modify-write the whole record; there is no per-tab storage item.

**Reading the selection for a prompt**
1. `const map = await selectedTextStorage.getValue()`
2. `const selection = map[String(tabId)]`
3. Undefined means "nothing selected in this tab" — omit the field.

**Clearing a selection**
1. Delete the tab's key and `setValue` the remaining record.
2. Also clear on tab close so the map doesn't grow unbounded.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2).
- [`../messaging/server/AGENTS.md`](../messaging/server/AGENTS.md) — the `selectedText` request field.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — `selection.content.ts`, the capture side.
