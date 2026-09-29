# `src/lib/clients/oauth/` — OAuth token lifecycle

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/clients/`.

## What's here

The full OAuth round trip: the provider catalogue, the manager that drives
authorize/exchange/refresh, the Drizzle-backed token store, a lazily-bound
local callback server on port 1455, and per-provider `fetch` wrappers that
inject the resulting credentials. `index.ts` is the process-wide lifecycle
(`initializeOAuth` / `shutdownOAuth`) that `../../api/server.ts` calls.

## Contents

| File | Purpose |
|---|---|
| `providers.ts` | `OAuthProviderConfig` records — endpoints and metadata per provider, sourced from `EXTERNAL_URLS`. |
| `token-manager.ts` | The core state machine: start, exchange, refresh, revoke, status. Uses `OAUTH_CALLBACK_PORT` and `TIMEOUTS`. |
| `token-store.ts` | Drizzle persistence over the `oauth_tokens` table (`../../db/schema`). |
| `callback-server.ts` | `OAuthCallbackServer` — binds port 1455 lazily, only while an authorize flow is in flight. |
| `copilot-fetch.ts` | `fetch` wrapper for the GitHub Copilot API that attaches the Copilot token. |
| `codex-fetch.ts` | `fetch` wrapper for `https://chatgpt.com/backend-api/codex/responses`. |
| `index.ts` | `initializeOAuth(db, browserosId)` / `shutdownOAuth()`. |

## Contents (port 1455)

Port 1455 is **required by OpenAI's Codex CLI OAuth client registration** —
`redirect_uri` must be `http://localhost:1455/auth/callback`. The server is
deliberately lazy: it does not bind at startup, only during an authorize
flow, and releases the port afterwards. Do not move it to eager binding.

## Rules

**OA1 — Tokens are stored, never held in memory across restarts.** The store
is Drizzle-only (`token-store.ts` over `oauth_tokens`). No raw SQL, no
JSON-file token cache.

**OA2 — 1455 is fixed by the Codex registration.** Changing
`OAUTH_CALLBACK_PORT` breaks Codex OAuth. Use the shared constant; don't
override it locally.

**OA3 — Bind the callback server lazily and release it.** Holding 1455
permanently blocks the real Codex CLI on the same machine. The class exists
specifically to avoid the old eager-binding behaviour.

**OA4 — Wrap provider traffic in the dedicated `fetch` helpers.**
`copilot-fetch.ts` and `codex-fetch.ts` exist because those APIs need auth
and (for Codex) a different error shape. Calling the global `fetch` bypasses
that.

**OA5 — Lifecycle is owned by `index.ts`.** `api/server.ts` calls
`initializeOAuth(getDb(), browserosId)` when a browseros id exists, and
`shutdownOAuth()` when it doesn't (in which case `/oauth` returns 503). Don't
construct a second `OAuthTokenManager` elsewhere.

**OA6 — Never log tokens.** Log provider name, status, and expiry — nothing
else.

## Workflows

**Adding an OAuth provider:** 1. Add `OAuthProviderConfig` in `providers.ts`.
2. Add its token fields to `../../db/schema/oauth.ts` if they don't fit the
existing shape, then `bunx drizzle-kit generate`. 3. Add any bespoke transport
wrapper alongside `copilot-fetch.ts`. 4. Expose it in
`../../../api/routes/oauth.ts`. 5. Add tests under
`tests/lib/clients/oauth/`.

**Handling a refresh failure:** let `token-manager.ts` mark the token
unusable and surface the failure through `GET /oauth/:provider/status`;
`DELETE /oauth/:provider` is the user's escape hatch.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/clients/` conventions.
- [`../../db/schema/oauth.ts`](../../db/schema/oauth.ts) — the `oauth_tokens` table.
- [`../../../api/routes/oauth.ts`](../../../api/routes/oauth.ts) — `GET /:provider/start`, `POST /:provider/token`, `GET /:provider/status`, `DELETE /:provider`.
- [`../../../../tests/lib/clients/oauth/AGENTS.md`](../../../../tests/lib/clients/oauth/AGENTS.md) — token store and lifecycle tests.
