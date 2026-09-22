# `packages/browseros-agent/` — Bun monorepo entry

> Sub-package AGENTS file. The Bun + TypeScript side of the BrowserOS
> monorepo: the open-source MCP server, the WXT browser extension, the
> Go CLI, the eval harness, and shared constants. For the cross-repo
> architecture see [`../../AGENTS-architecture.md`](../../AGENTS-architecture.md).
> For the build pipeline see [`../../AGENTS-build.md`](../../AGENTS-build.md).

## What's here

A Bun-managed monorepo. Apps (runnable products) live in `apps/`;
shared libraries (used by apps) live in `packages/`. All packages
share a single `bun.lock` and a single `tsconfig.json`. Linting is
Biome (`biome.json`); pre-commit hooks via `lefthook.yml`; dev
orchestration via `process-compose.yaml`.

```
packages/browseros-agent/
├── README.md                       ← high-level overview
├── CLAUDE.md                       ← coding guidelines (read first)
├── package.json                    ← root scripts: dev, test, lint, dist
├── bun.lock, bun.lockb             ← Bun lockfile
├── bunfig.toml                     ← Bun config
├── tsconfig.json                   ← base TS config
├── biome.json                      ← Biome lint config
├── lefthook.yml                    ← pre-commit hooks
├── process-compose.yaml            ← dev process orchestration
├── config.sample.json              ← sample config for users
├── config.dev.json                 ← dev-only config
├── server.json                     ← bundled MCP server release manifest
├── skills-lock.json                ← locked skill list
├── docs/                           ← additional package docs
├── third_party/                    ← vendored deps
├── apps/
│ ├── server/                       ← Bun MCP server (modern, open-source)
│ │ ├── AGENTS.md
│ │ ├── package.json, tsconfig.json, drizzle.config.ts
│ │ ├── src/
│ │ │ ├── index.ts                  ← bun entry
│ │ │ ├── main.ts                   ← Application class
│ │ │ ├── config.ts                 ← zod-validated config
│ │ │ ├── env.ts                    ← env loader
│ │ │ ├── types.ts
│ │ │ ├── rpc.ts
│ │ │ ├── version.ts
│ │ │ ├── agent/                    ← AI agent loop
│ │ │ │ ├── ai-sdk-agent.ts
│ │ │ │ ├── provider-factory.ts     ← LLM provider dispatch
│ │ │ │ ├── prompt.ts, soul-prompt.ts
│ │ │ │ ├── tool-adapter.ts
│ │ │ │ ├── session-store.ts        ← per-session state
│ │ │ │ ├── message-normalization.ts
│ │ │ │ ├── message-validation.ts
│ │ │ │ ├── chat-mode.ts
│ │ │ │ ├── mcp-builder.ts
│ │ │ │ ├── format-message.ts
│ │ │ │ ├── errors.ts
│ │ │ │ └── compaction/             ← long-history compaction
│ │ │ ├── api/                      ← Hono HTTP server
│ │ │ │ ├── server.ts               ← app + route registration
│ │ │ │ ├── types.ts
│ │ │ │ ├── routes/                 ← agents, chat, credits, health, mcp, oauth, …
│ │ │ │ ├── services/               ← chat-service, agent-harness-service, mcp/, klavis/
│ │ │ │ └── utils/                  ← cors, mcp-client, request-auth
│ │ │ ├── browser/                  ← CDP client + backends
│ │ │ │ ├── browser.ts
│ │ │ │ └── backends/cdp.ts
│ │ │ ├── lib/                      ← shared internals
│ │ │ │ ├── agents/runtime.ts       ← Claude/Codex/Hermes runtime config
│ │ │ │ ├── clients/
│ │ │ │ ├── container/              ← sandboxing
│ │ │ │ ├── db/                     ← Drizzle + SQLite
│ │ │ │ │ ├── client.ts, index.ts
│ │ │ │ │ ├── schema/               ← table defs
│ │ │ │ │ └── migrations/           ← SQL migrations
│ │ │ │ ├── vm/                     ← Lima VM integration
│ │ │ │ ├── port-binding.ts
│ │ │ │ ├── sentry.ts
│ │ │ │ ├── polyfill.ts
│ │ │ │ └── browseros-dir.ts        ← writes server-config.json
│ │ ├── monitoring/                  ← eval judge + monitoring
│ │ ├── tools/                       ← MCP tools (24+)
│ │ ├── graph/                       ← Drizzle schema (?)
│ ├── agent/                        ← WXT browser extension
│ │ ├── AGENTS.md
│ │ ├── wxt.config.ts
│ │ ├── web-ext.config.ts
│ │ ├── codegen.ts
│ │ ├── components.json
│ │ ├── assets/mcp-icons/
│ │ ├── schema/schema.graphql       ← GraphQL schema
│ │ ├── components/                  ← React UI
│ │ │ ├── ai-elements/              ← (likely) ai-elements shadcn-ui port
│ │ │ ├── auth/
│ │ │ ├── chat/
│ │ │ ├── credits/
│ │ │ ├── elements/
│ │ │ ├── execution-history/
│ │ │ ├── sidebar/
│ │ │ ├── theme-provider.tsx
│ │ │ └── ui/                       ← shadcn primitives
│ │ ├── entrypoints/                 ← WXT entries
│ │ │ ├── background/               ← service worker
│ │ │ ├── content.ts                ← content scripts
│ │ │ ├── app/                      ← extension-app routes
│ │ │ │ ├── agent-command/
│ │ │ │ ├── agents/
│ │ │ │ ├── ai-settings/
│ │ │ │ ├── connect-mcp/
│ │ │ │ ├── customization/
│ │ │ │ ├── jtbd-agent/
│ │ │ │ ├── layout/
│ │ │ │ ├── llm-hub/
│ │ │ │ ├── login/
│ │ │ │ ├── mcp-settings/
│ │ │ │ ├── profile/
│ │ │ │ ├── scheduled-tasks/
│ │ │ │ └── usage/
│ │ │ ├── newtab/                   ← new-tab page
│ │ │ ├── onboarding/               ← first-run flow
│ │ │ ├── sidepanel/                ← chat side panel
│ │ │ ├── glow.content
│ │ │ ├── auth.content
│ │ │ ├── selection.content.ts
│ │ │ └── ...
│ │ ├── hooks/use-mobile.ts
│ │ ├── lib/                        ← 30+ feature modules
│ │ │ ├── agent-conversations/
│ │ │ ├── analytics/
│ │ │ ├── auth/
│ │ │ ├── browseros/
│ │ │ ├── changelog/
│ │ │ ├── chat/, chat-actions/
│ │ │ ├── constants/
│ │ │ ├── conversations/
│ │ │ ├── credits/
│ │ │ ├── declined-apps/
│ │ │ ├── env.ts
│ │ │ ├── execution-history/
│ │ │ ├── getCurrentYear.ts, getFavicons.ts
│ │ │ ├── graphql/
│ │ │ ├── jtbd-popup/
│ │ │ ├── llm-hub/
│ │ │ ├── llm-providers/
│ │ │ ├── mcp/
│ │ │ ├── messaging/
│ │ │ ├── metrics/
│ │ │ ├── onboarding/
│ │ │ ├── personalization/
│ │ │ ├── rpc/
│ │ │ ├── schedules/
│ │ │ ├── search-actions/
│ │ │ ├── selected-text/
│ │ │ ├── sentry/
│ │ │ ├── sse.ts
│ │ │ ├── stop-agent/
│ │ │ ├── theme/
│ │ │ ├── tool-labels.ts
│ │ │ ├── useIsMac.ts
│ │ │ ├── utils.ts
│ │ │ ├── voice/
│ │ │ └── workspace/
│ │ ├── styles/, public/
│ │ └── assets/
│ ├── cli/                          ← Go CLI (browseros v2+)
│ │ ├── AGENTS.md
│ │ ├── main.go, go.mod, go.sum, Makefile
│ │ ├── cmd/                        ← cobra-style commands
│ │ ├── config/, mcp/, scripts/
│ │ ├── analytics/, output/
│ │ ├── integration_test.go
│ │ └── npm/                        ← node-side glue (?)
│ └── eval/                         ← Eval / benchmark harness
│ ├── AGENTS.md
│ ├── package.json, tsconfig.json
│ ├── README.md
│ ├── src/, tests/
│ ├── configs/, data/
│ └── scripts/
├── packages/
│ ├── shared/                       ← TS constants & types
│ │ ├── AGENTS.md
 │ │ ├── package.json
 │ │ ├── src/
 │ │ │ ├── constants/
 │ │ │ │ ├── ports.ts
 │ │ │ │ ├── timeouts.ts
 │ │ │ │ ├── limits.ts
 │ │ │ │ ├── urls.ts
 │ │ │ │ ├── paths.ts
 │ │ │ │ └── exit-codes.ts
 │ │ │ ├── types/
 │ │ │ │ └── logger.ts
 │ │ │ └── ...
 │ │ └── ...
 │ ├── build-tools/                 ← build helpers
 │ └── cdp-protocol/                ← Chrome DevTools Protocol type defs
├── scripts/                        ← dev scripts (incl. inspect-ui.ts)
└── tools/                          ← misc build/dev helpers
```

