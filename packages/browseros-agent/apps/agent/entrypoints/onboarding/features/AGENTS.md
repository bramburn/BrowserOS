# `entrypoints/onboarding/features/` — feature tour

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/onboarding`.

## What's here

A marketing-style bento grid of the product's features — the AI agent, the MCP
server, agentic coding, split view, workspaces — each with an icon, a tag, a short
and long description, a highlight list, an optional demo video, and a grid span.
`BentoCard` is the card; `VideoFrame` is the video wrapper. `Features.tsx` exports
the page component as `FeaturesPage` (note the name differs from the file).

Route: `/onboarding/features`. All copy, media URLs, and external links come from
`@/lib/constants/mediaUrls` and `@/lib/constants/productUrls` — nothing is
hard-coded inline.

## Contents

```
features/
├── Features.tsx     ← exports `FeaturesPage`; the `features: Feature[]` catalogue
├── BentoCard.tsx    ← exports `BentoCard` and the `Feature` type
└── VideoFrame.tsx   ← embedded demo video/GIF frame
```

## Rules

- **FT1 — The feature catalogue is a `Feature[]` literal in `Features.tsx`.** Add a
  feature by appending an entry with an `id`, `Icon`, `tag`, `title`,
  `description`, `detailedDescription`, `highlights`, `videoDuration` and
  `gridClass`. The card renders from that shape only.
- **FT2 — `gridClass` is the layout contract.** `md:col-span-2` style strings are what
  make the bento work; a new feature that spans oddly is a `gridClass` problem, not a
  CSS problem.
- **FT3 — Media and product URLs come from the constants modules.** Do not inline
  URLs in the catalogue — the CDN and repo links are updated centrally.
- **FT4 — `videoDuration` is a display string** (`'2:22'`), not a number. Keep it as a
  string so zero-padding decisions stay local to the card.
- **FT5 — `FeaturesPage` is the export name**, not `Features`. `app/App.tsx` imports
  `{ FeaturesPage }` from this file; renaming breaks the route.
- **FT6 — This page is promotional, not functional.** No state, no storage, no API
  calls. If you find yourself adding any, it belongs in `../index/` or `../steps/`.

## Workflows

**Adding a feature card**
1. Append a `Feature` entry in `Features.tsx` with an id and a `lucide-react` icon.
2. Pick a `gridClass` that fits the composition (span 2 for the hero entries).
3. Add the demo video to `@/lib/constants/mediaUrls` and reference it by name.
4. Check the layout at `md` and above; below `md` everything stacks.

**Editing copy**
1. `description` is the collapsed blurb; `detailedDescription` is the expanded one —
   they are different lengths on purpose.
2. `highlights` renders as a bullet list; keep entries parallel in voice and length.

**Verifying**
1. `bun scripts/dev/inspect-ui.ts open-app`, load `app.html#/onboarding/features`.
2. `screenshot` at a wide viewport to check the bento composition.
3. Confirm every video frame loads (they are external URLs).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the onboarding route rules.
- [`../index/AGENTS.md`](../index/AGENTS.md) — the welcome screen that links here.
- [`../../../../lib/constants/mediaUrls.ts`](../../../lib/constants/mediaUrls.ts) — demo video/GIF URLs.
- [`../../../../lib/constants/productUrls.ts`](../../../lib/constants/productUrls.ts) — GitHub/Slack/Discord/docs links.
