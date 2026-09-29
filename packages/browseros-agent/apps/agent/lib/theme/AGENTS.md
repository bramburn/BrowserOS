# `lib/theme/` — Theme preference

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/lib`.

## What's here

One file, `theme-storage.ts`: the persisted theme choice and its type.
The union is exactly `'light' | 'dark' | 'system'`, defaulting to
`'system'`. The React plumbing — reading it, applying the `dark` class to
`<html>`, and following `prefers-color-scheme` — lives in
`components/theme-provider.tsx`; the picker is
`components/elements/theme-toggle.tsx`.

## Contents

```
theme/
└── theme-storage.ts   ← type Theme = 'light' | 'dark' | 'system';
                         themeStorage = storage.defineItem<Theme>
                         ('local:theme', { fallback: 'system' })
```

## Rules

- **THM1 — Three values, no more.** `next-themes` is a dependency but this
  project uses its own provider; don't introduce a second theme source.
- **THM2 — `fallback: 'system'` is the default**, and
  `components/theme-provider.tsx` treats an absent value as `system` too.
  Changing the fallback changes the first-run experience for every user.
- **THM3 — Import the `Theme` type from here**, never redeclare
  `'light' | 'dark' | 'system'` in a component. `theme-toggle.tsx` and
  `theme-provider.tsx` both do.
- **THM4 — The stored key is `local:theme`.** Renaming it silently resets
  every user's preference; if you must change it, add a migration.
- **THM5 — Apply the theme by toggling the `dark` class on
  `document.documentElement`.** `styles/global.css` declares
  `@custom-variant dark (&:is(.dark *))`; a media query would not match.

## Workflows

**Adding a theme option**
1. Extend the `Theme` union in `theme-storage.ts`.
2. Handle it in `components/theme-provider.tsx` (it currently special-cases
   `system`).
3. Add the icon/label entry to `components/elements/theme-toggle.tsx`.
4. Confirm `.dark` and `:root` token blocks in `styles/global.css` cover it.

**Resetting to the OS preference**
1. `themeStorage.setValue('system')` — the provider re-applies on `watch`.
2. No migration concerns; `system` has always been valid.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — lib rules (LIB2).
- [`../../components/theme-provider.tsx`](../../components/theme-provider.tsx) — the consumer.
- [`../../components/elements/theme-toggle.tsx`](../../components/elements/theme-toggle.tsx) — the picker.
- [`../../styles/AGENTS.md`](../../styles/AGENTS.md) — the `.dark` variant and tokens.
