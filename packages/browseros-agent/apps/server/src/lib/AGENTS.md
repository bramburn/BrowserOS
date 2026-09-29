# `src/lib/` — shared server internals

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/`.

## What's here

Cross-cutting infrastructure that neither the HTTP layer nor the tool layer
owns. The flat files are the small, always-loaded pieces: the Pino logger,
Sentry, PostHog metrics, the BrowserOS identity + directory helpers, port
binding, the process lock, a Bun polyfill, and error serialisation. The
subdirectories hold heavier subsystems: `agents/` (ACPX runtimes for Claude
Code / Codex / Hermes), `clients/` (gateway, Klavis, LLM provider creation,
OAuth token management), `container/` and `vm/` (Lima + podman sandboxing),
and `db/` (Drizzle + SQLite + migrations).

## Contents

| File | Purpose |
|---|---|
| `logger.ts` | Pino logger with dev caller tracking (`file:line:function`), daily-rotating `browseros-server.log`, pretty console in dev. |
| `sentry.ts` | Sentry init + `Sentry` re-export used at error boundaries. |
| `metrics.ts` | PostHog-backed `metrics.log(event, props)`. |
| `identity.ts` | Stable per-install BrowserOS id (`identity.getBrowserOSId()`). |
| `browseros-dir.ts` | `~/.browseros` layout: `ensureBrowserosDir`, `getDbPath`, `getVmStateDir`, `cleanOldSessions`, `writeServerConfig`, `removeServerConfigSync`. |
| `browseros-fetch.ts` | `fetch` wrapper adding the `X-BrowserOS-ID` header for credit tracking. |
| `openrouter-fetch.ts` | `fetch` wrapper that unwraps OpenRouter-style `metadata.raw` provider errors into readable messages. |
| `port-binding.ts` | `isPortInUseError()` — maps `EADDRINUSE` to a typed error for exit-code handling. |
| `process-lock.ts` | Single-instance lock over a pid file. |
| `polyfill.ts` | Imported first by `../index.ts`; installs runtime shims. |
| `serialize-error.ts` | Normalises unknown throwables into a message + cause chain. |
| `wait-for-helper.ts` | Shared polling/await helper. |
| `mcp-transport-detect.ts` | Probes an endpoint to determine its MCP transport type. |
| `agents/` | ACPX runtimes, agent store, catalog, turn registry. |
| `clients/` | Gateway, Klavis, LLM provider factory, OAuth. |
| `container/` | `podman`-backed container CLI + managed container base. |
| `db/` | Drizzle client, schema, generated migrations. |
| `vm/` | Lima VM CLI, config, paths, telemetry. |

## Rules

**L1 — `lib/` never imports from `api/` or `tools/`.** The dependency
direction is `tools/` and `api/` → `lib/` → `@browseros/shared`. A `lib/`
module that needs a `Browser` receives it as a parameter.

**L2 — Shared constants come from `@browseros/shared`.** Ports
(`constants/ports`), timeouts (`constants/timeouts`), limits
(`constants/limits`), URLs (`constants/urls`), paths (`constants/paths`),
exit codes (`constants/exit-codes`). No magic numbers in `lib/`.

**L3 — Logging only through `lib/logger.ts`.** `logger.info/debug/warn/error`.
No `console.log`, no `[Prefix]` tags.

**L4 — Config only through `ServerConfig`.** `lib/` receives resolved config
as arguments; the only files permitted to read env are `../config.ts` and
`../env.ts`.

**L5 — DB access is Drizzle-only.** `getDb()` from `db/index.ts`; migrations
are generated, never hand-edited.

**L6 — `browseros-dir.ts` is the single source of on-disk layout.** If you
need a path under `~/.browseros`, add a helper there. This is what keeps the
host paths, the Lima VM bind mounts, and the Hermes harness dirs in agreement.

## Workflows

**Adding a telemetry event:** add it in the owning module and emit with
`metrics.log('<area>.<event>', props)`. VM events are enumerated in
`vm/telemetry.ts` (`VM_TELEMETRY_EVENTS`) — extend that set rather than
free-typing a string.

**Adding a new external client:** create it under `clients/<name>/` with its
own `index.ts` barrel, following `clients/oauth/` (token store, manager,
callback server, providers) as the template.

**Testing:** mock-only tests live in `../../tests/lib/` and need no browser.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`./db/AGENTS.md`](./db/AGENTS.md) — Drizzle client, schema, migrations.
- [`./agents/AGENTS.md`](./agents/AGENTS.md) — ACPX runtimes and agent store.
- [`./container/AGENTS.md`](./container/AGENTS.md) — podman sandboxing.
- [`./vm/AGENTS.md`](./vm/AGENTS.md) — Lima VM integration.
- [`../../../../tests/lib/AGENTS.md`](../../tests/lib/AGENTS.md) — mock-only tests.
