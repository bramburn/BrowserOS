# `lib/credits/` — Credit balance query and thresholds

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

A read-only view of the user's credit balance from the local agent
server's `/credits` endpoint, plus the two colour helpers that decide how
a balance is presented. `useCredits` is a thin TanStack Query wrapper with
a 30 s stale time and refetch-on-focus; `credit-colors.ts` holds the
thresholds so the badge, the bar, and any future surface agree.

## Contents

```
credits/
├── useCredits.ts    ← CreditsInfo { credits, dailyLimit, lastResetAt? };
│                      CREDITS_QUERY_KEY = ['credits'];
│                      fetchCredits() GETs {agentServerUrl}/credits;
│                      useCredits() → useQuery { staleTime 30_000,
│                      refetchOnWindowFocus, retry 1 };
│                      useInvalidateCredits() → queryClient.invalidateQueries
└── credit-colors.ts ← LOW_THRESHOLD = 30;
                        getCreditTextColor(credits) → text-red/yellow/green-500;
                        getCreditBarColor(credits)  → bg-red/yellow/green-500
```

## Rules

- **CRD-L1 — This is the one place `/credits` is called.** It deliberately
  uses `fetch` rather than the Hono client, and it is a `lib/` module, so it
  satisfies the no-direct-fetch-in-components rule. Don't call it from a
  component.
- **CRD-L2 — `CREDITS_QUERY_KEY` is the invalidation handle.** Use
  `useInvalidateCredits()` after any action that spends credits (sending a
  message, running a scheduled job). Don't invent a second key shape.
- **CRD-L3 — The base URL comes from `getAgentServerUrl()`**, not a literal
  port.
- **CRD-L4 — Text and bar colours are separate functions on purpose** —
  different utility prefixes for the same thresholds. Change both together.
- **CRD-L5 — `LOW_THRESHOLD` is a single constant.** Duplicating `30` in a
  component desynchronises the badge from every other balance surface.
- **CRD-L6 — The feature is version-gated** by `Feature.CREDITS_SUPPORT`
  (`minServerVersion 0.0.78`). Check `useCapabilities()` before rendering.

## Workflows

**Displaying the balance**
1. `const { data } = useCredits()`; `data` is `undefined` while loading.
2. Render `<CreditBadge credits={data.credits} />` from
   `components/credits/`.
3. Use `getCreditTextColor` / `getCreditBarColor` for any custom rendering.

**Refreshing after an action**
1. `const invalidate = useInvalidateCredits()`
2. `await runTheAction(); invalidate()`

**Changing the warning threshold**
1. Edit `LOW_THRESHOLD` in `credit-colors.ts` only.
2. Both colour functions pick it up.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB3, LIB7).
- [`../../components/credits/AGENTS.md`](../../components/credits/AGENTS.md) — `CreditBadge`, the renderer.
- [`../browseros/AGENTS.md`](../browseros/AGENTS.md) — `getAgentServerUrl()` and `Feature.CREDITS_SUPPORT`.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the `usage/` route.
