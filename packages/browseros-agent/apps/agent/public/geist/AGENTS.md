# `public/geist/` — Geist Sans variable font

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/public`.

## What's here

Two variable-weight WOFF2 files for the **Geist Sans** UI typeface:

```
geist/
├── Geist[wght].woff2          ← roman, weight axis 100–900
└── Geist-Italic[wght].woff2   ← italic, weight axis 100–900
```

WXT copies them verbatim into the build root, so they are served at
`/geist/Geist[wght].woff2` and `/geist/Geist-Italic[wght].woff2`.

## Rules

- **GST1 — Two files only.** No `@font-face` is defined here; the declarations
  live in `styles/global.css` under the `Geist` family. Add CSS there, not here.
- **GST2 — Variable fonts: never add a `font-weight` list.** The `[wght]`
  axis is declared once as `font-weight: 100 900` in `global.css`.
- **GST3 — The `[` / `]` in the filename are literal.** Renaming the file
  breaks the `url()` in `styles/global.css` silently — update both together.
- **GST4 — Keep the roman and italic pair in sync.** A weight-axis change to
  one without the other produces a mismatched italic fallback.

## Workflows

**Upgrading Geist**
1. Replace both `.woff2` files, keeping the exact filenames (or update
   `styles/global.css` in the same change).
2. Confirm the `font-family: "Geist"` rules in `styles/global.css` still
   resolve — no other change is needed.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `public/` conventions.
- [`../../styles/AGENTS.md`](../../styles/AGENTS.md) — the `@font-face` rules.
- [`../geist-mono/AGENTS.md`](../geist-mono/AGENTS.md) — the monospace sibling.
