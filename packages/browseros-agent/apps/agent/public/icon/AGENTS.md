# `public/icon/` — Extension icon PNGs

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/public`.

## What's here

The raster icon set referenced by the generated `manifest.json`:

```
icon/
├── 16.png    ← toolbar icon
├── 32.png    ← toolbar icon
├── 48.png    ← toolbar icon
├── 96.png    ← shipped but not referenced by wxt.config.ts
└── 128.png   ← toolbar icon / store listing size
```

`wxt.config.ts` sets `manifest.action.default_icon` to
`{16, 32, 48, 128: 'icon/<n>.png'}`, and `default_title: 'Ask BrowserOS'`.
These paths are relative to the build root, which is why the files live
in `public/`.

## Rules

- **ICN1 — Add to the `default_icon` map, don't just drop a PNG.** A file
  not listed in `wxt.config.ts` is inert.
- **ICN2 — Ship the full ladder.** Changing the design means regenerating
  16/32/48/128 together; a mismatched set reads as a broken toolbar icon.
- **ICN3 — Keep `96.png` in the directory.** It is not manifest-referenced
  today, but removing it silently diverges the folder from the other
  generated sizes.
- **ICN4 — The extension ID is pinned by `manifest.key` in `wxt.config.ts`.**
  Regenerating icons does not change the ID; changing the key does.

## Workflows

**Replacing the extension icon**
1. Regenerate `16/32/48/96/128.png` from the source artwork.
2. Build (`bun run build` from `packages/browseros-agent`) and confirm
   `dist/manifest.json` still points at `icon/<n>.png`.
3. Reload the unpacked extension in `chrome://extensions`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `public/` conventions.
- [`../../wxt.config.ts`](../../wxt.config.ts) — `manifest.action.default_icon`.
- [`../../package.json`](../../package.json) — `version`, which is the `release`
  tag Sentry reports use.
