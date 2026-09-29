# `src/` — server source root

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server`.

## What's here

The root of the Bun MCP server's TypeScript source. `index.ts` is the
executable entry (`browseros-server` bin) — it hard-exits if `Bun` is
undefined, imports the polyfill first, loads config, and hands off to
`Application` in `main.ts`. `config.ts` owns the single `ServerConfig` type
(CLI > config file > env > defaults, zod-validated); `env.ts` holds the
build-time-inlined env vars; `types.ts` re-exports the config types; `rpc.ts`
exports the `AppType` used for typed Hono clients; `version.ts` reads the
`__BROWSEROS_VERSION__` build-time define. Everything else is one subsystem per
directory: `agent/`, `api/`, `browser/`, `lib/`, `monitoring/`, `tools/`.

## Contents

```
src/
├── index.ts        ← bun entry: runtime check, polyfill, config, start/stop
├── main.ts         ← Application class: CDP connect, HTTP server, db init
├── config.ts       ← ServerConfigSchema + loadServerConfig()
├── env.ts          ← INLINED_ENV (build-time inlined), REQUIRED_FOR_PRODUCTION
├── types.ts        ← re-exports of config types
├── rpc.ts          ← AppType = typeof createHttpServer()['app']
├── version.ts      ← VERSION from __BROWSEROS_VERSION__ define
├── agent/          ← AI agent loop (Vercel AI SDK) + compaction/
├── api/            ← Hono HTTP server: routes/, services/, utils/
├── browser/        ← Browser facade over CDP + backends/cdp.ts
├── lib/            ← shared internals: agents, clients, container, db, vm
├── monitoring/     ← tool-execution observer + LLM judge
└── tools/          ← MCP tool definitions (registry.ts is canonical)
```

## Rules

**S1 — `tools/registry.ts` is the canonical tool list.** It exports a single
`registry` built by `createRegistry([...])` with **61 tools** grouped by
commented category (Navigation 8, Observation 9, Input 17, Page Actions 3,
Windows 6, Bookmarks 6, History 4, Tab Groups 5, Info 1, Nudges 2). A tool
that is not in that array is not exposed over MCP. `ToolRegistry` throws on
duplicate names at construction.

**S2 — Config comes from `ServerConfig`, never `process.env` directly.**
`config.ts` is the only file that maps `BROWSEROS_*` env vars. `env.ts` is the
only file allowed to read build-time-inlined vars, and those reads must stay
*literal* `process.env.X` expressions or Bun's `--env inline` breaks. Required
for production: `SENTRY_DSN`, `CODEGEN_SERVICE_URL`, `POSTHOG_API_KEY`,
`BROWSEROS_CONFIG_URL`.

**S3 — No `console.log`.** Use `logger` from `lib/logger.ts` (Pino, with
dev source tracking and daily-rotation file output). No `[prefix]` tags —
Pino already adds `file:line:function`. `src/index.ts` is the one legitimate
exception: it uses `console.error` before the logger is configured.

**S4 — Drizzle only for SQL.** `lib/db/client.ts` opens `bun:sqlite` + Drizzle
and runs migrations. Never write raw SQL against the BrowserOS DB; add a
migration with `bunx drizzle-kit generate` (config: `../drizzle.config.ts`).

**S5 — Extensionless imports, kebab-case filenames.** Per
[`../../CLAUDE.md`](../../../CLAUDE.md). No `.js` suffixes; PascalCase classes live
in kebab-case files.

**S6 — `index.ts` is only a barrel, never a re-export shim for consumers.**
`src/rpc.ts` and `src/types.ts` exist for backward compatibility; new
consumers import the concrete file.

## Workflows

**Adding an MCP tool:** 1. Create `tools/<tool>.ts` extending `defineTool()`
from `tools/framework.ts` (mirror `snapshot.ts` or `dom.ts`). 2. Register it in
`tools/registry.ts` (S1). 3. Add a test at `../tests/tools/<tool>.test.ts` that
runs under `withBrowser()`. 4. Run `bun run test:tools`.

**Adding an HTTP route:** 1. Create `api/routes/<name>.ts` exporting
`create<Name>Routes(deps)`. 2. Mount it in `api/server.ts`. 3. Validate with
zod; guard with `requireTrustedAppOrigin()` where the surface is app-facing.
4. Add a test at `../tests/api/routes/<name>.test.ts`.

**Adding a DB table or column:** 1. Edit `lib/db/schema/<table>.ts`. 2. Run
`bunx drizzle-kit generate` (writes `lib/db/migrations/000N_*.sql` + `meta/`).
3. Verify the generated SQL. 4. Query through `getDb()` from `lib/db/index.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`../../AGENTS.md`](../../AGENTS.md) — Bun monorepo parent.
- [`../../CLAUDE.md`](../../../CLAUDE.md) — coding guidelines (authoritative).
- [`../../../../AGENTS.md`](../AGENTS.md) — repo root.
- [`../../../../../docs/MCP_TOOL_SPEC.md`](../../../../../docs/MCP_TOOL_SPEC.md) — captured tool schemas for the *other* (port 9200) server; useful surface comparison only.