## Opinionated rules

These combine with the global rules in
[`AGENTS-architecture.md`](../../AGENTS-architecture.md) R1–R14.

### B1 — Bun is the runtime
Don't write `node` scripts. Bun only. `apps/server/src/index.ts`
exits early if `Bun === undefined`.

### B2 — No `index.ts` re-exports
Per architecture Rule R3. Export from named files and import directly.
The `package.json` `exports` field must list each one:

```json
"exports": {
 "./constants/ports": {
 "types": "./src/constants/ports.ts",
 "default": "./src/constants/ports.ts"
 }
}
```

### B3 — Bun test, not Jest/Vitest
`bun test` runs `*.test.ts` files. Test fixtures in `__fixtures__/`
(next to `apps/server/tests/__fixtures__/`). Helpers in
`__helpers__/`. Integration tests separately under
`apps/server/tests/integration/`.

### B4 — Server entry is `apps/server/src/index.ts`
The `Application` class lives in `main.ts`. Don't bypass — go through
`Application.start()` so Sentry init, config load, and shutdown hooks
all fire.

### B5 — Extension entry points are WXT-discovered
WXT scans `apps/agent/entrypoints/` for files matching
`{name}/index.tsx` or `{name}.ts`. Don't fight the convention.

### B6 — One tool = one file (or family dir)
For simple tools, `apps/server/src/tools/<tool>.ts`. For a tool
family (e.g. filesystem), `apps/server/src/tools/<family>/<tool>.ts`
+ `apps/server/src/tools/<family>/build-toolset.ts` aggregator. The
`tools/registry.ts` file is the canonical list — *Rule R10*.

