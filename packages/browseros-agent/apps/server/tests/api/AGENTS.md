# `tests/api/` — HTTP layer tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/`.

## What's here

Tests for `../../src/api/`. They are fast and hermetic because they call the
route factories and service classes **directly** — no server boot, no port, no
browser. `request-auth.test.ts` covers the origin/loopback trust gate;
`routes/` covers the endpoint behaviour; `services/` covers the logic behind
the endpoints. This is the mirror of the folder's "factories take `deps`"
design: because nothing is constructed at import time, a test can pass a stub
`Browser`, a fake db, or a fabricated Hono `Context`.

## Contents

| File | Covers |
|---|---|
| `request-auth.test.ts` | `isTrustedAppOrigin()` and `requireTrustedAppOrigin()` from `../../src/api/utils/request-auth.ts`, including the origin-less loopback fallback. |
| `routes/health.test.ts` | `createHealthRoute` — the minimal factory pattern. |
| `routes/status.test.ts` | `createStatusRoute`. |
| `routes/agents.test.ts` | `createAgentRoutes`. |
| `routes/klavis.test.ts` | `createKlavisRoutes`. |
| `services/chat-service.test.ts` | `ChatService` from `../../src/api/services/chat-service.ts`. |
| `services/agents/agent-harness-service.test.ts` | `AgentHarnessService` — see [`services/agents/AGENTS.md`](services/agents/AGENTS.md). |
| `services/klavis/strata-cache.test.ts` | `KlavisStrataCache` — see [`services/klavis/AGENTS.md`](services/klavis/AGENTS.md). |
| `services/klavis/strata-proxy.test.ts` | `connectKlavisProxy` / `buildKlavisToolSet` / `registerKlavisTools`. |

## Rules

**API-T1 — Call the factory, never `createHttpServer()`.** `createHealthRoute({...})`
returns a `Hono` app you can invoke directly. Booting the full server to test
one route is what made these tests slow; don't reintroduce it.

**API-T2 — Pass fake `deps`.** Every factory takes a deps object, so a test
supplies a stub `Browser`, a stub service, and whatever else the module
needs. No module-level singletons, no real ports.

**API-T3 — The trust gate is tested once, here.** `request-auth.test.ts` is
the single place that pins loopback + `chrome-extension:` behaviour. If you
change `isTrustedAppOrigin`, update it; the route tests do not cover it.

**API-T4 — A route change that alters middleware goes through
`../../src/api/server.ts`.** Route tests don't exercise mount-time guards —
they mount the factory bare. Guard behaviour is covered by
`request-auth.test.ts`.

**API-T5 — Keep the grouping.** `request-auth.test.ts` sits at the root of
`tests/api/` because it tests a util, not a route; new util tests follow the
same placement.

## Workflows

**Running:** `bun run test:api` from `apps/server/` (also in `bun run test:core`).

**Testing a new route:** 1. Create `routes/<name>.test.ts`. 2. Import
`create<Name>Routes` from `../../../src/api/routes/<name>`. 3. Build it with
minimal `deps`. 4. Assert on `c.json()` output and status codes.

**Testing a new service:** put it in `services/` mirroring
`src/api/services/`, including subdirectories for `agents/`, `klavis/`, `mcp/`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../src/api/AGENTS.md`](../../src/api/AGENTS.md) — the Hono server and route conventions.
- [`../../src/api/routes/AGENTS.md`](../../src/api/routes/AGENTS.md) — the route modules under test.
- [`../__helpers__/AGENTS.md`](../__helpers__/AGENTS.md) — the harness (not required here).
