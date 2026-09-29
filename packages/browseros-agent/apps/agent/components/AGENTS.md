# `components/` — Presentational React components

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

Every React component in the extension that is not a route or an entry
point. The rule that governs this tree: **`components/` is presentational;
side effects, storage, API calls and stateful logic live in `lib/`.** A
component may import from `lib/`, but it should not call `fetch`,
`chrome.storage`, or a browser API directly. (The existing code is not
fully pure here — see CMP1.)

## Contents

```
components/
├── theme-provider.tsx        ← ThemeProvider + useTheme(); writes the
│                              `dark` class on <html>, persists via
│                              lib/theme/theme-storage.ts
├── ui/                        ← vendored shadcn/ui primitives (33 files)
├── ai-elements/               ← vendored ai-elements chat primitives (30 files)
├── elements/                  ← first-party product widgets: tab picker,
│                               workspace selector, app selector, theme
│                               toggle, laser + glowing-border animations
├── sidebar/                   ← app + settings navigation chrome
├── chat/                      ← unified provider/agent picker + its types
├── credits/                   ← CreditBadge
├── execution-history/         ← collapsible task/step cards
└── auth/                      ← AuthGuard route guard
```

## Rules

- **CMP1 — No side effects in this tree.** Storage writes, `fetch`, SSE, and
  `chrome.*` calls belong in `lib/<feature>/`. A component that needs them
  imports a hook from `lib/`. Exceptions that already exist and are
  deliberate: `components/elements/use-available-tabs.ts` and
  `components/sidebar/SidebarBranding.tsx` (documented in their own files).
- **CMP2 — `ui/` and `ai-elements/` are vendored; `elements/` is ours.** Do not
  hand-edit the vendored two. Put new product UI in `elements/` or a feature
  folder.
- **CMP3 — Merge `cn()` from `@/lib/utils` for every conditional class.**
  Every file in this tree does this; a raw template string for
  `className` is a review blocker.
- **CMP4 — Icon set is `lucide-react`.** Third-party brand glyphs
  (`@lobehub/icons`, `assets/*.svg`) are the only exceptions.
- **CMP5 — Route components are NOT here.** They live under
  `entrypoints/app/<route>/`; this tree only holds pieces they compose.

## Workflows

**Adding a reusable widget**
1. Decide where it belongs: generic shadcn base → `ui/`; chat surface →
   `ai-elements/`; product-specific → `elements/`; feature-specific → the
   feature's own folder or inside `entrypoints/`.
2. Write the component props-first, taking all behaviour via props.
3. Put any state or side effect in `lib/<feature>/` and pass it in.

**Adding a new feature's UI**
1. Create `components/<feature>/`.
2. Import state from `lib/<feature>/` — never duplicate it in component state.
3. Add an `AGENTS.md` for the new folder (house rule: one per directory).

**Re-theming a component**
1. Use tokens (`bg-primary`, `text-muted-foreground`) from `styles/global.css`.
2. Check the `.dark` overrides before committing a hard-coded palette class.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root (rules X2, X6, X7, X8).
- [`../lib/AGENTS.md`](../lib/AGENTS.md) — the side-effect home.
- [`../components/ui/AGENTS.md`](../components/ui/AGENTS.md) — vendored shadcn.
- [`../components/elements/AGENTS.md`](elements/AGENTS.md) — first-party widgets.
- [`../entrypoints/AGENTS.md`](../entrypoints/AGENTS.md) — route components.
- [`../styles/AGENTS.md`](../styles/AGENTS.md) — design tokens.
