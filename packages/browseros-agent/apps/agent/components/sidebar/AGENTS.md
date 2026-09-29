# `components/sidebar/` — App navigation chrome

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent/components`.

## What's here

The left-hand navigation for the extension's full-page app surface
(`entrypoints/app/`, served from `app.html`). Two shells —
`AppSidebar` for the top-level app and `SettingsSidebar` for the nested
settings area — composed from branding, navigation and footer pieces.
Every route here is gated by `useCapabilities()` so a nav item only
appears when the paired browser/server version supports it.

## Contents

```
sidebar/
├── AppSidebar.tsx           ← shell: 3-slice flex column, w-14 collapsed /
│                              w-64 expanded
├── SidebarBranding.tsx      ← logo + workspace folder name + sign-in/out
│                              dropdown; reads useSessionInfo(), useWorkspace()
│                              and a GraphQL profile query
├── SidebarNavigation.tsx    ← primaryNavItems: Home, Connect Apps
│                              (Feature.MANAGED_MCP_SUPPORT), Scheduled Tasks,
│                              Settings; each item may carry a `feature` gate
├── SidebarUserFooter.tsx    ← About link, credits/shortcuts affordances;
│                              currently holds a commented-out sign-in block
└── SettingsSidebar.tsx      ← settings-specific NavSection list, mixes
                               internal (NavLink `to`) and external
                               (`href`) items, mounts ThemeToggle
```

## Rules

- **SB1 — New nav entries need a `feature` gate.** Every item in
  `primaryNavItems` and `SettingsSidebar`'s sections may carry
  `feature?: Feature`; `useCapabilities()` filters on it. A nav item
  without a gate ships to browsers that cannot run it.
- **SB2 — `isNavItemActive` special-cases `/settings/ai`.** Any new settings
  sub-route must be covered by that prefix check or it will never highlight.
- **SB3 — Both sides are router-dependent.** Use `NavLink`/`useLocation` from
  `react-router`; these components only render inside a router.
- **SB4 — Collapsed width is a fixed `w-14`, expanded is `w-64`.** Keep the
  `transition-all` on the container; children take an `expanded` boolean and
  are responsible for hiding their own labels.
- **SB5 — `SettingsSidebar` splits internal vs external items by the
  `href`/`to` discriminant.** Follow the `InternalNavItem | ExternalNavItem`
  union rather than branching on truthiness.

## Workflows

**Adding a top-level nav entry**
1. Add a `{ name, to, icon, feature? }` object to `primaryNavItems` in
   `SidebarNavigation.tsx`.
2. Add the `Feature` enum member in `lib/browseros/capabilities.ts` and its
   version bound in `FEATURE_CONFIG`.
3. Create the route component at `entrypoints/app/<route>/`, then import it
   and add a `<Route>` in `entrypoints/app/App.tsx` — WXT does **not**
   auto-discover it; an unregistered route folder is dead code.
4. Screenshot the app route via
   `bun scripts/dev/inspect-ui.ts screenshot app /tmp/sidebar.png`.

**Adding a settings sub-page**
1. Add the route under `entrypoints/app/`.
2. Add a `NavSection` entry in `SettingsSidebar.tsx`.
3. Verify the active highlight via the `/settings` prefix branch.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — component-tree rules.
- [`ui/tooltip.tsx`](../ui/tooltip.tsx) — `TooltipProvider` used for collapsed labels.
- [`../elements/theme-toggle.tsx`](../elements/theme-toggle.tsx) — mounted in `SettingsSidebar`.
- [`../../lib/browseros/AGENTS.md`](../../lib/browseros/AGENTS.md) — `Feature` enum and `useCapabilities`.
- [`../../lib/auth/AGENTS.md`](../../lib/auth/AGENTS.md) — `useSessionInfo`.
- [`../../entrypoints/AGENTS.md`](../../entrypoints/AGENTS.md) — the routes these link to.
