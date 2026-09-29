# `src/lib/clients/` — outbound service clients

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/src/lib/`.

## What's here

Clients for everything the server talks *out* to. `gateway.ts` sits at the
root of the folder; the subdirectories each own one integration: `klavis/`
(the external MCP service), `llm/` (Vercel AI SDK model creation and the
lightweight text client), and `oauth/` (provider token acquisition, storage,
and the local callback server). All of them read their endpoints and timeouts
from `@browseros/shared/constants/`.

## Contents

| Path | Purpose |
|---|---|
| `gateway.ts` | The BrowserOS gateway client — `Provider` interface and the calls the server makes through the gateway (credit tracking, config, `X-BrowserOS-ID` attribution). |
| `klavis/` | Klavis HTTP client + the OAuth-enabled MCP server catalogue. See [`klavis/AGENTS.md`](klavis/AGENTS.md). |
| `llm/` | LLM config resolution, Vercel AI SDK provider creation, a lightweight text client, prompt refinement, and mock/test providers. See [`llm/AGENTS.md`](llm/AGENTS.md). |
| `oauth/` | OAuth token lifecycle: providers, manager, Drizzle-backed store, lazy port-1455 callback server, and per-provider fetch wrappers. See [`oauth/AGENTS.md`](oauth/AGENTS.md). |

## Contents (shared conventions)

| Concern | Convention |
|---|---|
| Endpoints | `EXTERNAL_URLS` from `@browseros/shared/constants/urls`. |
| Timeouts | `TIMEOUTS` from `@browseros/shared/constants/timeouts`. |
| Attribution | `X-BrowserOS-ID` header via `../browseros-fetch.ts`. |
| Logging | `logger` from `../logger.ts`. |
| Persistence | Drizzle, via the store modules — never raw SQL. |

## Rules

**CL1 — No endpoint literals.** Every external base URL comes from
`EXTERNAL_URLS`. A hard-coded `https://…` in a client is a bug; the gateway
and the extension both rely on those constants being the single source.

**CL2 — Clients take config, they don't read it.** A client receives its
browseros id, db handle, or logger as constructor/`init` arguments.
`initializeOAuth(db, browserosId)` is the pattern.

**CL3 — Identity headers on every gateway call.** Anything routed through
`gateway.ts` or `browseros-fetch.ts` must carry `X-BrowserOS-ID`, otherwise
credits are not attributed to the install.

**CL4 — Failures are typed and logged, never swallowed silently.** A client
that degrades (Klavis, judge) logs a warning; one that must fail (OAuth token
read) throws.

**CL5 — Test seams live beside the real implementation.** `llm/test-provider.ts`
and `llm/mock-language-model.ts` are the sanctioned fakes — don't add a second
mocking strategy.

## Workflows

**Adding an external service client:** 1. Create `clients/<name>/` with a
client module, a types module, and an `index.ts` barrel (no cross-package
`index.ts` re-export policy applies to consumers, but intra-package barrels
are the norm here). 2. Add URLs to `EXTERNAL_URLS` and timeouts to `TIMEOUTS`
in `@browseros/shared`. 3. Add tests under `tests/lib/clients/`.

**Attributing a new call to the install:** use the `fetch` from
`../browseros-fetch.ts` rather than the global, so the identity header is
attached automatically.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `src/lib/` conventions.
- [`./klavis/AGENTS.md`](klavis/AGENTS.md) — Klavis client.
- [`./llm/AGENTS.md`](llm/AGENTS.md) — LLM provider creation.
- [`./oauth/AGENTS.md`](oauth/AGENTS.md) — OAuth token lifecycle.
- [`../../../../tests/lib/clients/AGENTS.md`](../../../tests/lib/clients/AGENTS.md) — client tests.
