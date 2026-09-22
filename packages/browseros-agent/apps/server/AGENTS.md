# `apps/server/` — MCP server internals

> Sub-package AGENTS file. The Bun MCP server, the open-source modern
> implementation of the BrowserOS MCP protocol. For the cross-repo
> architecture see [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md).
> For the parent monorepo see
> [`../../AGENTS.md`](../../AGENTS.md). For the build pipeline see
> [`../../../AGENTS-build.md`](../../../AGENTS-build.md).

## What's here

The MCP server. Listens on multiple ports (configurable; defaults set
in `packages/shared/src/constants/ports.ts`):
- `serverPort` — primary Hono HTTP server for `/chat`, `/agents`, etc.
- `agentPort` — internal agent API.
- `extensionPort` — extension-ws / SSE.
- `cdpPort` — outbound: connection to a running Chromium via CDP.

Communication flow:
```
AI Agent / MCP Client → Hono HTTP (serverPort) → tool handler
 ↓
 CDP (cdpPort) → BrowserOS / Chrome APIs
```

24+ MCP tools are registered in `src/tools/registry.ts`. The
`ToolRegistry` class (in `src/tools/tool-registry.ts`) deduplicates by
name and is the canonical catalog.

## Source map

```
apps/server/
├── package.json, tsconfig.json, drizzle.config.ts
├── src/
│ ├── index.ts                       ← bun entry, runtime check
│ ├── main.ts                        ← Application class (lifecycle)
│ ├── config.ts                      ← zod-validated ServerConfig
│ ├── env.ts                         ← env loader
│ ├── types.ts
│ ├── rpc.ts                         ← JSON-RPC plumbing
│ ├── version.ts
│ ├── agent/                         ← AI agent loop
│ │ ├── ai-sdk-agent.ts
│ │ ├── provider-factory.ts
│ │ ├── prompt.ts, soul-prompt.ts
│ │ ├── tool-adapter.ts
│ │ ├── session-store.ts
│ │ ├── message-normalization.ts
│ │ ├── message-validation.ts
│ │ ├── chat-mode.ts
│ │ ├── mcp-builder.ts
│ │ ├── format-message.ts
│ │ ├── errors.ts
│ │ └── compaction/                  ← long-history compaction
│ ├── api/                           ← Hono HTTP server
│ │ ├── server.ts
│ │ ├── types.ts
│ │ ├── routes/                      ← 12+ route modules
│ │ │ ├── agents.ts
│ │ │ ├── chat.ts
│ │ │ ├── credits.ts
│ │ │ ├── health.ts
│ │ │ ├── klavis.ts
│ │ │ ├── mcp.ts
│ │ │ ├── monitoring.ts
│ │ │ ├── oauth.ts
│ │ │ ├── provider.ts
│ │ │ ├── refine-prompt.ts
│ │ │ ├── shutdown.ts
│ │ │ └── status.ts
│ │ ├── services/
│ │ │ ├── chat-service.ts
│ │ │ ├── agents/agent-harness-service.ts
│ │ │ ├── klavis/strata-cache.ts, strata-proxy.ts
│ │ │ └── mcp/mcp-prompt.ts, mcp-server.ts, register-mcp.ts
│ │ └── utils/cors.ts, mcp-client.ts, request-auth.ts
│ ├── browser/                       ← CDP client
│ │ ├── browser.ts
│ │ └── backends/cdp.ts
│ ├── lib/                           ← shared internals
│ │ ├── agents/runtime.ts            ← Claude/Codex/Hermes config
│ │ ├── clients/
│ │ ├── container/                   ← sandboxing
│ │ ├── db/                          ← Drizzle + SQLite
│ │ │ ├── client.ts, index.ts
│ │ │ ├── schema/index.ts            ← all schemas
│ │ │ └── migrations/                ← Drizzle-generated SQL
│ │ ├── vm/                          ← Lima VM integration
│ │ ├── port-binding.ts
│ │ ├── sentry.ts
│ │ ├── polyfill.ts
│ │ └── browseros-dir.ts             ← writes ~/.browseros/server-config.json
│ ├── monitoring/                    ← eval judge + monitoring
│ │ └── judge/
│ ├── tools/                         ← MCP tools (24+)
│ │ ├── framework.ts                 ← ToolDefinition base
│ │ ├── registry.ts                  ← CANONICAL tool list (R10)
│ │ ├── tool-registry.ts             ← ToolRegistry class
│ │ ├── tool-label-registry.ts
│ │ ├── response.ts
│ │ ├── snapshot.ts
│ │ ├── dom.ts
│ │ ├── navigation.ts
│ │ ├── page-actions.ts
│ │ ├── input.ts
│ │ ├── console.ts
│ │ ├── history.ts
│ │ ├── bookmarks.ts
│ │ ├── tab-groups.ts
│ │ ├── windows.ts
│ │ ├── nudges.ts
│ │ ├── output-file.ts
│ │ ├── browseros-info.ts
│ │ ├── ElementProperties/           ← CDP element property helpers
│ │ ├── filesystem/                  ← bash, edit, find, grep, ls, read, write, utils
│ │ └── acl/                         ← agentic contrastive learning
│ │   ├── acl-scorer.ts
│ │   ├── acl-edit-distance.ts
│ │   ├── acl-embeddings.ts
│ │   └── acl-stopwords.ts
│ ├── graph/                         ← (Drizzle schema placeholder)
│ └── tests/                         ← see "Testing" below
├── graph/                           ← top-level Drizzle config dir
├── tests/                           ← see "Testing" below
└── README.md
```

