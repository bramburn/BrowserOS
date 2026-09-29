# `entrypoints/sidepanel/` — the side panel (WXT HTML entry → `sidepanel.html`)

> Part of the WXT entry points in `apps/agent/entrypoints`. Parent guide:
> [`../../AGENTS.md`](../../AGENTS.md).

## What's here

**The primary user surface of the extension.** `index.html` is a WXT HTML entry that
mounts `main.tsx`, which renders `App.tsx` — a small `HashRouter` with exactly two
routes: the chat (`/`) and the conversation history (`/history`). The panel is opened
programmatically from the toolbar action (the service worker toggles it) and from
search actions, and it shares its chat session with the new tab via
`ChatSessionContext`.

The heavy lifting lives in `index/` (the chat surface and its session hook) and
`layout/` (provider + shell). `history/` is the conversation list, backed by either
GraphQL (signed in) or local storage (signed out).

## Contents

```
sidepanel/
├── index.html    ← WXT HTML entry; loads ./main.tsx
├── main.tsx      ← React root with the same provider stack as app/main.tsx
├── App.tsx       ← HashRouter: `/` → Chat, `/history` → ChatHistory
├── index/        ← the chat surface + useChatSession (see its own AGENTS.md)
├── layout/       ← ChatLayout + ChatSessionContext (see its own AGENTS.md)
└── history/      ← conversation list: remote + local (see its own AGENTS.md)
```

## Rules

- **SP1 — Two routes only.** Adding a third screen means editing `App.tsx` *and*
  deciding where the chrome lives; `ChatLayout` supplies the header and provider to
  both existing routes.
- **SP2 — `main.tsx` is a near-duplicate of `../../app/main.tsx`,** and the provider
  order must stay identical: `AuthProvider → QueryProvider → AnalyticsProvider →
  ThemeProvider → <App/> + <Toaster>`. Change one, change both.
- **SP3 — The chat session is owned by `layout/ChatSessionContext`.** Components
  consume `useChatSessionContext()`; they must not call `useChatSession` directly.
  That hook is expensive (transport, SSE, persistence, credits invalidation).
- **SP4 — The panel never calls the local server directly.** MCP URL resolution,
  health, and tool fetches go through the worker via `sendServerMessage`.
- **SP5 — `index.html` still has the placeholder title "Default Side Panel Title".**
  It is user-visible in the panel header on some Chrome versions; changing it is a
  small, worthwhile fix, but do it deliberately.
- **SP6 — Feature work lands here first.** New chat behaviour goes into `index/`
  before it goes into `../newtab/` or `../app/`.
- **SP7 — `ChatLayout` gates rendering on a resolved provider.** Until
  `useChatSession` reports `isLoading === false` and a `selectedProvider`, it shows a
  spinner instead of the outlet — don't move that guard into children.

## Workflows

**Adding a panel screen**
1. Create the component under `index/` (or a new subfolder).
2. Add `<Route path="…" element={…} />` in `App.tsx` inside the `ChatLayout` block.
3. Read state from `useChatSessionContext()`; add new state to `useChatSession` in
   `index/` so both routes see it.
4. Keep the `h-screen … overflow-hidden` shell assumptions — the panel has no page
   scroll; inner regions scroll.

**Verifying a change**
1. `bun run dev:watch` from `packages/browseros-agent`, export `BROWSEROS_CDP_PORT`.
2. `bun scripts/dev/inspect-ui.ts open-sidepanel`
3. `snapshot sidepanel` → `click`/`fill` by id → `screenshot sidepanel /tmp/panel.png`.
4. Test the history route too: `app.html` is a different page, but the panel's
   `#/history` hash is reachable in the same target.

**Extending the chat session**
1. Add the field to the object returned by `useChatSession` in `index/useChatSession.ts`.
2. It becomes available on `useChatSessionContext()` automatically — the context type
   is `ReturnType<typeof useChatSession>`.
3. Add a unit test in `index/useChatSession.test.ts` if the derivation is non-trivial.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals, rules X1–X10.
- [`./index/AGENTS.md`](./index/AGENTS.md) — the chat surface.
- [`./layout/AGENTS.md`](./layout/AGENTS.md) — `ChatSessionContext` and the shell.
- [`./history/AGENTS.md`](./history/AGENTS.md) — conversation history.
- [`../../wxt.config.ts`](../../wxt.config.ts) — `sidePanel` permission and action icon.
- [`../background/AGENTS.md`](../background/AGENTS.md) — opens the panel on toolbar click.
- [`../../../../CLAUDE.md`](../../../../CLAUDE.md) — CDP inspector workflow.
