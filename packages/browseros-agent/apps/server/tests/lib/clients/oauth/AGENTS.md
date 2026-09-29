# `tests/lib/clients/oauth/` — OAuth lifecycle tests (offline)

> Part of [`../AGENTS.md`](../AGENTS.md) in `apps/server/tests/lib/clients/`.

## What's here

Two tests for `../../../../../src/lib/clients/oauth/`: the process-wide
lifecycle and the Drizzle token store. Both are hermetic — a temp SQLite
handle and a stubbed transport, no browser and no network.

## Contents

| File | Covers |
|---|---|
| `index.test.ts` | `initializeOAuth(db, browserosId)` and `shutdownOAuth()` — the module-level lifecycle that `../../../../../src/api/server.ts` drives. |
| `token-store.test.ts` | `token-store.ts` persistence over the `oauth_tokens` table: upsert, read, delete, and refresh-state transitions. |

## Rules

**OA-T1 — Temp database only.** Open the db with
`openBrowserOsDatabase({ dbPath: <tmp>, migrationsDir })`. A test that
touches the install's `~/.browseros` database is a data-loss bug.

**OA-T2 — No real callback server.** The port-1455 listener exists in
`callback-server.ts`; these tests do not exercise a real authorize flow. Keep
them at the lifecycle and store level.

**OA-T3 — Never assert on a token value.** Assert on provider, status, and
expiry. A failing assertion that prints a token leaks it into CI logs.

**OA-T4 — `shutdownOAuth()` in teardown.** The module holds process-level
state (the token manager, the lazy callback server). Without teardown, tests
share it.

**OA-T5 — Route behaviour is not covered here.** `GET /oauth/:provider/start`,
`POST /oauth/:provider/token`, `GET /oauth/:provider/status`, and
`DELETE /oauth/:provider` live in `../../../../../src/api/routes/oauth.ts`
and have no route test today.

## Workflows

**Running:** `bun run test:lib` from `apps/server/`.

**Changing the `oauth_tokens` schema:** regenerate the migration
(`bunx drizzle-kit generate`), then update `token-store.test.ts` — it is the
only test that exercises the table shape.

**Adding a provider:** extend `../../../../../src/lib/clients/oauth/providers.ts`
and add a case here if the token shape differs from the existing one.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — `tests/lib/clients/` conventions.
- [`../../../../../src/lib/clients/oauth/AGENTS.md`](../../../../src/lib/clients/oauth/AGENTS.md) — the module under test, including the port-1455 constraint.
- [`../../../../../src/lib/db/schema/oauth.ts`](../../../../src/lib/db/schema/oauth.ts) — the table.
- [`../../../__helpers__/AGENTS.md`](../../../__helpers__/AGENTS.md) — the harness.