## Opinionated rules

### S1 — `tools/registry.ts` is the canonical tool list
Per architecture Rule R10, every tool is imported + exported in
`src/tools/registry.ts`. The `ToolRegistry` class deduplicates by
name and throws on duplicates. **Before opening a PR for a new tool,
that file is updated.**

### S2 — `ToolDefinition` is the contract
Every tool extends `ToolDefinition` (in `src/tools/framework.ts`).
The contract: name, description, zod-validated input schema, async
handler returning a typed result. ACL scorer and tool-label-registry
auto-pick up tools at startup.

### S3 — Hono + zod for all routes
`api/server.ts` builds the Hono app. Each route file exports
`register<X>Routes(app: Hono)`. Validate input with zod. Auth via
`api/utils/request-auth.ts`. Don't return raw `Response` objects.

### S4 — Drizzle, not raw SQL
DB access via Drizzle ORM (`src/lib/db/client.ts`). Schema in
`src/lib/db/schema/<table>.ts`. Migrations generated by
`bunx drizzle-kit generate`. Repos in `src/lib/db/<repo>.ts`.

### S5 — Sentry at the boundary
Wrap route handlers in `Sentry.startTransaction`. Don't sprinkle
`Sentry.captureException` everywhere — use Sentry's auto-instrumentation.

### S6 — Pino or built-in logger
Use the logger interface from `@browseros/shared/types/logger`. No
`console.log`. No `[prefix]` tags (per architecture Rule R5).

### S7 — Tests mirror src structure
Tool tests live at `apps/server/tests/tools/<tool>.test.ts`. Route
integration tests at `apps/server/tests/integration/`. Helpers at
`apps/server/tests/__helpers__/`. Fixtures at
`apps/server/tests/__fixtures__/`.

### S8 — Don't read env directly
Use `ServerConfig` (loaded via `config.ts`). Env vars are mapped
in `env.ts`. Magic numbers go in `@browseros/shared`.

## Opinionated workflow

### "Add an MCP tool"
1. Create `apps/server/src/tools/<tool>.ts`. Extend `ToolDefinition`
 from `src/tools/framework.ts`. Use `snapshot.ts` or `dom.ts` as
 templates.
2. **Register in `src/tools/registry.ts`** (S1). Add the import and
 add to the tools array.
3. **Add fixtures** at `apps/server/tests/tools/__fixtures__/<tool>/`
 (HTML snapshots, JSON responses).
4. **Add a test** `apps/server/tests/tools/<tool>.test.ts`.
5. Run `bun run test:tools`.
6. If your tool is in the `acl/` family, also add a scorer entry.
7. Cross-ref in `apps/agent/components/mcp-settings/`.

### "Add an HTTP route"
1. Create `apps/server/src/api/routes/<route>.ts` exporting a Hono
 handler.
2. **Register in `src/api/server.ts`** (S3).
3. Use `request-auth.ts` for auth checks.
4. Validate input with zod.
5. Integration test at `tests/integration/<route>.integration.test.ts`.
6. Run `bun run test:integration`.

### "Add an LLM provider"
1. Create `apps/server/src/agent/provider-<name>.ts`. If it's an
 `@ai-sdk/<name>` provider, wrap the SDK.
2. **Register in `src/agent/provider-factory.ts`**.
3. Add config key to `ServerConfigSchema` in `src/config.ts`.
4. Add to `packages/browseros-agent/config.sample.json`.
5. UI in `apps/agent/components/ai-settings/`.

### "Add a DB table / column"
1. Edit `apps/server/src/lib/db/schema/<table>.ts`.
2. Run `bunx drizzle-kit generate`.
3. Verify the migration `.sql` file at
 `apps/server/src/lib/db/migrations/000N_<name>.sql`.
4. Update repo methods.
5. Test with `bun run test:integration` (Drizzle auto-migrates).

### "Add a long-running tool that needs a custom timeout"
1. Import the timeout from `@browseros/shared/constants/timeouts`
 (or add a new constant there).
2. Use it in your `ToolDefinition.handler` wrapper.
3. Document the timeout in the tool's JSDoc.

### "Add a new monitoring/eval check"
1. Add the check at `apps/server/src/monitoring/judge/<check>.ts`.
2. **Register** in the monitoring service.
3. Test at `apps/server/tests/monitoring-judge.test.ts`.

## Testing

```bash
# From packages/browseros-agent
bun run test                 # all tool tests
bun run test:tools           # alias for tool tests
bun run test:integration     # HTTP route integration
bun run test:sdk             # agent SDK
bun run test:agent           # agent tests
bun run test -- --watch      # watch mode
bun --env-file=.env.development test apps/server/tests/path/to/file.test.ts
```

Tool tests need a running BrowserOS with a CDP port. The test harness
boots the server in-process, talks to a real Chromium, and exercises
each tool's full handler. Mock-only tests live at `tests/lib/`.

## Cross-references

- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — overall map.
- [`../../../AGENTS-build.md`](../../../AGENTS-build.md) — build pipeline.
- [`../../CLAUDE.md`](../../CLAUDE.md) — coding guidelines.
- [`../../docs/MCP_TOOL_SPEC.md`](../../../docs/MCP_TOOL_SPEC.md) — captured tool schemas for port 9200 (different protocol, similar surface).
