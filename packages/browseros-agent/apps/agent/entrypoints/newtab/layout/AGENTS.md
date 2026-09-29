# `entrypoints/newtab/layout/` — new-tab route layout

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/newtab`.

## What's here

`NewTabLayout` is the `<Route element={...}>` wrapper for the whole `/home` branch in
`app/App.tsx`. It does three things: optionally renders the decorative focus-grid
background, decides whether to mount a `ChatSessionProvider`, and renders the
`<Outlet />`.

The decisions are pure functions in `route-utils.ts` — `isAgentCommandPath`,
`isAgentConversationPath`, `shouldHideFocusGrid`, `shouldUseChatSession` — and they
are unit-tested. That split is deliberate: the path logic is the part that breaks
silently when routes move.

## Contents

```
layout/
├── NewTabLayout.tsx     ← the layout component; provider + focus grid + Outlet
├── NewTabFocusGrid.tsx  ← decorative grid/radial background (pointer-events-none)
└── route-utils.ts       ← pure path predicates  (+ route-utils.test.ts)
```

## Rules

- **NL1 — Path knowledge lives in `route-utils.ts`, not in the component.** Both
  predicates are pure string checks and both are covered by `route-utils.test.ts`.
  Add a case there, not an `if` in JSX.
- **NL2 — `ChatSessionProvider` mounts only where `shouldUseChatSession` is true**
  (`/home/chat`, plus `/home` when the non-alpha `useChatSessionOnHome` prop is set).
  Mounting it unconditionally would construct a chat session on every new-tab render.
- **NL3 — `NewTabFocusGrid` is `pointer-events-none` and purely decorative.** Never
  put interactive content in it; it sits behind the whole page.
- **NL4 — The `origin` prop distinguishes the two surfaces.**
  `<ChatSessionProvider origin="newtab">` here, no origin (defaults to `sidepanel`) in
  `../../sidepanel/layout/ChatLayout.tsx`. Analytics and persistence branch on it.
- **NL5 — Route paths in this file must match `../../app/App.tsx` exactly.**
  `'/home'`, `'/home/chat'`, `'/home/agents/'` are duplicated in both files; a rename
  in one without the other silently changes the background and the provider.

## Workflows

**Hiding the grid on a new route**
1. Add the path to `HIDE_FOCUS_GRID_PATHS` in `route-utils.ts`, or extend a predicate.
2. Add the case to `route-utils.test.ts`.
3. Nothing in `NewTabLayout.tsx` changes.

**Mounting the chat session on a route**
1. Extend `shouldUseChatSession(pathname, useChatSessionOnHome)` in `route-utils.ts`.
2. Add a test case.
3. Check the surface still gets the right `origin` — only the side panel uses
   `sidepanel`.

**Changing the background**
1. `NewTabFocusGrid.tsx` renders `bg-grid-pattern` + `bg-gradient-radial-focus`
   (both defined in `apps/agent/styles/`).
2. Keep the `pointer-events-none` wrapper.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the new-tab route rules.
- [`../../app/AGENTS.md`](../../app/AGENTS.md) — where `NewTabLayout` is mounted.
- [`../../../sidepanel/layout/AGENTS.md`](../../sidepanel/layout/AGENTS.md) — the other `ChatSessionProvider` mount.
- [`./route-utils.test.ts`](./route-utils.test.ts) — the canonical expectations for these predicates.
