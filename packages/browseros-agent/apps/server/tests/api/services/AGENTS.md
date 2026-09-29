# `tests/api/services/` — service unit tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/api/`.

## What's here

Tests for `../../../../src/api/services/`, mirroring that folder's structure.
Each service is constructed directly with fake dependencies, so the tests are
hermetic — no server, no port, no browser, no Klavis network call.

## Contents

| File | Covers |
|---|---|
| `chat-service.test.ts` | `ChatService` and `ChatServiceDeps` from `../../../../src/api/services/chat-service.ts`. |
| `agents/agent-harness-service.test.ts` | `AgentHarnessService` — agent CRUD, liveness, turn lifecycle, typed errors. |
| `klavis/strata-cache.test.ts` | `KlavisStrataCache` — caching behaviour for Klavis `createStrata`. |
| `klavis/strata-proxy.test.ts` | `connectKlavisProxy`, `connectKlavisInBackground`, `buildKlavisToolSet`, `registerKlavisTools`. |

## Rules

**SV-T1 — Services take `deps`; so do the tests.** If a service can't be
constructed with fakes, that is a design smell, not a testing problem —
`../../../../src/api/server.ts` already constructs every service with
injected dependencies for exactly this reason.

**SV-T2 — The MCP service has no test here.** `../../../../src/api/services/mcp/`
is covered indirectly through `../../server.integration.test.ts`, because the
tool wrapper is only meaningful against a live transport. Add coverage
there, not with a new mock in this folder.

**SV-T3 — Klavis tests must not hit the network.** Stub `KlavisClient`; the
proxy's value is that it *degrades* to a null handle, and that degradation is
what the tests pin.

**SV-T4 — Mirror the source tree.** A new service gets a test file at the
mirrored path — `services/<name>/<name>.test.ts` — so the mapping stays
greppable in both directions.

## Workflows

**Running:** `bun run test:api` from `apps/server/`.

**Testing a service change:** 1. Construct the service with the same `deps`
shape `server.ts` uses. 2. Assert on the returned data, not on internal state.
3. Cover the failure path explicitly — services here are built to degrade
(503s, null handles, swallowed monitoring errors), and that behaviour is the
part worth pinning.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/api/` conventions.
- [`../../../../src/api/services/AGENTS.md`](../../../src/api/services/AGENTS.md) — the services under test.
- [`./agents/AGENTS.md`](./agents/AGENTS.md) — harness-service tests.
- [`./klavis/AGENTS.md`](./klavis/AGENTS.md) — Klavis proxy and cache tests.
