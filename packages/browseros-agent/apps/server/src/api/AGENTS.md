# `src/api/` — Hono HTTP server

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/`.

## What's here

The single consolidated Hono server. `server.ts` builds the `Hono<Env>` app,
pre-flight-checks the port, and mounts every route module; `types.ts` holds
the `Env` bindings, `HttpServerConfig`, and the zod request schemas shared by
the agent and chat surfaces. The route modules live in `routes/`, the
non-trivial behaviour in `services/`, and the shared middleware/helpers in
`utils/`. Two surfaces sit behind `requireTrustedAppOrigin()` (`/monitoring`
and `/agents`); the rest are mounted directly with a permissive CORS policy.

## Contents

| File | Purpose |
|---|---|
| `server.ts` | `createHttpServer(config)` — port pre-flight, CORS, OAuth init, Klavis background connect, route mounting, WS/MCP wiring. |
| `types.ts` | `Env`, `HttpServerConfig`, `AgentLLMConfigSchema`, `ChatRequestSchema`; re-exports `BrowserContext`/`Tab` schemas from `@browseros/shared`. |
| `routes/` | 12 route modules — see [`routes/AGENTS.md`](routes/AGENTS.md). |
| `services/` | `chat-service.ts` plus `agents/`, `klavis/`, `mcp/` — see [`services/AGENTS.md`](services/AGENTS.md). |
| `utils/` | CORS, request auth, security, zod validation, UI stream formatting, MCP client, browser-context page-id resolution — see [`utils/AGENTS.md`](utils/AGENTS.md). |

## Rules

**API1 — Route modules are `create<X>Routes(deps)` factories, not
`register<X>Routes(app)`.** Each returns a `Hono<Env>` sub-app that
`server.ts` mounts with `.route('/prefix', ...)`. (Note: the parent AGENTS
files say `register<X>Routes` — that is stale; the real exports are
`createHealthRoute`, `createAgentRoutes`, `createMcpRoutes`, and so on.)

**API2 — Every new route module must be mounted in `server.ts`.** A factory
that is imported nowhere is dead code; the app is assembled in exactly one
place so route ordering and middleware scope stay auditable.

**API3 — Guard app-facing surfaces with `requireTrustedAppOrigin()`.** It
allows loopback `http(s)` origins and `chrome-extension:` /
`moz-extension:` origins, and falls back to a loopback socket check for
origin-less `GET`/`HEAD`/`OPTIONS` only. `/monitoring` and `/agents` use a
wrapper `Hono` for this; do not bypass it.

**API4 — Validate request bodies with zod at the handler boundary**
(`zValidator('json', Schema)` from `@hono/zod-validator`). Schemas that span
routes live in `types.ts`.

**API5 — Return `c.json(...)`, never a raw `Response`.** Status codes use
Hono's `ContentfulStatusCode` typing.

**API6 — CORS stays permissive; auth is the localhost/origin check.**
`utils/cors.ts` allows any origin because the real gate is
`requireTrustedAppOrigin()` plus the CDP-loopback assumption. Tightening one
without the other breaks the extension.

## Workflows

**Adding a route:** 1. Create `routes/<name>.ts` exporting
`create<Name>Routes(deps)` returning `new Hono<Env>()`. 2. Mount it in
`server.ts` under a path prefix (API2). 3. Add the guard middleware if the
surface is app-facing (API3). 4. Validate the body with zod (API4). 5. Put
reusable logic in `services/`, not in the handler. 6. Add
`tests/api/routes/<name>.test.ts` — those call the factory directly, so no
server boot is required.

**Wiring a new service into a route:** services are constructed in
`server.ts` (e.g. `new KlavisClient()`) and passed down as `deps`, which
keeps handlers testable with fakes.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `apps/server/` MCP server internals.
- [`./routes/AGENTS.md`](routes/AGENTS.md) — the 12 route modules.
- [`./services/AGENTS.md`](services/AGENTS.md) — chat, agents, klavis, MCP services.
- [`./utils/AGENTS.md`](utils/AGENTS.md) — middleware and helpers.
- [`../../../../docs/MCP_TOOL_SPEC.md`](../../../../../../docs/MCP_TOOL_SPEC.md) — the port 9200 server's tool schemas, for protocol comparison.
