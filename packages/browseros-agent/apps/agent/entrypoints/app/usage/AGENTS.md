# `entrypoints/app/usage/` — usage & billing (`/settings/usage`)

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/app`.

## What's here

A single read-only page showing BrowserOS AI credit usage: remaining credits, a
progress bar, and per-plan/per-window breakdowns. It is the smallest route folder in
`app/` — one component, no subfolders, no tests.

Data comes from `useCredits()` in `@/lib/credits/useCredits`. The service worker
invalidates those queries after a completed turn (see `useChatSession` in the
side panel), so a new page load is not needed to see updated usage.

## Contents

```
usage/
└── UsagePage.tsx   ← loading / error / success states; credit bar and breakdown tiles
```

## Rules

- **US1 — Three states, all handled explicitly:** `isLoading` → plain message,
  `error` → a non-destructive "unable to load" card, success → the numbers. Add a
  fourth state only with a matching UI; do not let `credits` default silently.
- **US2 — Read from `useCredits()` only.** No direct fetch, no local storage mirror.
  The hook owns invalidation.
- **US3 — Zero credits is a normal value, not an error.** `data?.credits ?? 0` is the
  current behaviour. Keep zero rendering as an empty bar, not as the error card.
- **US4 — Colours come from `@/lib/credits/credit-colors`** (`getCreditBarColor`,
  `getCreditTextColor`). Do not hardcode a green/amber/red threshold in JSX.
- **US5 — Add a nav entry** in `@/components/sidebar/SettingsSidebar` when the route
  is new; the page itself has no in-page navigation.

## Workflows

**Adding a usage metric**
1. Extend the credits query/response in `@/lib/credits/`.
2. Add the tile to the success branch of `UsagePage.tsx`.
3. If it needs a new threshold, add a helper to `@/lib/credits/credit-colors.ts`
   rather than branching in the component.
4. Confirm the server invalidates the credits query so the value is fresh.

**Debugging a stale number**
1. The hook is invalidated by the side panel's `useInvalidateCredits` after a turn.
2. Force a remount (`snapshot`/`reload` via the CDP inspector) to rule out caching
   before suspecting the query.

**Checking the page**
1. `bun scripts/dev/inspect-ui.ts open-app`
2. Navigate to `app.html#/settings/usage` and `screenshot`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — route table.
- [`../../../../lib/credits/`](../../../lib/credits/) — `useCredits`, `useInvalidateCredits`, colour helpers.
- [`../../../../components/sidebar/SettingsSidebar.tsx`](../../../components/sidebar/SettingsSidebar.tsx) — the nav entry.
- [`../../../sidepanel/index/AGENTS.md`](../../sidepanel/index/AGENTS.md) — where credits are invalidated after a turn.
