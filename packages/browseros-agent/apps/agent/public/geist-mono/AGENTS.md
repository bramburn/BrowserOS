# `public/geist-mono/` — Geist Mono variable font

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/public`.

## What's here

Two variable-weight WOFF2 files for the **Geist Mono** monospace typeface:

```
geist-mono/
├── GeistMono[wght].woff2         ← roman, weight axis 100–900
└── GeistMono-Italic[wght].woff2  ← italic, weight axis 100–900
```

WXT copies them verbatim into the build root, so they are served at
`/geist-mono/GeistMono[wght].woff2` and
`/geist-mono/GeistMono-Italic[wght].woff2`.

## Rules

- **GMT1 — The `font-family` is `"Geist Mono"` (with a space), not
  `GeistMono`.** That string is what `styles/global.css` declares; changing
  one without the other falls back to the system monospace.
- **GMT2 — Variable font: declare the weight range `100 900` once, never a
  per-weight `src` list.**
- **GMT3 — Filenames are bracket-bearing and literal.** Any rename must be
  paired with a `styles/global.css` edit in the same change.
- **GMT4 — `font-mono` utilities resolve here.** Tailwind's default
  `font-mono` stack starts with `"Geist Mono"`; don't re-point it.

## Workflows

**Upgrading Geist Mono**
1. Replace both `.woff2` files under the same names.
2. Re-check the `font-family: "Geist Mono"` block in `styles/global.css`
   still resolves in the built extension.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `public/` conventions.
- [`../../styles/AGENTS.md`](../../styles/AGENTS.md) — the `@font-face` rules.
- [`../geist/AGENTS.md`](../geist/AGENTS.md) — the sans-serif sibling.
