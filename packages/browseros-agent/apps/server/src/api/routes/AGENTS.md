# `src/api/routes/` — HTTP route modules

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/`.

## What's here

Twelve route modules, each a factory returning a `Hono<Env>` sub-app that
`../server.ts` mounts. `agents.ts` is by far the largest (793 lines) — it
exposes the full agent CRUD + sidepanel-chat surface backed by
`AgentHarnessService`. `mcp.ts` mounts the MCP transport at `/mcp`. The rest
are small, single-purpose endpoints.

## Contents

| File | Mounted at | Exports | Endpoints |
|---|---|---|---|
| `agents.ts` | `/agents` | `createAgentRoutes` | `GET /adapters`, `GET /`, `POST /`, `GET /:agentId`, `POST /:agentId/sidepanel/chat`, … |
| `chat.ts` | `/chat` | `createChatRoutes` | `POST /` (zod-validated `ChatRequestSchema`), `DELETE /…` |
| `mcp.ts` | `/mcp` | `createMcpRoutes` | `GET /` status ping, `POST /` StreamableHTTP MCP transport |
| `monitoring.ts` | `/monitoring` | `createMonitoringRoutes` | `GET /runs`, `GET /runs/:id`, `POST /debug/runs`, `POST /debug/runs/:id/finalize` |
| `klavis.ts` | `/klavis` | `createKlavisRoutes` | `GET /servers`, `GET /oauth-urls`, `GET /user-integrations`, `POST /servers/add`, `DELETE …` |
| `oauth.ts` | `/oauth` | `createOAuthRoutes` | `GET /:provider/start`, `POST /:provider/token`, `GET /:provider/status`, `DELETE /:provider` |
| `credits.ts` | `/credits` | `createCreditsRoutes` | credit balance / listing; returns 503 when unavailable |
| `health.ts` | `/health` | `createHealthRoute` | `GET /` |
| `status.ts` | `/status` | `createStatusRoute` | `GET /` |
| `shutdown.ts` | `/shutdown` | `createShutdownRoute` | `POST /` — invokes the `onShutdown` callback |
| `provider.ts` | `/test-provider` | `createProviderRoutes` | `POST /` — LLM provider verify endpoint |
| `refine-prompt.ts` | `/refine-prompt` | `createRefinePromptRoutes` | `POST /` — prompt rewriting |

## Rules

**RT1 — Factory naming is `create<X>Routes(deps)`, not `register<X>Routes`.**
The parent AGENTS files are stale on this point. `server.ts` is the only
importer of every one of these.

**RT2 — Return a sub-app, never mutate the root.** Each factory does
`const app = new Hono<Env>()` and returns it; `server.ts` decides the mount
prefix and whether a guard middleware wraps it.

**RT3 — Guard placement lives in `server.ts`, not here.** `/monitoring` and
`/agents` are wrapped with `requireTrustedAppOrigin()` at mount time. Do not
add per-handler origin checks here — you will end up with two gates.

**RT4 — Parse request headers defensively.** `mcp.ts` has
`parseOptionalNumber()` rejecting non-integers for window ids, because CDP
rejects fractional window ids with an opaque protocol error. Follow that
pattern for any header value that reaches CDP.

**RT5 — Scope ids are per-request.** `mcp.ts` reads
`X-BrowserOS-Scope-Id` (default `'ephemeral'`) and `X-BrowserOS-Default-Window-Id`
per request; the monitoring session and the tool-call observer are wired from
those, not from process globals.

**RT6 — Graceful degradation over throwing.** `credits.ts` and the `/oauth`
mount in `server.ts` return 503 with a JSON error when their dependency is
absent, rather than failing app construction.

## Workflows

**Adding an endpoint to an existing module:** add the handler inside the
existing factory in its file, keep the `Hono<Env>` instance, and reuse the
module's `deps` object. Don't create a new module for one handler.

**Adding a new route module:** create the file, export
`create<Name>Routes(deps)`, mount it in `../server.ts` with a guard if
app-facing, then add `tests/api/routes/<name>.test.ts` calling the factory
directly (see `tests/api/routes/health.test.ts` for the pattern).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/` conventions.
- [`../services/AGENTS.md`](../services/AGENTS.md) — the services these routes delegate to.
- [`../../../../tests/api/routes/AGENTS.md`](../../../tests/api/routes/AGENTS.md) — route unit tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
