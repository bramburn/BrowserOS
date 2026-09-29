# `styles/` — Global CSS and design tokens

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

One file, `global.css` — the single Tailwind v4 entry point for the whole
extension. It imports Tailwind and the third-party plugin layers, declares
the Geist / Geist Mono `@font-face` rules, and defines every design token
as a CSS custom property in OKLCH under `:root` and `.dark`. It is named
in `components.json` (`"css": "styles/global.css"`) and imported by
`entrypoints/sidepanel/main.tsx` and `entrypoints/app/main.tsx`.

## Contents

```
styles/
└── global.css   ← Tailwind imports, @custom-variant dark, @font-face ×4,
                   :root tokens, .dark overrides, base/global layer rules
```

## Rules

- **STY1 — This is the only global stylesheet; do not add a second one.**
  Both extension entry points import it by the `@/styles/global.css` alias.
- **STY2 — Tokens are OKLCH custom properties (`--primary`, `--muted-foreground`,
  …), not hex.** Add a token to **both** `:root` and `.dark`, or components
  break in the other theme.
- **STY3 — Dark mode is class-based**, wired by
  `@custom-variant dark (&:is(.dark *))` and toggled by
  `components/theme-provider.tsx`. Never assume a `prefers-color-scheme`
  media query.
- **STY4 — Tailwind v4 syntax only** — `@import "tailwindcss"`, `@plugin`,
  `@theme`, `@custom-variant`. There is no `tailwind.config.js` in this app.
- **STY5 — No CSS-in-JS and no component-scoped `<style>`.** Styling lives in
  `className` with `cn()` from `lib/utils.ts`.

**Known discrepancy — font paths:** the four `@font-face` rules declare
`url("../geist/Geist[wght].woff2")` and `url("../geist-mono/GeistMono[wght].woff2")`.
Relative to `styles/global.css` those resolve to `apps/agent/geist/` and
`apps/agent/geist-mono/`, but the WOFF2 files actually live in
`public/geist/` and `public/geist-mono/`. If you touch a font file, verify the
`url()` path rather than assuming it is right.

## Workflows

**Adding a design token**
1. Add the custom property to `:root` in `global.css`.
2. Add the dark-mode counterpart inside the `.dark` block.
3. Consume it via Tailwind's token utilities (`bg-primary`, `text-muted-foreground`).

**Changing a colour**
1. Edit the OKLCH value in `global.css` only — components reference tokens, not literals.
2. Check both themes; some components hard-code Tailwind palettes
   (`text-green-500` in `lib/credits/credit-colors.ts`, status icon colours in
   `components/execution-history/`) and will not follow a token change.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root.
- [`../components.json`](../components.json) — declares this file as the Tailwind CSS entry.
- [`../components/theme-provider.tsx`](../components/theme-provider.tsx) — writes the `.dark` class.
- [`../lib/theme/theme-storage.ts`](../lib/theme/theme-storage.ts) — persists the theme choice.
- [`../public/geist/AGENTS.md`](../public/geist/AGENTS.md) — the font files themselves.
