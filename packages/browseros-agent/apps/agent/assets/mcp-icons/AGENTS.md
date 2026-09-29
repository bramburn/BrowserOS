# `assets/mcp-icons/` — MCP server icon set

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/assets`.

## What's here

One icon per supported MCP server, imported directly by
`entrypoints/app/connect-mcp/McpServerIcon.tsx`. 47 files: 44 SVG plus
three raster files (`exa.png`, `memo.webp`, `whatsapp.webp`) that exist
because the brand has no usable vector mark. Files are snake_case and
named after the server (`google_calendar.svg`, `brave_search.svg`), not
after the product's display name.

## Contents

```
mcp-icons/
├── SVG: airtable, asana, box, brave_search, cal_com, canva, clickup,
│        cloudflare, confluence, discord, dropbox, figma, github,
│        gitlab, gmail, google, google_calendar, google_docs_editors,
│        google_drive, google_forms, hubspot, intercom, jira, linkedin,
│        linear, microsoft_teams, mixpanel, monday, notion, onedrive,
│        outlook, posthog, postman, resend, salesforce, shopify,
│        slack, stripe, supabase, vercel, wordpress, youtube, zendesk
└── Raster: exa.png, memo.webp, whatsapp.webp
```

## Rules

- **MCI1 — An icon here is inert until `McpServerIcon.tsx` imports it.**
  Adding a file alone changes nothing in the UI; the `switch`/map in
  `entrypoints/app/connect-mcp/McpServerIcon.tsx` is the registry.
- **MCI2 — Filename must match the import path exactly**, snake_case, no
  spaces or capitals (`mcp-icons/google_drive.svg`, never `GoogleDrive.svg`).
- **MCI3 — Prefer SVG.** Only add a raster file when the vendor genuinely
  ships no vector mark, and name it after the server.
- **MCI4 — Don't re-draw or edit an existing brand icon.** These are third-party
  marks; replace the whole file only when the vendor rebrands.

## Workflows

**Adding an icon for a new MCP server**
1. Add `mcp-icons/<server>.svg` (snake_case).
2. Add the import and the `case`/map entry in
   `entrypoints/app/connect-mcp/McpServerIcon.tsx`.
3. Verify with `bun scripts/dev/inspect-ui.ts screenshot app /tmp/mcp.png`
   on the Connect Apps route.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the `assets/` parent.
- [`../../lib/mcp/AGENTS.md`](../../lib/mcp/AGENTS.md) — MCP server state.
- [`../../entrypoints/app/connect-mcp/AGENTS.md`](../../entrypoints/app/connect-mcp/AGENTS.md) — the consuming UI.
