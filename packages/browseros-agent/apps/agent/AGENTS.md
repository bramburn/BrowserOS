# `apps/agent/` — Browser extension internals

> Sub-package AGENTS file. The WXT + React + TypeScript browser extension.
> For the cross-repo architecture see
> [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> For the parent monorepo see [`../../AGENTS.md`](../../AGENTS.md).
> For the build pipeline see
> [`../../../AGENTS-build.md`](../../../AGENTS-build.md).

## What's here

A WXT-built browser extension that gives the user a sidebar chat panel,
new-tab experience, AI settings, MCP server config, scheduled tasks,
LLM hub, and more. Tech: WXT + React + TypeScript + shadcn/ui +
Tailwind + TanStack Query/GraphQL. Talks to the MCP server at
`http://127.0.0.1:9100` (configurable).

WXT (https://wxt.dev) auto-discovers entry points in `entrypoints/`.
Subdirs of `entrypoints/app/` are React routes; `entrypoints/sidepanel/`
is the side panel; `entrypoints/newtab/` is the new-tab page;
`entrypoints/onboarding/` is the first-run flow.

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
├── entrypoints/                     ← WXT entry points
│ ├── background/                    ← service worker (MV3)
│ ├── content.ts                     ← content scripts
│ ├── app/                           ← extension-app routes
│ │ ├── agent-command/
│ │ ├── agents/                      ← list/manage agents
│ │ ├── ai-settings/                 ← LLM provider + key config
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
│ ├── newtab/                        ← new-tab page
│ │ ├── index/
│ │ ├── layout/
│ │ └── personalize/
│ ├── onboarding/                    ← first-run flow
│ │ ├── demo/
│ │ ├── features/
│ │ ├── index/
│ │ └── steps/
│ ├── sidepanel/                     ← chat side panel
│ ├── glow.content                   ← content script for "Glow" feature
│ ├── auth.content
│ ├── selection.content.ts
│ └── ...
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
│ ├── graphql/                       ← Apollo/urql client setup
│ ├── jtbd-popup/
│ ├── llm-hub/
│ ├── llm-providers/                 ← provider config + UI bindings
│ ├── mcp/                           ← MCP server connection state
│ ├── messaging/
│ ├── metrics/
│ ├── onboarding/
│ ├── personalization/
│ ├── rpc/                           ← fetch wrappers
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

### X1 — WXT entry discovery
Files matching `entrypoints/<name>/index.tsx` (or `<name>.ts`) are
auto-discovered. Don't fight the convention. If you need to override
WXT's default config, edit `wxt.config.ts`.

### X2 — `lib/` is the side-effect home
Components are pure presentational; `lib/<feature>/` holds state,
side effects, API calls, storage. A `lib/<feature>/index.ts` is the
public surface. Tests live next to the lib module.

### X3 — Storage: `chrome.storage.local` for user data
Per-user state (preferences, conversation history, scheduled tasks)
goes in `chrome.storage.local`. Use a wrapper in
`lib/<feature>/storage.ts` — never call `chrome.storage` directly
in components.

### X4 — RPC: `lib/rpc/` for MCP server calls
All MCP server communication goes through `lib/rpc/<service>.ts`.
Components import the typed RPC client, not raw `fetch`. SSE via
`lib/sse.ts`.

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
Components dispatch through hooks in `lib/`. Hooks call `lib/rpc/`.
Tests for hooks in `lib/<feature>/<hook>.test.ts`.

### X9 — Sidepanel is the primary surface
The most-used UI is `entrypoints/sidepanel/`. New chat features go
there first. Other entrypoints (newtab, app/*) are secondary.

### X10 — `lib/llm-providers/` is the single source of truth for provider config
Adding a new LLM provider? Edit `lib/llm-providers/<name>/`,
register in the index, add UI to `components/ai-settings/`. Don't
duplicate the provider list anywhere else.

## Opinionated workflow

### "I'm adding a new extension route (in the extension-app)"
1. Create `entrypoints/app/<route>/index.tsx` + component files.
2. WXT auto-discovers. No manifest update needed.
3. Add a sidebar entry in `components/sidebar/` and the relevant
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
1. Add the task definition at `lib/schedules/<name>.ts`.
2. Register in `lib/schedules/index.ts`.
3. Add UI in `entrypoints/app/scheduled-tasks/`.
4. Persist via `lib/schedules/storage.ts`.

### "I'm adding a new LLM provider"
1. Create `lib/llm-providers/<name>/` with:
 - `config.ts` (zod schema for provider config)
 - `client.ts` (RPC client to the server's provider endpoint)
 - `index.ts` (public surface)
2. Register in `lib/llm-providers/index.ts`.
3. Add to `components/ai-settings/` provider list.
4. Update `packages/browseros-agent/config.sample.json`.

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
1. Add the operation to `schema/schema.graphql` (or the SDL file
 that generates it).
2. Run `bun run codegen` to regenerate the typed client.
3. Use the generated hook from `lib/graphql/`.

## Build / dev

```bash
# From packages/browseros-agent
bun install
bun run dev:ext            # build extension for development
bun run dev:watch          # full dev mode (MCP + extension HMR)
bun run dist:ext           # production extension build
bun run codegen            # regenerate GraphQL client from schema
```

Load the dev extension in Chrome:
1. Open `chrome://extensions`.
2. Enable Developer mode.
3. Click "Load unpacked" and select
 `packages/browseros-agent/apps/agent/.output/chrome-mv3/`.

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
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — overall map.
- [`../../../AGENTS-build.md`](../../../AGENTS-build.md) — build pipeline.
- [`../../CLAUDE.md`](../../CLAUDE.md) — coding guidelines.
- [`../server/AGENTS.md`](../server/AGENTS.md) — MCP server side.
