# `tests/api/routes/` — route unit tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/api/`.

## What's here

Four tests for the twelve route modules in
[`../../../src/api/routes/`](../../../src/api/routes/AGENTS.md). Each imports
the route factory directly, constructs it with a minimal `deps` object, and
exercises the returned `Hono<Env>` app — no `createHttpServer()`, no port, no
browser. `health.test.ts` is the reference example of the pattern.

## Contents

| File | Covers |
|---|---|
| `health.test.ts` | `createHealthRoute` — the simplest factory; the template for the rest. |
| `status.test.ts` | `createStatusRoute`. |
| `agents.test.ts` | `createAgentRoutes` — the largest route module (793 lines in src). |
| `klavis.test.ts` | `createKlavisRoutes` — the Klavis integration endpoints. |

## Rules

**RT-T1 — Test the factory, not the server.** Import
`create<Name>Routes` from `../../../src/api/routes/<name>` and call it.
`createHttpServer()` in `../../../src/api/server.ts` is covered by
`../../server.integration.test.ts` instead.

**RT-T2 — Supply the deps.** The factories take a deps object (a stub
`Browser`, a browseros id, a service handle). A test that constructs a real
one has over-coupled itself to the server.

**RT-T3 — These tests mount the factory bare.** Mount-time middleware —
`requireTrustedAppOrigin()` on `/monitoring` and `/agents`, the CORS policy —
is applied in `../../../src/api/server.ts` and is not exercised here. Guard
behaviour is pinned by `../request-auth.test.ts`.

**RT-T4 — Response shape, not implementation.** Assert on status code and
JSON body. Don't reach into handler internals; that couples the test to a
refactor that preserves behaviour.

**RT-T5 — Coverage is partial by design.** Four of twelve route modules are
tested. Adding a test for a new route is expected; rewriting an existing
factory's signature without adding one is not.

## Workflows

**Running:** `bun run test:api` from `apps/server/`, or
`bun --env-file=.env.development test tests/api/routes/<name>.test.ts`.

**Adding a test for an untested route** (`chat`, `mcp`, `monitoring`, `oauth`,
`credits`, `provider`, `refine-prompt`, `shutdown`): 1. Create
`<name>.test.ts` beside its siblings. 2. Import the factory. 3. Build with
stub deps. 4. Assert status + body.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/api/` conventions.
- [`../../../src/api/routes/AGENTS.md`](../../../src/api/routes/AGENTS.md) — the route modules.
- [`../AGENTS.md`](../AGENTS.md) — the test suite and group rules.
- [`../../../../AGENTS.md`](../../../../AGENTS.md) — `apps/server/` MCP server internals.
