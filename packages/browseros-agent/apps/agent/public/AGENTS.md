# `public/` — Served-verbatim static files

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

Files WXT copies **verbatim** into the built extension root — no bundling,
no hashing, no import. They are reachable at their path from inside the
extension and from the generated `manifest.json`. That makes this the
right home for the toolbar icon set (`icon/`) and the variable webfonts
(`geist/`, `geist-mono/`). `wxt.svg` is an unused scaffold leftover.

## Contents

```
public/
├── wxt.svg                 ← WXT scaffold leftover; no importer in apps/agent
├── icon/                   ← manifest + toolbar PNG set (see icon/AGENTS.md)
│   ├── 16.png, 32.png, 48.png, 96.png, 128.png
├── geist/                  ← Geist Sans variable woff2 (see geist/AGENTS.md)
│   ├── Geist[wght].woff2
│   └── Geist-Italic[wght].woff2
└── geist-mono/             ← Geist Mono variable woff2
    ├── GeistMono[wght].woff2
    └── GeistMono-Italic[wght].woff2
```

## Rules

- **PUB1 — `public/` means "copy as-is"; `assets/` means "bundle it".** A file
  needed at a stable runtime URL (manifest reference, font) goes here; a file
  consumed by JS/TSX goes in `../assets/`.
- **PUB2 — Never `import` from `public/`.** Use an absolute runtime URL
  (`/geist/Geist[wght].woff2`) if you reference one from code. The only
  in-repo reference is the `@font-face` block in `../styles/global.css`.
- **PUB3 — Filenames are case- and bracket-sensitive.** `Geist[wght].woff2`
  contains literal `[`/`]`; renaming requires editing `styles/global.css`
  in the same commit.
- **PUB4 — The manifest only names `icon/16|32|48|128.png`.** `96.png` ships
  but is not wired into `wxt.config.ts`; keep it or wire it, don't assume
  it is referenced.

## Workflows

**Adding a font file**
1. Drop the file under `public/<family>/`.
2. Add or update the matching `@font-face` rule in `styles/global.css`.
3. Confirm the `url()` path matches the `public/` subdirectory exactly.

**Changing the toolbar icon**
1. Replace `icon/16.png`, `32.png`, `48.png`, `128.png` (and `96.png`).
2. The `action.default_icon` map in `../wxt.config.ts` already points at
   these paths — no config change needed for a same-name swap.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root.
- [`../assets/AGENTS.md`](../assets/AGENTS.md) — the bundled-assets counterpart.
- [`../styles/AGENTS.md`](../styles/AGENTS.md) — the only consumer of the fonts.
- [`../wxt.config.ts`](../wxt.config.ts) — manifest `action.default_icon`.
