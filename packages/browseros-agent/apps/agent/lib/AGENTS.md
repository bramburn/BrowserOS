# `lib/` — Side effects, state, storage, clients

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/agent`.

## What's here

The non-React half of the extension. Every piece of stateful or effectful
logic lives here: storage wrappers, hooks, typed clients, SSE parsing,
feature gates, and GraphQL plumbing. `../components/` renders; this
directory acts. Each feature gets its own folder, and the convention is
that per-feature state goes through a `storage.ts` built on
`@wxt-dev/storage`'s `local:` area (i.e. `chrome.storage.local`).

## Contents

```
lib/
├── env.ts                     ← zod-validated `import.meta.env`; import `env`,
│                                never read import.meta.env directly
├── utils.ts                   ← cn() (clsx + tailwind-merge)
├── sse.ts                     ← SSE frame parser: parseSSELines(),
│                                consumeSSEStream(); honours AbortSignal
├── attachments.ts             ← composer attachment validation/compression +
│                                attachments.test.ts
├── tool-labels.ts             ← MCP tool name + args → human activity label
├── useIsMac.ts                ← navigator.platform === Mac
├── getFavicons.ts, getCurrentYear.ts   ← tiny shared helpers
│
├── browseros/                 ← BrowserOSAdapter (chrome.browserOS promise
│                                wrapper), Feature enum + Capabilities,
│                                prefs key registry, port/URL resolution
├── rpc/                       ← Hono `hc<AppType>` client + RpcClientProvider
├── graphql/                   ← execute() + useGraphqlQuery/Mutation/InfiniteQuery
├── mcp/                       ← raw JSON-RPC MCP client, MCP server storage,
│                                remote-integration sync
├── sse consumers → see ../entrypoints/sidepanel/index/useChatSession*.ts
│
├── auth/                      ← better-auth client, AuthProvider, session cache
├── analytics/                 ← PostHog init/provider, identify/resetIdentity
├── sentry/                    ← Sentry init, sanitizer, React root handlers
├── metrics/                   ← track(): chrome.browserOS.logMetric wrapper
│
├── llm-providers/             ← provider config, templates, models.dev cache,
│                                OAuth flows, GraphQL upload (graphql/)
├── llm-hub/                   ← third-party LLM hub providers (browser prefs)
├── credits/                   ← credits query hook + colour thresholds
│
├── conversations/             ← conversation storage, history formatting,
│                                GraphQL upload (graphql/)
├── agent-conversations/       ← IndexedDB (idb-keyval) agent transcripts
├── execution-history/         ← execution record types, storage, normalizer
├── schedules/                 ← cron-like jobs, chrome.alarms, backend sync
│                                (graphql/), prompt refinement
├── messaging/                 ← extension-internal message protocols:
│                                schedules/, server/, sidepanel/
│
├── chat/                      ← harness adapter visibility
├── chat-actions/              ← useChatActions composer hook + action types
├── voice/                     ← MediaRecorder hook + gateway transcription
├── onboarding/                ← first-run storage + profile sync
├── personalization/           ← debounced personalization prompt storage
├── workspace/                 ← workspace folders storage + useWorkspace
├── theme/                     ← theme preference storage
├── search-actions/            ← newtab→sidepanel handoff payload
├── selected-text/             ← per-tab text selection map
├── declined-apps/             ← per-user declined integration ids
├── stop-agent/                ← last stop signal for the active conversation
├── jtbd-popup/                ← survey popup sampling + state
├── changelog/                 ← version whitelist + one-shot notification
├── constants/                 ← product URLs, shortcuts, analytics event names
└── agent-conversations/, analytics/, …  (see tree above)
```

## Rules

- **LIB1 — This is the side-effect home.** `fetch`, `chrome.*`,
  `chrome.storage`, timers, and network state belong here. Components import
  from `lib/`; they don't call browser or network APIs themselves.
- **LIB2 — Per-user state goes through a `storage.ts` wrapper** using
  `@wxt-dev/storage`'s `local:` area. Components never touch `chrome.storage`
  directly. Two exceptions are real and worth knowing: `declined-apps/storage.ts`
  imports `storage` from `#imports` (WXT's alias) rather than
  `@wxt-dev/storage`, and `agent-conversations/storage.ts` uses
  **IndexedDB via `idb-keyval`**, not `chrome.storage`, because transcripts
  are large.
- **LIB3 — Server calls go through `rpc/getClient.ts` (Hono `hc<AppType>`),
  `graphql/execute.ts`, or the feature's own client** — never a bare
  `fetch` in a component. A handful of modules do use `fetch` directly by
  design: `voice/transcribe-audio.ts` (external gateway), `mcp/client.ts`
  (raw JSON-RPC MCP), `lib/credits/useCredits.ts`, `llm-providers/testProvider.ts`,
  `llm-providers/useOAuthStatus.ts`, `schedules/getChatServerResponse.ts`,
  `schedules/refine-prompt.ts`. Keep new direct `fetch` calls inside `lib/`.
- **LIB4 — Streaming goes through `lib/sse.ts`.** Use `consumeSSEStream()` /
  `parseSSELines()`; don't hand-roll a `TextDecoder` loop.
- **LIB5 — `env.ts` is the only env accessor.** Import `{ env }` from
  `@/lib/env`; `import.meta.env` direct reads bypass zod validation.
- **LIB6 — Port/URL resolution belongs to `lib/browseros/helpers.ts`.**
  Never hard-code `127.0.0.1:9100`; call `getAgentServerUrl()` /
  `getMcpServerUrl()`, which read the `browseros.server.*` prefs.
- **LIB7 — Tests are colocated** as `<name>.test.ts` (`mcp/client.test.ts`,
  `chat/adapter-visibility.test.ts`, `execution-history/normalize.test.ts`,
  `browseros/capabilities.test.ts`, `attachments.test.ts`, …). Run with
  `bun test`.

## Workflows

**Adding a new feature module**
1. Create `lib/<feature>/`.
2. `storage.ts` — `storage.defineItem<T>('local:<key>', { fallback })`.
3. `types.ts` — the persisted shapes.
4. `<feature>.ts` or `use<Feature>.ts` — the hook/API layer.
5. Register the `Feature` enum member in `lib/browseros/capabilities.ts`
   with its `FEATURE_CONFIG` version bounds if it needs a gate.
6. Add `lib/<feature>/AGENTS.md`.

**Adding a new MCP-server call**
1. Extend the Hono client usage: `const client = await getClient()` then
   `client.<route>` — the route types come from `@browseros/server`'s `AppType`.
2. For raw MCP JSON-RPC (tools/list, initialize), extend `lib/mcp/client.ts`
   and bump `MCP_PROTOCOL_VERSION` only if the server side moved.
3. Never call the server from a component.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — extension root (X2–X5, X8, X10).
- [`../components/AGENTS.md`](../components/AGENTS.md) — the presentational side.
- [`browseros/AGENTS.md`](browseros/AGENTS.md) — feature gates and port resolution.
- [`rpc/AGENTS.md`](rpc/AGENTS.md) — the typed server client.
- [`graphql/AGENTS.md`](graphql/AGENTS.md) — `execute()` and the query hooks.
- [`../schema/AGENTS.md`](../schema/AGENTS.md) — the hand-maintained SDL.
- [`../../../CLAUDE.md`](../../../CLAUDE.md) — extensionless imports, kebab-case files.
