# `entrypoints/sidepanel/layout/` — panel shell and chat session provider

> Part of [`../AGENTS.md`](../AGENTS.md) in `entrypoints/sidepanel`.

## What's here

Two files that every screen in the panel depends on. `ChatSessionProvider` builds
the single `useChatSession(...)` instance and publishes it on a React context;
`ChatLayout` consumes that context to render the header and gate the outlet on a
ready provider.

`ChatSessionProvider` is also mounted by the new tab
(`../../newtab/layout/NewTabLayout.tsx`) with `origin="newtab"`, so anything you add
to the session affects both surfaces.

## Contents

```
layout/
├── ChatLayout.tsx         ← header + spinner gate + Outlet; wraps ChatSessionProvider
└── ChatSessionContext.tsx ← provider, useChatSessionContext(), and the context type
```

## Rules

- **LC1 — One session, one provider.** The context type is
  `ReturnType<typeof useChatSession>`, so adding a return field to the hook in
  `../index/useChatSession.ts` surfaces it everywhere with no wiring.
- **LC2 — Consume via `useChatSessionContext()`,** never by calling `useChatSession`
  in a component. The hook owns the transport, SSE stream, persistence and credits
  invalidation; a second instance duplicates all of it.
- **LC3 — The context hook throws when used outside the provider,** by design. That
  error is the signal that a component was mounted in the wrong tree — fix the tree,
  do not add a null check.
- **LC4 — `isIntegrationsSynced` is injected, not fetched here.**
  `useSyncRemoteIntegrations()` runs inside the provider and its `hasSynced` flag is
  passed into `useChatSession`; the session gates chat start on it. Removing that
  call lets a send race the integration sync.
- **LC5 — `ChatLayout` shows a spinner until `isLoading` is false and
  `selectedProvider` exists.** Do not move that guard into the child routes; the
  header depends on the same data.
- **LC6 — The provider accepts `ChatSessionOptions`,** currently `{ origin }` with
  `origin: 'sidepanel' | 'newtab'`. New options must have sensible defaults — the
  new-tab mount passes only `origin`.
- **LC7 — The panel has no page scroll.** `ChatLayout` is `h-screen w-screen
  overflow-hidden`; inner regions scroll. Adding content that needs page scroll
  breaks the panel.

## Workflows

**Adding state to the chat session**
1. Add it to the object returned by `useChatSession` in `../index/useChatSession.ts`.
2. Consume with `useChatSessionContext()` — no provider change required.
3. If it must differ per surface, add an option to `ChatSessionOptions` and branch on
   `origin`.

**Adding a header control**
1. Edit the `<ChatHeader …>` call in `ChatLayout.tsx`; the props it already receives
   are `selectedProvider`, `onSelectProvider`, `providers`, `onNewConversation`,
   `hasMessages`.
2. For anything needing session state, add it to the context rather than prop-drilling
   through the layout.

**Adding a panel route**
1. Add the `<Route>` in `../App.tsx` inside the `ChatLayout` block.
2. The route inherits the header, the spinner gate, and the session automatically.

**Debugging "context is null"**
1. The component is outside `ChatLayout` — check where it is mounted.
2. On the new tab, the provider mounts **conditionally**
   (`shouldUseChatSession` in `../../newtab/layout/route-utils.ts`); a new-tab route
   that consumes the context must be added to that predicate.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — panel overview and rules.
- [`../index/AGENTS.md`](../index/AGENTS.md) — `useChatSession`, the hook behind the context.
- [`../../newtab/layout/AGENTS.md`](../../newtab/layout/AGENTS.md) — the other provider mount.
- [`../../../../lib/mcp/useSyncRemoteIntegrations.ts`](../../../lib/mcp/useSyncRemoteIntegrations.ts) — the sync gate.
- [`../../../components/chat/chatComponentTypes.ts`](../../../components/chat/chatComponentTypes.ts) — `Provider` type used by the header.
