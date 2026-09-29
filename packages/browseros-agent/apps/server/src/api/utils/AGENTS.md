# `src/api/utils/` — HTTP middleware and helpers

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/`.

## What's here

Small, single-purpose helpers shared by the route and service layers:
the CORS policy, the origin/loopback trust gate, zod validation helpers, the
UI-message-stream formatter, an internal MCP client, and the resolver that
maps a serialised `BrowserContext` back onto live page ids.

## Contents

| File | Purpose |
|---|---|
| `cors.ts` | `defaultCorsConfig` — permissive (`origin: (o) => o || '*'`, `GET/POST/DELETE/OPTIONS`, `Content-Type/Authorization/Accept`, `credentials: true`) because the real gate is the localhost check. |
| `request-auth.ts` | `isTrustedAppOrigin(origin)` and `requireTrustedAppOrigin()`. Trusts loopback `http`/`https` and `chrome-extension:` / `moz-extension:`; with no `Origin` header, allows only loopback-socket `GET`/`HEAD`/`OPTIONS`. |
| `security.ts` | `isLocalhostRequest(c)` — checks the actual client socket, so a spoofed `Host` header cannot pass. |
| `validation.ts` | `SESSION_ID_PATTERN = /^[a-zA-Z0-9_-]+$/` and the session-id validator built on it. |
| `ui-message-stream.ts` | `formatUIMessageStreamEvent()` for `UIMessageStreamEvent` from `@browseros/shared/schemas/ui-stream`. |
| `mcp-client.ts` | Internal `@modelcontextprotocol/sdk` `Client` for SDK routes, with typed `CallToolResult` access. |
| `resolve-browser-context-page-ids.ts` | Maps a `BrowserContext`'s serialised tabs back to live CDP page ids via the `Browser` facade. |

## Rules

**UT1 — `requireTrustedAppOrigin()` is the security boundary, not CORS.**
`defaultCorsConfig` is deliberately permissive. If you tighten CORS, you will
break the extension; if you need a real restriction, extend
`request-auth.ts` instead.

**UT2 — Trust the socket, not the header.** `security.ts` exists because
`Origin`-less requests (local reads, CLI calls) still need a gate, and a
`Host`-header check is spoofable. Never replace `isLocalhostRequest` with a
header inspection.

**UT3 — Mount the guard in `server.ts`, not per handler.** `/monitoring` and
`/agents` are wrapped at mount time. Adding a second check inside a route
produces two divergent policies.

**UT4 — Session ids are validated before they touch the filesystem or the
db.** `validation.ts`'s pattern is the allowlist; a raw session id in a path
is a traversal risk.

**UT5 — Keep these helpers side-effect free.** They take a Hono `Context` or
plain data and return data. No db handle, no config load, no `process.env`.

## Workflows

**Adding a new middleware:** put it in this folder as a
`MiddlewareHandler` factory, then apply it at the mount site in
`../server.ts` so the scope is visible in one place.

**Adding a route-scoped validation:** extend `validation.ts` for a shared
primitive (ids, tokens) and use `zValidator` inline in the route for a
request-shape rule.

**Testing:** `tests/api/request-auth.test.ts` covers the trust gate. Route
tests under `tests/api/routes/` call the factories directly and can pass a
stub `Context` for `security.ts`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/` conventions and route factories.
- [`../server.ts`](../server.ts) — where the guard and CORS are mounted.
- [`../../../../tests/api/AGENTS.md`](../../../tests/api/AGENTS.md) — auth and route tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
