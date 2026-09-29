# `assets/` — Imported image assets

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

Source image files that Vite resolves through the `@/assets/*` alias and
inlines as URL modules at build time. This is the **bundle** asset dir
(WXT's default `assetsDir`), not `public/` — anything placed in `public/`
is served verbatim instead. The directory holds brand marks and social
logos, plus the `mcp-icons/` subfolder of per-integration icons.

## Contents

```
assets/
├── product_logo.svg          ← BrowserOS wordmark. Imported by
│                              components/sidebar/SidebarBranding.tsx,
│                              lib/llm-providers/providerIcons.tsx,
│                              entrypoints/app/layout/AuthLayout.tsx,
│                              entrypoints/app/ai-settings/LlmProvidersHeader.tsx,
│                              entrypoints/newtab/index/NewTabBranding.tsx,
│                              entrypoints/onboarding/index/OnboardingHeader.tsx
├── discord-logo.svg          ← social/community links
├── github-logo.svg           ← social/community links
├── slack-logo.svg            ← social/community links
├── google_chrome_logo.svg    ← Chrome-store / compatibility copy
├── react.svg                 ← unused scaffold leftover (no importer in apps/agent)
└── mcp-icons/                ← per-MCP-server icons (see its own AGENTS.md)
```

## Rules

- **AST1 — Import with the `@/assets/...` alias, never a relative path.** All six
  root SVGs are consumed as `import ProductLogo from '@/assets/product_logo.svg'`.
- **AST2 — `@/assets/*` is bundled, `public/*` is served verbatim.** Choose
  `assets/` when the file must be inlined/hashed, `public/` when a stable runtime
  URL (manifest icon, font file) is required.
- **AST3 — Naming is inconsistent by history; match the existing file you extend.**
  Root logos use kebab-case (`product_logo.svg` is the exception), `mcp-icons/`
  uses snake_case (`google_calendar.svg`). Don't normalise existing names —
  the imports would break.
- **AST4 — Don't add raster art here.** PNG/WebP belongs in
  `mcp-icons/` only when there is no vector equivalent of the brand mark.

## Workflows

**Adding a new brand/social logo**
1. Drop the SVG in `assets/` using the kebab-case name of the brand.
2. Import it in the consuming component as `import X from '@/assets/x.svg'`.
3. No manifest or config change is needed — Vite handles the rest.

**Adding an icon for a new MCP server**
1. Add the file to `assets/mcp-icons/` named after the server.
2. Wire it in `entrypoints/app/connect-mcp/McpServerIcon.tsx`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root.
- [`mcp-icons/AGENTS.md`](mcp-icons/AGENTS.md) — MCP server icon set.
- [`../public/AGENTS.md`](../public/AGENTS.md) — the served-verbatim counterpart.
