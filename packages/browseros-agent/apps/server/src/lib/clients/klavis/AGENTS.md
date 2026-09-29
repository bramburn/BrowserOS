# `src/lib/clients/klavis/` — Klavis service client

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/clients/`.

## What's here

The HTTP-side client for **Klavis**, the external service that brokers
integrations with 40+ third-party MCP servers. `klavis-client.ts` is the API
client; `oauth-mcp-servers.ts` is the catalogue of OAuth-enabled servers the
user can connect. The live proxying and caching that sit on top of these live
two levels up in
[`../../../api/services/klavis/`](../../../api/services/klavis/AGENTS.md).

## Contents

| File | Purpose |
|---|---|
| `klavis-client.ts` | `KlavisClient` — calls into the Klavis API (session/user/integration lifecycle). Uses `TIMEOUTS` and `EXTERNAL_URLS`. |
| `oauth-mcp-servers.ts` | `OAuthMcpServer` records (`name`, `description`, …). The `name` field is the exact string to pass to the Klavis API — it is not a display label. |

## Rules

**KC1 — Server names are exact API identifiers.** `OAuthMcpServer.name` is
passed verbatim to Klavis. Renaming it for readability in the UI breaks the
integration; display text belongs in `description`.

**KC2 — All endpoints from `EXTERNAL_URLS`, all waits from `TIMEOUTS`.** No
literals in `klavis-client.ts`.

**KC3 — Construct the client in `../../../api/server.ts`.** It is passed into
`connectKlavisInBackground` and into the Klavis routes as `deps`; nothing else
should instantiate it.

**KC4 — The cache lives in the service layer, not here.** `klavis-client.ts`
performs the request; `strata-cache.ts` decides whether it is needed. Don't
add memoisation to the client.

## Workflows

**Adding an OAuth-enabled MCP server:** 1. Add a `OAuthMcpServer` record in
`oauth-mcp-servers.ts` with the exact Klavis `name`. 2. Confirm it appears via
`GET /klavis/servers`. 3. Confirm `GET /klavis/oauth-urls` returns a URL for
it.

**Debugging a Klavis outage:** remember the server keeps working with a null
proxy handle — see the cross-reference below before assuming the MCP server
is down.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/clients/` conventions.
- [`../../../api/services/klavis/AGENTS.md`](../../../api/services/klavis/AGENTS.md) — `strata-proxy.ts` and `strata-cache.ts`.
- [`../../../api/routes/klavis.ts`](../../../api/routes/klavis.ts) — the HTTP surface.
- [`../../../../tests/api/services/klavis/AGENTS.md`](../../../../tests/api/services/klavis/AGENTS.md) — proxy and cache tests.
