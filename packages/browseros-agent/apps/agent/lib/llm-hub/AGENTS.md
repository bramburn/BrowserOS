# `lib/llm-hub/` — Third-party LLM hub providers

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

The LLM Hub feature: a small, user-managed list of OpenAI-compatible
endpoints (name + base URL) the user adds themselves, as opposed to the
curated providers in `../llm-providers/`. Unlike most of `lib/`, this state
lives in **BrowserOS preferences** rather than `chrome.storage.local`, so
it is shared with the browser's own settings UI. `useLlmHubProviders` is
the optimistic CRUD hook around it.

## Contents

```
llm-hub/
├── storage.ts              ← LlmHubProvider { name, url };
│                             loadProviders() / saveProviders() read and write
│                             BROWSEROS_PREFS.THIRD_PARTY_LLM_PROVIDERS via
│                             getBrowserOSAdapter(), swallowing errors;
│                             getFaviconUrl(url, size) → google s2 favicons
└── useLlmHubProviders.ts   ← useLlmHubProviders(): { providers, isLoading,
                              saveProvider(provider, editIndex?),
                              deleteProvider(index) }
```

## Rules

- **LH1 — Persist through BrowserOS prefs, not `local:` storage.** The key is
  `browseros.third_party_llm.providers`, declared in `../browseros/prefs.ts`.
  Using `storage.defineItem` here would fork the user's data away from the
  browser.
- **LH2 — Reads and writes are best-effort and return safe defaults.**
  `loadProviders()` returns `[]` and `saveProviders()` returns `false` on
  failure; the hook re-reads storage when a save fails. Keep that contract.
- **LH3 — The last provider cannot be deleted.** `deleteProvider` early-returns
  when `providers.length <= 1`; that guard belongs in the hook, not the UI.
- **LH4 — `getFaviconUrl` normalises a bare host** by prefixing `https://`
  before `new URL()`. Reuse it instead of constructing favicon URLs elsewhere.
- **LH5 — Optimistic updates are the pattern here.** Set state, persist, and
  roll back by reloading on failure. Don't block the UI on the pref write.

## Workflows

**Adding a hub provider**
1. Read the list with `useLlmHubProviders().providers`.
2. Call `saveProvider({ name, url })`; pass `editIndex` to replace an entry.
3. Surface a save failure by re-reading — the hook already rolled back.

**Extending `LlmHubProvider`**
1. Add the field to the interface in `storage.ts`.
2. Confirm the BrowserOS pref can hold it — the value is JSON-serialised
   straight into a `chrome.browserOS` pref.
3. Check the consuming route in `entrypoints/app/llm-hub/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2, LIB3).
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — the adapter and `BROWSEROS_PREFS`.
- [`../llm-providers/AGENTS.md`](../llm-providers/AGENTS.md) — the curated provider set (different concern).
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the `llm-hub/` route that uses this hook.