### B7 — Server tests require a running BrowserOS
Tool tests (`bun run test:tools`) hit a real CDP port. The
`tests/__helpers__/` set up the test server. Integration tests use
the same helper. Mock tests live separately in `tests/lib/` (no
external dependencies).

### B8 — GraphQL schema is hand-maintained
`apps/agent/schema/schema.graphql` is the source. Don't auto-generate
from TS — keep it human-readable for codegen.

### B9 — Don't import from `apps/` in `packages/`
`packages/shared` is reusable; `apps/*` are not. Keep the import
graph strictly one-way: `apps → packages`, never `packages → apps`.

### B10 — Use `CLAUDE.md` for code style questions
The coding rules in `packages/browseros-agent/CLAUDE.md` are
authoritative. They cover Bun preferences, file naming (kebab-case),
imports (extensionless), and no `index.ts` re-exports.

## Opinionated workflow

### "I'm adding a new MCP tool"
1. Create `apps/server/src/tools/<tool>.ts` (or
 `apps/server/src/tools/<family>/<tool>.ts`).
2. Implement the tool extending `ToolDefinition` from
 `apps/server/src/tools/framework.ts`. Use `snapshot.ts` or `dom.ts`
 as templates.
3. **Register** in `apps/server/src/tools/registry.ts` (R10).
4. Add fixtures under `apps/server/tests/tools/__fixtures__/<tool>/`.
5. Add a test `apps/server/tests/tools/<tool>.test.ts`.
6. Run `bun run test:tools`.
7. Cross-ref in `apps/agent/components/mcp-settings/` so users see
 the new tool.

