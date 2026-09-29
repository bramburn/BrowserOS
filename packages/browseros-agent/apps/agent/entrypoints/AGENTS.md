# `entrypoints/` — WXT entry point root

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`. This directory is
> the boundary between the WXT build and the extension's own source: WXT reads
> it at build time, and the folders inside it are a mix of *build entries* and
> *React route trees*. Telling those two apart is the single most important
> thing to get right here.

## What's here

Every file WXT treats as an extension surface, plus the component trees those
surfaces render. WXT discovers entries by convention, not by manifest editing:

| Pattern | Becomes | Found here |
|---|---|---|
| `<name>/index.html` | HTML page (side panel, tab page, options) | `app/`, `sidepanel/` |
| `<name>/index.ts` | Background service worker (MV3) — only `background/index.ts` uses this | `background/` |
| `<name>.ts`, `<name>.content.ts` | Content script | `content.ts`, `glow.content`, `auth.content`, `selection.content.ts` |

**There are exactly two HTML entries: `app/index.html` and `sidepanel/index.html`.**
Everything else in this tree is either the background worker, a content script,
or a plain React component folder consumed by a route table — not an entry point.

## Contents

```
entrypoints/
├── app/                 ← HTML entry #1 → app.html; HashRouter shell, chrome_url_overrides.newtab
├── sidepanel/           ← HTML entry #2 → sidepanel.html; the primary user surface
├── background/          ← background/index.ts, the MV3 service worker
├── content.ts           ← 3-line stub: matches *://*.google.com/*, empty main()
├── glow.content         ← content script for the "Glow" in-page feature
├── auth.content         ← content script for auth state detection
├── selection.content.ts ← content script for text-selection capture
├── newtab/              ← NOT an entry point — route components under /home
└── onboarding/          ← NOT an entry point — route components under /onboarding
```

## Rules

- **EP1 — Only `app/` and `sidepanel/` are HTML entry points.** Neither `newtab/`
  nor `onboarding/` has an `index.html`; they are route trees rendered inside
  `app.html`. Do not add an `index.html` to a route folder to "make it an entry
  point" — that creates a second, competing surface.
- **EP2 — Routes are hand-registered in `app/App.tsx`.** Creating
  `app/<route>/index.tsx` does nothing on its own; the component must be imported
  and mounted under a `<Route>`. An unmounted route folder is dead code. This
  corrects the auto-discovery advice in the parent guide, which does not hold here.
- **EP3 — `sidepanel/main.tsx` and `app/main.tsx` are near-duplicates.** Both wrap
  `AuthProvider` → `QueryProvider` → `AnalyticsProvider` → `ThemeProvider`. Change
  provider wiring in one and you must change the other; the nesting order is
  load-bearing.
- **EP4 — Keep `content.ts` a stub unless there is a reason not to.** It currently
  registers an empty `main()` for `*://*.google.com/*`. Adding real logic there is a
  choice, not an oversight — document it in this directory's sibling guides.
- **EP5 — Content scripts talk to the rest of the extension over `chrome.runtime`
  messaging**, not by importing `lib/` modules that assume a DOM page. The
  background worker is the only place that may hold long-lived state.
- **EP6 — The background worker is the only place that may hold long-lived state.**
  Anything scheduled or shared across tabs belongs in `background/index.ts` or
  `background/scheduledJobRuns.ts`, not in a content script or a page route.
- **EP7 — Build output is `dist/`, not `.output/`.** `wxt.config.ts` sets
  `outDir: 'dist'`. Load the unpacked extension from
  `apps/agent/dist/chrome-mv3/`.
- **EP8 — The background entry declares no `type`.** `background/index.ts` calls
  `defineBackground(() => {…})` with **no second argument**, so `background.type`
  in the generated manifest comes from WXT's own default, not from this repo.
  Do not add a `type: 'module'` argument or a matching claim to a guide without
  changing `defineBackground` first.

## Workflows

**Adding a new extension route**
1. Create the component under `app/<route>/` (not at the `entrypoints/` root).
2. Add the nav entry in `@/components/sidebar/AppSidebar` or `SettingsSidebar`.
3. Mount it in `app/App.tsx` under the appropriate layout.
4. Move state and side effects into `lib/<feature>/` — route components stay presentational.
5. Verify with `bun scripts/dev/inspect-ui.ts snapshot app` from `packages/browseros-agent`.

**Adding a new content script**
1. Create `entrypoints/<name>.content.ts` exporting `defineContentScript({ matches, main })`.
2. Add the match patterns to `wxt.config.ts` if the host permissions are not already declared.
3. Send data to the extension with `chrome.runtime.sendMessage`; do not import `lib/` modules
   that assume an extension-page context.

**Changing the background worker**
1. Edit `background/index.ts` (entry) — the file name and location are load-bearing for WXT.
2. Keep listener registration at the top level so MV3 can re-register the worker on wake.
3. Put scheduled-job persistence in `background/scheduledJobRuns.ts` rather than in a route.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension internals, rules X1–X10, dev/build commands.
- [`../wxt.config.ts`](../wxt.config.ts) — manifest, `outDir`, `chrome_url_overrides`, permissions.
- [`app/AGENTS.md`](app/AGENTS.md) — the app shell and full route table.
- [`sidepanel/AGENTS.md`](sidepanel/AGENTS.md) — the primary user surface.
- [`background/AGENTS.md`](background/AGENTS.md) — MV3 service worker.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — monorepo coding guidelines (authoritative).
