# `entrypoints/app/` — extension app shell (WXT HTML entry → `app.html`)

> Part of [`../../AGENTS.md`](../../AGENTS.md) in `apps/agent`. Nearest ancestor
> with its own guide is `apps/agent/AGENTS.md` — there is no `entrypoints/AGENTS.md`
> today.

## What's here

This is the WXT **HTML entry point** for the whole extension app. `index.html`
mounts `#root` from `main.tsx`, which renders `App.tsx` — a `HashRouter` route
table that is the real navigation authority for the extension. It is also the
page Chrome uses for `chrome_url_overrides.newtab` and for `options_ui`
(`app.html#/settings`), both configured in `wxt.config.ts`.

The subdirectories (`agent-command/`, `agents/`, `ai-settings/`, `connect-mcp/`,
`customization/`, `jtbd-agent/`, `layout/`, `llm-hub/`, `login/`, `mcp-settings/`,
`profile/`, `scheduled-tasks/`, `usage/`) are **React route components**, not WXT
entry points. Routes are declared by hand in `App.tsx`; nothing auto-discovers
them. Note the correction: the common "create `entrypoints/app/<route>/index.tsx`
and WXT picks it up" advice in `apps/agent/AGENTS.md` does **not** hold here —
`app/` has exactly one WXT entry (`index.html`), and its subdirs must be imported
into `App.tsx` and mounted under a `<Route>` to exist at all.

## Contents

```
app/
├── index.html          ← WXT HTML entry; applies theme pre-paint, loads ./main.tsx
├── main.tsx            ← React root: AuthProvider → QueryProvider → AnalyticsProvider → ThemeProvider → <App/> + <Toaster/>
├── App.tsx             ← HashRouter route table (the routing source of truth)
├── layout/             ← Route layouts: AuthLayout, SidebarLayout, SettingsSidebarLayout
├── agent-command/      ← `/home` composer + `/home/agents/:agentId` harness chat (alpha)
├── agents/             ← Agent-harness data layer: adapters, agents, CRUD hooks
├── ai-settings/        ← `/settings/ai` — LLM providers + per-adapter agent panes
├── connect-mcp/        ← `/connect-apps` — managed/custom MCP server connections
├── customization/      ← `/settings/customization` — browser toolbar prefs
├── jtbd-agent/         ← `/settings/survey` — JTBD survey chat
├── llm-hub/            ← `/settings/chat` — LLM Hub provider list
├── login/              ← `/login`, `/logout`, `/auth/magic-link`
├── mcp-settings/       ← `/settings/mcp` — MCP server URL, health, tool list
├── profile/            ← `/profile` — name/avatar, GraphQL-backed
├── scheduled-tasks/    ← `/scheduled` — cron-like scheduled job manager
└── usage/              ← `/settings/usage` — credit usage & billing
```

## Rules

- **APP1 — Routes live in `App.tsx`, not in the filesystem.** Adding a page means:
  create the folder + component, add the import, add the `<Route>`. WXT only sees
  `index.html`. A route folder that is not in `App.tsx` is dead code.
- **APP2 — `main.tsx` provider order is load-bearing.** `AuthProvider` must wrap
  `QueryProvider`; `QueryProvider` must wrap `AnalyticsProvider`. Changing the
  order breaks session-scoped GraphQL and analytics identity. Keep both files
  (`app/main.tsx` and `sidepanel/main.tsx`) in sync — they are near-duplicates.
- **APP3 — Capability-gate new surfaces behind `useCapabilities()`.** `App.tsx`
  already branches the `/home` subtree on `Feature.ALPHA_FEATURES_SUPPORT`; new
  experimental routes follow the same pattern rather than being shown and hidden
  inside the component.
- **APP4 — Every new route needs a settings/sidebar entry.** `SidebarLayout` reads
  `@/components/sidebar/AppSidebar`; `SettingsSidebarLayout` reads
  `@/components/sidebar/SettingsSidebar`. A route with no nav entry is unreachable
  except by typing the hash.
- **APP5 — Keep back-compat redirects in `App.tsx`, not scattered.** `OptionsRedirect`,
  `LegacyAgentRedirect` and the `/audit`, `/observability`, `/executions`,
  `/settings/connect-mcp`, `/agents` redirects all live in one block. Add new legacy
  paths there so the table stays auditable.
- **APP6 — Cross-route imports are legal and used.** `app/ai-settings` imports from
  `app/agents`, `app/agents/agent-row` imports from `app/agents`, `newtab/index`
  imports from `app/connect-mcp`. Prefer importing a shared piece into the nearest
  folder over duplicating it in a second route.

## Workflows

**Adding a new settings page**
1. Create `app/<route>/<Page>.tsx` exporting an `FC` (name the page component after
   the route, e.g. `UsagePage.tsx`).
2. Add a nav item in `@/components/sidebar/SettingsSidebar`.
3. In `App.tsx`, add `<Route path="<subpath>" element={<YourPage />} />` inside the
   `SettingsSidebarLayout` block.
4. If an old path must keep working, add it to the backward-compatibility block at
   the bottom of `App.tsx`.
5. Verify with `bun scripts/dev/inspect-ui.ts snapshot app` from
   `packages/browseros-agent`.

**Adding a new top-level (non-settings) page**
1. Create the component under `app/<route>/`.
2. Mount it in `App.tsx` inside the `SidebarLayout` block (`/connect-apps` and
   `/scheduled` are the two current examples) or as a top-level route
   (`/onboarding/*`).
3. Add the sidebar link in `AppSidebar`.
4. Wrap in `RpcClientProvider` if it calls the local agent server — the layouts
   already do this, so nest inside the layout rather than re-wrapping.

**Adding a GraphQL query to a route**
1. Put the `graphql(...)` document in that route's `graphql/` subfolder (e.g.
   `app/ai-settings/graphql/aiSettingsDocument.ts`).
2. Add or extend the operation in `apps/agent/schema/schema.graphql`.
3. Run `bun run codegen:agent` from `packages/browseros-agent`
   (`bun run codegen` from `apps/agent` is the same script).
4. Call it through `@/lib/graphql/useGraphqlQuery` / `useGraphqlMutation` and
   invalidate with `getQueryKeyFromDocument(...)`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — extension internals, rules X1–X10, dev/build commands.
- [`../../../../CLAUDE.md`](../../../../CLAUDE.md) — monorepo coding guidelines (authoritative).
- [`../../wxt.config.ts`](../../wxt.config.ts) — manifest: `chrome_url_overrides`, `options_ui`, permissions.
- [`../../schema/schema.graphql`](../../schema/schema.graphql) — hand-maintained GraphQL SDL.
- [`../sidepanel/AGENTS.md`](../sidepanel/AGENTS.md) — the side panel entry point.
