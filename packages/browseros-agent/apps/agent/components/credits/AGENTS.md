# `components/credits/` — Credit display

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

`CreditBadge.tsx` — a small pill button showing the remaining credit
count with a `Coins` icon. It is a leaf component: no hooks, no side
effects. It takes the number as a prop and pulls its colour class from
`lib/credits/credit-colors.ts`.

## Contents

```
credits/
└── CreditBadge.tsx   ← <button> + Coins icon + count; colour from
                        getCreditTextColor(); optional onClick
```

## Rules

- **CRD1 — Props only: `credits: number`, optional `onClick`.** Fetching the
  number is `useCredits()`'s job, in `lib/credits/`.
- **CRD2 — Colour comes from `getCreditTextColor()`, never a local
  conditional.** Thresholds (0, 30) live in `lib/credits/credit-colors.ts`;
  duplicating them here desynchronises the badge from every other surface.
- **CRD3 — The `title` attribute is the accessible label**
  (`"<n> credits remaining"`); keep it in sync with the rendered number.
- **CRD4 — `cn()` merges the hover styles.** Append new classes through `cn`,
  never by string-concatenating a `className`.

## Workflows

**Showing credits somewhere new**
1. Call `useCredits()` from `lib/credits/useCredits.ts` in the parent.
2. Render `<CreditBadge credits={data.credits} onClick={...} />`.
3. Do not fetch inside the badge.

**Changing the colour thresholds**
1. Edit `lib/credits/credit-colors.ts` (`getCreditTextColor` and
   `getCreditBarColor` together).
2. Never special-case a number here.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules.
- [`../../lib/credits/AGENTS.md`](../../lib/credits/AGENTS.md) — the query hook and colour thresholds.
- [`../ui/AGENTS.md`](../ui/AGENTS.md) — the `ui/` primitives available for restyling.
