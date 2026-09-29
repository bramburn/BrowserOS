# `lib/personalization/` — User personalization prompt

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

One storage item and one hook for the free-text instruction the user
writes about themselves ("I work in Portuguese, be terse, …") that gets
injected into the agent's system prompt. The hook is a debounced,
watch-backed wrapper: it keeps local state in sync for instant typing,
suppresses the echo from its own writes, and persists after 300 ms of
inactivity.

## Contents

```
personalization/
└── personalizationStorage.ts  ← personalizationStorage
                                  (local:personalization, fallback '');
                                  usePersonalization() →
                                  { personalization, setPersonalization,
                                    clearPersonalization }
                                  - 300 ms debounce on writes
                                  - isLocalUpdate ref prevents the storage
                                    watch() from clobbering local edits
                                  - timer cleared on unmount
```

## Rules

- **PSN1 — Only this hook writes the value.** Direct
  `personalizationStorage.setValue()` from a component would race the
  debounce and the `isLocalUpdate` guard.
- **PSN2 — Keep the 300 ms debounce.** It is what stops a keystroke-rate
  write storm into `chrome.storage.local`; the `watch()` round trip is
  synchronous enough to feel laggy otherwise.
- **PSN3 — Preserve the `isLocalUpdate` ref.** Without it the storage
  watcher overwrites in-flight typing with a stale value on the next
  external write.
- **PSN4 — `clearPersonalization()` writes immediately** (no debounce) and
  also clears local state; the user pressed a button and expects it to be
  gone now.
- **PSN5 — The string is sent as `userSystemPrompt`**
  (`../messaging/server/buildChatRequestBody.ts`). It is user-authored, not
  system-authored — keep it in that field so the server treats it correctly.
- **PSN6 — The feature is version-gated** by
  `Feature.PERSONALIZATION_SUPPORT` in `../browseros/capabilities.ts`. Check
  it before rendering the settings UI.

## Workflows

**Wiring the settings field**
1. `const { personalization, setPersonalization, clearPersonalization } = usePersonalization()`
2. `onChange` → `setPersonalization(value)` (do not debounce again in the UI).
3. Gate the whole panel on `supports(Feature.PERSONALIZATION_SUPPORT)`.

**Resetting personalization**
1. Call `clearPersonalization()`.
2. The next chat request simply omits `userSystemPrompt`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB7).
- [`../messaging/server/AGENTS.md`](../messaging/server/AGENTS.md) — the `userSystemPrompt` field.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `Feature.PERSONALIZATION_SUPPORT`.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the `customization/` settings route.