### "I'm adding an HTTP route"
1. Create `apps/server/src/api/routes/<route>.ts` exporting a
 Hono handler function `register<X>Routes(app)`.
2. **Register** in `apps/server/src/api/server.ts`.
3. Auth check via `apps/server/src/api/utils/request-auth.ts`.
4. Zod validation in the route file.
5. Integration test in `apps/server/tests/integration/`.
6. Run `bun run test:integration`.

### "I'm adding an LLM provider"
1. New file `apps/server/src/agent/provider-<name>.ts` (or extend
 existing if it's `@ai-sdk/<name>` based).
2. **Register** in `apps/server/src/agent/provider-factory.ts`.
3. Add config key to `packages/browseros-agent/config.sample.json`
 and `ServerConfigSchema` in `apps/server/src/config.ts`.
4. UI: extend `apps/agent/components/ai-settings/` provider list.
5. Run `bun run generate:models` to refresh `models-dev` cache.

### "I'm adding an extension route"
1. Create `apps/agent/entrypoints/app/<route>/index.tsx` + components.
2. WXT auto-discovers. No manifest update needed.
3. Add nav entry in `apps/agent/components/sidebar/` and the relevant
 `entrypoints/app/layout/`.
4. Move side-effect logic into `apps/agent/lib/<feature>/` so it's
 reusable.
5. Test via `bun scripts/dev/inspect-ui.ts` (CDP inspector —
 see `CLAUDE.md` § "Self-Testing UI Changes").

### "I'm changing the DB schema"
1. Edit `apps/server/src/lib/db/schema/<table>.ts`.
2. Run `bunx drizzle-kit generate` — produces
 `apps/server/src/lib/db/migrations/000N_<name>.sql`.
3. Verify the migration.
4. Update repo methods in `apps/server/src/lib/db/<repo>.ts`.
5. Run `bun run test:integration` — Drizzle applies migrations
 in test setup.

### "I'm adding a feature flag (runtime toggle)"
1. Add to `apps/agent/lib/llm-providers/<provider>/flags.ts` (or
 feature-specific `lib/<feature>/flags.ts`).
2. Add UI to `apps/agent/components/ai-settings/` or the relevant
 settings page.
3. Persist in `lib/<feature>/storage.ts` (chrome.storage.local wrapper).

## Cross-references

- [`AGENTS-architecture.md`](../../AGENTS-architecture.md) — overall map.
- [`AGENTS-build.md`](../../AGENTS-build.md) — full build pipeline.
- [`CLAUDE.md`](CLAUDE.md) — coding guidelines (authoritative for this sub-package).
- [`apps/server/AGENTS.md`](apps/server/AGENTS.md) — MCP server internals.
- [`apps/agent/AGENTS.md`](apps/agent/AGENTS.md) — extension internals.
- [`apps/cli/AGENTS.md`](apps/cli/AGENTS.md) — Go CLI.
- [`apps/eval/AGENTS.md`](apps/eval/AGENTS.md) — eval harness.
- [`packages/shared/AGENTS.md`](packages/shared/AGENTS.md) — shared constants.
- [`docs/MCP_TOOL_SPEC.md`](../../docs/MCP_TOOL_SPEC.md) — captured tool schemas (port 9200).
