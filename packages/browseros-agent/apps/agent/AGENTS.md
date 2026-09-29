# `apps/agent/` — Browser extension internals

> Sub-package AGENTS file. The WXT + React + TypeScript browser extension.
> For the cross-repo architecture see
> [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).
> For the build pipeline see
> [`../../../../AGENTS-build.md`](../../../../AGENTS-build.md).

## What's here

A WXT-built browser extension that gives the user a sidebar chat panel,
new-tab experience, AI settings, MCP server config, scheduled tasks,
LLM hub, and more. Tech: WXT + React + TypeScript + shadcn/ui +
Tailwind + TanStack Query/GraphQL. Talks to the MCP server at
`http://127.0.0.1:9100` (configurable).

WXT (https://wxt.dev) auto-discovers entry points in `entrypoints/`, but only the
four conventions actually used here: `<name>/index.html` (HTML entry),
`<name>/index.ts` (background service worker), `<name>.ts` / `<name>.content.ts`
(content script). That means `app/`, `sidepanel/`, `background/`, `content.ts`,
`selection.content.ts`, `glow.content/`, and `auth.content/` are entries —
`entrypoints/app/`, `entrypoints/newtab/` and `entrypoints/onboarding/`
subfolders are **not**: they are React route trees mounted by hand in
`entrypoints/app/App.tsx`. `entrypoints/sidepanel/` is the side panel.

## Source map

```
apps/agent/
├── package.json, tsconfig.json, wxt.config.ts, web-ext.config.ts
├── codegen.ts                       ← GraphQL codegen
├── components.json                  ← shadcn config
├── biome.json
├── assets/mcp-icons/                ← MCP server icons for the UI
├── schema/schema.graphql            ← GraphQL schema
├── components/                      ← React UI components
│ ├── ai-elements/                   ← (shadcn ai-elements primitives)
│ ├── auth/
│ ├── chat/
│ ├── credits/
│ ├── elements/
│ ├── execution-history/
│ ├── sidebar/
│ ├── theme-provider.tsx
│ └── ui/                            ← shadcn primitives (Button, Card, etc.)
├── hooks/use-mobile.ts
├── entrypoints/                     ← WXT entry points + the route trees they render
│ ├── app/                           ← HTML entry #1 → app.html; HashRouter shell,
│ │ │                                  chrome_url_overrides.newtab + options_ui.
│ │ │                                  Its subdirs are React routes registered by
│ │ │                                  hand in app/App.tsx — NOT auto-discovered:
│ │ ├── agent-command/                ← /home (alpha) + /home/agents/:agentId
│ │ ├── agents/                      ← agent-harness data layer (not a route)
│ │ ├── ai-settings/                 ← /settings/ai — LLM provider + key config
│ │ ├── connect-mcp/
│ │ ├── customization/
│ │ ├── jtbd-agent/
│ │ ├── layout/
│ │ ├── llm-hub/
│ │ ├── login/
│ │ ├── mcp-settings/                ← MCP server connection mgmt
│ │ ├── profile/
│ │ ├── scheduled-tasks/             ← cron-like task scheduler
│ │ └── usage/
│ ├── sidepanel/                     ← HTML entry #2 → sidepanel.html; chat side panel
│ ├── background/                    ← background/index.ts, the MV3 service worker
│ ├── content.ts                     ← 3-line stub content script (google.com, empty main())
│ ├── selection.content.ts           ← content script for text-selection capture
│ ├── glow.content/                  ← content script for the "Glow" in-page feature
│ ├── auth.content/                  ← content script for auth state detection
│ ├── newtab/                        ← NOT an entry — route components under /home
│ │ ├── index/
│ │ ├── layout/
│ │ └── personalize/
│ └── onboarding/                    ← NOT an entry — route components under /onboarding
│   ├── demo/
│   ├── features/
│   ├── index/
│   └── steps/
├── lib/                             ← 30+ feature modules
│ ├── agent-conversations/
│ ├── analytics/
│ ├── auth/
│ ├── browseros/                     ← BrowserOS-specific helpers
│ ├── changelog/
│ ├── chat/, chat-actions/
│ ├── constants/
│ ├── conversations/
│ ├── credits/
│ ├── declined-apps/
│ ├── env.ts
│ ├── execution-history/
│ ├── getCurrentYear.ts
│ ├── getFavicons.ts
│ ├── graphql/                       ← execute() + TanStack Query provider/hooks
│ ├── jtbd-popup/
│ ├── llm-hub/
│ ├── llm-providers/                 ← flat: ProviderType union, config storage,
│ │                                    models.dev cache, OAuth flows, GraphQL upload
│ ├── mcp/                           ← raw MCP JSON-RPC client + server storage
│ ├── messaging/
│ ├── metrics/
│ ├── onboarding/
│ ├── personalization/
│ ├── rpc/                           ← Hono hc<AppType> client + RpcClientProvider
│ │                                    (2 files: getClient.ts, RpcClientProvider.tsx)
│ ├── schedules/
│ ├── search-actions/
│ ├── selected-text/
│ ├── sentry/
│ ├── sse.ts                         ← SSE client
│ ├── stop-agent/
│ ├── theme/
│ ├── tool-labels.ts
│ ├── useIsMac.ts
│ ├── utils.ts
│ ├── voice/
│ └── workspace/
├── styles/, public/
├── CHANGELOG.md
├── CLAUDE.md
└── README.md
```

## Opinionated rules

### X1 — WXT entry discovery is convention-based, and only four conventions are in use
WXT auto-discovers `<name>/index.html` (HTML entry), `<name>/index.ts`
(background service worker), and `<name>.ts` / `<name>.content.ts` (content
script). In this repo that means exactly: `app/`, `sidepanel/`, `background/`,
`content.ts`, `selection.content.ts`, `glow.content/`, `auth.content/`.

**Everything else under `entrypoints/` is a React route tree, not an entry.**
`entrypoints/app/<route>/index.tsx`, `entrypoints/newtab/…` and
`entrypoints/onboarding/…` are NOT auto-discovered — a component only exists once
it is imported and mounted under a `<Route>` in `entrypoints/app/App.tsx`. An
unregistered route folder is dead code. If you need to override WXT's default
config, edit `wxt.config.ts`.

### X2 — `lib/` is the side-effect home
Components are pure presentational; `lib/<feature>/` holds state,
side effects, API calls, storage. Tests live next to the lib module.
There is no per-feature `index.ts` barrel convention in this repo — import the
named file you need (e.g. `@/lib/llm-providers/storage`).

### X3 — Storage: `chrome.storage.local` for user data
Per-user state (preferences, conversation history, scheduled tasks)
goes in `chrome.storage.local`, always through a per-feature wrapper module —
never call `chrome.storage` directly in components. There is no shared
`lib/storage.ts` and no generic `Storage` class. Only six features name the file
literally `storage.ts` (`agent-conversations/`, `declined-apps/`,
`execution-history/`, `jtbd-popup/`, `llm-hub/`, `llm-providers/`); the rest use a
camelCase or kebab sibling (`schedules/scheduleStorage.ts`,
`conversations/conversationStorage.ts`, `theme/theme-storage.ts`,
`mcp/mcpServerStorage.ts`, …). Match the neighbour's name in the folder you touch.
Two documented exceptions: `lib/agent-conversations/storage.ts` uses IndexedDB
via `idb-keyval`, and `lib/declined-apps/storage.ts` imports `storage` from
`#imports` rather than `@wxt-dev/storage`.

### X4 — Server calls: `lib/rpc/` for the Hono client, `lib/mcp/` for raw JSON-RPC
`lib/rpc/` holds exactly two modules — `getClient.ts` (Hono `hc<AppType>`, typed
from `@browseros/server`) and `RpcClientProvider.tsx`. There is no
`lib/rpc/<service>.ts`, no `lib/rpc/index.ts`, and no `sendStreamingMessage` — raw
MCP JSON-RPC (`tools/list`, `initialize`) lives in `lib/mcp/client.ts`, and GraphQL
goes through `lib/graphql/execute.ts`. Components import the typed RPC client, not
raw `fetch`. SSE via `lib/sse.ts`.

### X5 — GraphQL is hand-maintained
`schema/schema.graphql` is the source. Run `bun run codegen` after
schema edits. Don't auto-generate from TS.

### X6 — shadcn/ui for new components
Use shadcn primitives from `components/ui/`. New primitives: run
`bunx shadcn@latest add <name>`. Customize in place; don't fork
shadcn wholesale.

### X7 — Tailwind for styling, no CSS-in-JS
Tailwind utilities only. No styled-components, no emotion. The
`theme-provider.tsx` handles dark/light/system.

### X8 — No direct API calls in components
Components dispatch through hooks in `lib/`. Hooks call `getClient()` /
`useRpcClient()`, `lib/graphql/execute.ts`, or the feature's own client.
Tests for hooks in `lib/<feature>/<hook>.test.ts`.

### X9 — Sidepanel is the primary surface
The most-used UI is `entrypoints/sidepanel/`. New chat features go
there first. The other surfaces (`entrypoints/app/`, `entrypoints/newtab/`,
`entrypoints/onboarding/`) are secondary.

### X10 — `lib/llm-providers/` is the single source of truth for provider config
`lib/llm-providers/` is **flat** — there are no per-provider subfolders
(`lib/llm-providers/anthropic/`, `claude.ts`, etc. do not exist) and no
`index.ts` barrel. `types.ts` declares `ProviderType` as a **string union**
(14 members), not an enum. Adding a provider means: extend the `ProviderType`
union → add a `providerTemplates.ts` entry → add an icon in `providerIcons.tsx`
→ add UI under `entrypoints/app/ai-settings/` (there is no
`components/ai-settings/`). Don't duplicate the provider list anywhere else.

## Opinionated workflow

### "I'm adding a new extension route (in the extension-app)"
1. Create the component under `entrypoints/app/<route>/`.
2. **Register it by hand** — import it in `entrypoints/app/App.tsx` and add a
   `<Route>` inside the right layout (`AuthLayout` / `SidebarLayout` /
   `SettingsSidebarLayout`). WXT does *not* auto-discover this folder, so an
   unregistered route is dead code. No manifest update is needed either way.
3. Add a sidebar entry in `components/sidebar/` (`AppSidebar` or
   `SettingsSidebar`) and, if it is a settings page, the relevant
   `entrypoints/app/layout/`.
4. Move side-effect logic into `lib/<feature>/`.
5. Self-test via the CDP inspector (see
   [`../../CLAUDE.md`](../../CLAUDE.md) § "Self-Testing UI Changes"):
   ```bash
   bun scripts/dev/inspect-ui.ts open-sidepanel
   bun scripts/dev/inspect-ui.ts screenshot sidepanel /tmp/panel.png
   bun scripts/dev/inspect-ui.ts snapshot sidepanel
   ```

### "I'm adding a new scheduled task"
1. Add the task type/logic at `lib/schedules/<name>.ts` (there is no
   `lib/schedules/index.ts` barrel and no `lib/schedules/storage.ts` — storage is
   `lib/schedules/scheduleStorage.ts`).
2. Persist via `lib/schedules/scheduleStorage.ts`.
3. Wire the alarm in `entrypoints/background/scheduledJobRuns.ts`.
4. Add UI in `entrypoints/app/scheduled-tasks/`.

### "I'm adding a new LLM provider"
`lib/llm-providers/` is flat — do **not** create a `<name>/` subfolder.
1. Add the member to the `ProviderType` string union in `lib/llm-providers/types.ts`.
2. Add a `providerTemplates.ts` entry (`defaultBaseUrl`, `defaultModelId`,
   `supportsImages`, `contextWindow`).
3. Add the icon in `lib/llm-providers/providerIcons.tsx` (`null` falls back to
   `BrowserOSIcon`).
4. Add the UI to `entrypoints/app/ai-settings/` — there is no
   `components/ai-settings/` folder.
5. Nothing to "register": `useLlmProviders()` iterates storage.

### "I'm adding a new MCP server connection"
1. Add to `lib/mcp/` (connection state, schema fetch, OAuth flow).
2. Add UI to `entrypoints/app/mcp-settings/`.
3. Icons in `assets/mcp-icons/`.

### "I'm changing the chat UI"
1. Edit components in `components/chat/`.
2. State changes in `lib/chat/`.
3. Streaming via `lib/sse.ts`.
4. Test via CDP inspector.

### "I'm adding a feature flag (runtime)"
1. Add to `lib/<feature>/flags.ts` (or co-locate with the feature).
2. Add UI in the relevant settings page.
3. Default to a safe value.

### "I'm adding a GraphQL operation"
1. Add the operation to `schema/schema.graphql` (the hand-maintained SDL).
2. Run `bun run codegen:agent` from `packages/browseros-agent`
   (`bun run codegen` from `apps/agent` does the same thing) to regenerate
   the typed client.
3. Use the generated hook from `lib/graphql/`.

## Build / dev

Real script names — `packages/browseros-agent/package.json` and
`apps/agent/package.json` are the source of truth. There is no `dev:ext`,
`dist:ext`, or root `codegen` script.

```bash
# From packages/browseros-agent
bun install
bun run dev:watch              # full dev loop (tools/dev/run.sh watch)
bun run build:agent:dev        # development extension build
bun run build:agent            # codegen + production extension build
bun run codegen:agent          # regenerate GraphQL client from schema
bun run lint / lint:fix        # biome
bun run typecheck              # wxt prepare + tsgo --noEmit

# From apps/agent
bun run dev                    # wxt dev server (writes to outDir)
bun run build / build:dev / zip
bun run test                   # bun test across apps/agent
```

Load the dev extension in Chrome:
1. Open `chrome://extensions`.
2. Enable Developer mode.
3. Click "Load unpacked" and select
 `packages/browseros-agent/apps/agent/dist/chrome-mv3/`.

`wxt.config.ts` sets `outDir: 'dist'` — **not** `.output/`. Anything pointing at
`.output/chrome-mv3` is stale.

## Self-testing UI changes

Per `packages/browseros-agent/CLAUDE.md` § "Self-Testing UI Changes",
use the CDP inspector for UI work:

```bash
bun scripts/dev/inspect-ui.ts targets
bun scripts/dev/inspect-ui.ts open-sidepanel
bun scripts/dev/inspect-ui.ts screenshot sidepanel /tmp/panel.png
bun scripts/dev/inspect-ui.ts snapshot sidepanel
bun scripts/dev/inspect-ui.ts click sidepanel 142
bun scripts/dev/inspect-ui.ts fill sidepanel 85 "search query"
bun scripts/dev/inspect-ui.ts eval sidepanel "document.title"
```

The `screenshot sidepanel` output is a PNG you can `read` to see what
the user sees.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md) — overall map.
- [`../../../../AGENTS-build.md`](../../../../AGENTS-build.md) — build pipeline.
- [`../../CLAUDE.md`](../../CLAUDE.md) — coding guidelines.
- [`../server/AGENTS.md`](../server/AGENTS.md) — MCP server side.
