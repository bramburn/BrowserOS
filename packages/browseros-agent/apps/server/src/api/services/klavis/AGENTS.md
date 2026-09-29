# `src/api/services/klavis/` — Klavis proxy and cache

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/api/services/`.

## What's here

Integration with the external **Klavis** MCP service. `strata-proxy.ts` owns
the live connection and exposes Klavis's tools to the MCP server; `strata-cache.ts`
is an in-process cache for Klavis's `createStrata` response, which exists
because that call is Worker-proxied and was blocking `/chat`.

## Contents

| File | Purpose |
|---|---|
| `strata-proxy.ts` | `KlavisProxyHandle`, `KlavisProxyRef` (`{ handle: null }` initially), `connectKlavisProxy`, `connectKlavisInBackground` (retrying, non-blocking), `buildKlavisToolSet(handle)`, `registerKlavisTools`. |
| `strata-cache.ts` | `KlavisStrataCache` class + the `klavisStrataCache` module singleton. |

## Contents (context)

The Klavis *client* itself lives one level up the tree at
[`../../../lib/clients/klavis/`](../../../lib/clients/klavis/) —
`klavis-client.ts` and `oauth-mcp-servers.ts`. The split is: the client
speaks the API, this folder wires it into the server's request lifecycle.

## Rules

**KV1 — Connect in the background; never block startup.** `server.ts` calls
`connectKlavisInBackground(klavisRef, { klavisClient, browserosId })` so
browser tools are available immediately. Do not move this to an awaited call
in `Application.start()`.

**KV2 — Treat `handle` as nullable.** `KlavisProxyRef.handle` starts as
`null` and stays null if the background connect keeps failing. Every
consumer must tolerate that; `/shutdown` does
`klavisRef.handle?.close().catch(...)` with a `logger.warn` on failure.

**KV3 — Cache `createStrata`; don't re-fetch per turn.** Conversation
creation in `/chat` was blocking on this. If you change the cache key or
TTL, update `strata-cache.test.ts`, which pins the behaviour.

**KV4 — URLs and timeouts come from `@browseros/shared`.** `TIMEOUTS` and
`EXTERNAL_URLS` — never inline a Klavis host or a wait duration.

**KV5 — Tool names registered here are Klavis's, not ours.** They are added
by `registerKlavisTools` on top of the browser registry; they are not in
`../../../../tools/registry.ts` and must not be added there.

## Workflows

**Adding a Klavis-backed tool:** 1. Add it in `oauth-mcp-servers.ts`
(`../../../lib/clients/klavis/`) with the exact name the Klavis API expects.
2. Confirm `buildKlavisToolSet` / `registerKlavisTools` surface it. 3. Add a
route under `../../routes/klavis.ts` if the extension needs to drive it.

**Debugging a Klavis outage:** check the background-connect logger warnings
from `connectKlavisInBackground` first — the server is designed to keep
working with `handle === null`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/api/services/` conventions.
- [`../../../lib/clients/klavis/AGENTS.md`](../../../lib/clients/klavis/AGENTS.md) — the Klavis HTTP client and OAuth server catalogue.
- [`../../../../tests/api/services/klavis/AGENTS.md`](../../../../tests/api/services/klavis/AGENTS.md) — proxy and cache tests.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
